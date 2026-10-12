/*
 * enc.c — 모터 홀 엔코더 (A/B 2상, 직교)
 *
 *   A상 P2.04 (노랑), B상 P2.05 (초록)
 *   P2 포트는 GPIO 인터럽트를 쓸 수 없다 (gpio2 노드에 gpiote-instance 가 없어 -ENOTSUP).
 *   그래서 타이머(100 us)로 두 핀을 읽는다. 최대 속도의 엣지는 초당 약 4.2 k 개라서 놓치지 않는다.
 *   두 핀의 엣지를 모두 세므로 모터축 11 PPR 은 x4 = 44 틱/축 회전이 된다.
 *   부호는 직교 상태표로 정한다. 어느 방향이 + 인지는 모터를 돌려서 확인한다.
 */

#include "enc.h"


#ifdef _USE_HW_ENC
#include "cli.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "log.h"


typedef struct
{
  struct gpio_dt_spec     pin_a;
  struct gpio_dt_spec     pin_b;
  struct k_timer          timer;
  volatile int32_t        count;
  volatile uint8_t        state;    // (A << 1) | B
  volatile uint32_t       err;      // 두 핀이 동시에 바뀐 경우 (틱을 놓침)
} enc_tbl_t;

static enc_tbl_t enc_tbl[ENC_MAX_CH] = {
  {GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 0),
   GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 1)},
};

// [이전 상태 << 2 | 현재 상태] → 증감
static const int8_t step_tbl[16] = {
   0, +1, -1,  0,
  -1,  0,  0, +1,
  +1,  0,  0, -1,
   0, -1, +1,  0,
};


#ifdef _USE_HW_CLI
static void cliEnc(cli_args_t *args);
#endif


static void encPoll(struct k_timer *timer)
{
  enc_tbl_t *p = CONTAINER_OF(timer, enc_tbl_t, timer);
  uint8_t cur;
  int8_t  step;

  cur  = (uint8_t)((gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin) << 1) |
                    gpio_pin_get_raw(p->pin_b.port, p->pin_b.pin));
  step = step_tbl[(p->state << 2) | cur];

  if (step == 0 && cur != p->state)
    p->err++;

  p->count += step;
  p->state  = cur;
}

bool encInit(void)
{
  bool ret = true;

  for (int i=0; i<ENC_MAX_CH; i++)
  {
    enc_tbl_t *p = &enc_tbl[i];

    int e[2];

    e[0] = gpio_pin_configure_dt(&p->pin_a, GPIO_INPUT);
    e[1] = gpio_pin_configure_dt(&p->pin_b, GPIO_INPUT);

    p->state = (uint8_t)((gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin) << 1) |
                          gpio_pin_get_raw(p->pin_b.port, p->pin_b.pin));

    k_timer_init(&p->timer, encPoll, NULL);
    k_timer_start(&p->timer, K_USEC(100), K_USEC(100));

    for (int k=0; k<2; k++)
    {
      if (e[k] < 0) ret = false;
    }
  }

  logPrintf("[%s] encInit()\n", ret ? "OK" : "E_");

#ifdef _USE_HW_CLI
  cliAdd("enc", cliEnc);
#endif

  return ret;
}

int32_t encGetCount(uint8_t ch)
{
  if (ch >= ENC_MAX_CH) return 0;

  return enc_tbl[ch].count;
}

void encClear(uint8_t ch)
{
  if (ch >= ENC_MAX_CH) return;

  enc_tbl[ch].count = 0;
  enc_tbl[ch].err   = 0;
}





#ifdef _USE_HW_CLI

void cliEnc(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    for (int i=0; i<ENC_MAX_CH; i++)
    {
      cliPrintf("ch%d : count %d, err %u, A %d, B %d\n", i + 1,
                (int)enc_tbl[i].count, (unsigned)enc_tbl[i].err,
                gpio_pin_get_raw(enc_tbl[i].pin_a.port, enc_tbl[i].pin_a.pin),
                gpio_pin_get_raw(enc_tbl[i].pin_b.port, enc_tbl[i].pin_b.pin));
    }
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "clear") == true)
  {
    for (int i=0; i<ENC_MAX_CH; i++)
    {
      encClear(i);
    }
    cliPrintf("clear\n");
    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "watch") == true)
  {
    uint32_t pre_time = millis();

    while (cliKeepLoop())
    {
      if (millis() - pre_time >= (uint32_t)args->getData(1))
      {
        pre_time = millis();
        cliPrintf("count %d, err %u\n", (int)encGetCount(0), (unsigned)enc_tbl[0].err);
      }
    }
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("enc info\n");
    cliPrintf("enc clear\n");
    cliPrintf("enc watch time_ms\n");
  }
}


#endif


#endif
