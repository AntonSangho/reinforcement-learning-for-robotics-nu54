/*
 * enc.c — 모터 홀 엔코더 (A/B 2상, 직교)
 *
 *   모터 A: A상 P2.04 (노랑), B상 P2.05 (초록)
 *   모터 B: A상만 P2.06 (노랑), 초록은 비워 둠. 엣지 x2 = 22 틱/축 회전, 방향은 motorGet() 부호
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
#include "motor.h"


typedef struct
{
  struct gpio_dt_spec     pin_a;
  struct gpio_dt_spec     pin_b;
  bool                    single;   // true: A상만 연결 (핀 예산), 방향은 모터 명령 부호로 추정
  int8_t                  dir;      // single 일 때 마지막으로 명령한 방향 (+1/-1), 정지 중에도 유지
  struct k_timer          timer;
  volatile int32_t        count;
  volatile uint8_t        state;    // (A << 1) | B
  volatile uint32_t       err;      // 두 핀이 동시에 바뀐 경우 (틱을 놓침)
} enc_tbl_t;

static enc_tbl_t enc_tbl[ENC_MAX_CH] = {
  {GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 0),
   GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 1), false, 1},
  // 모터 B: A상만 P2.06. pin_b 는 쓰지 않는다
  {GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 2),
   GPIO_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), enc_gpios, 2), true, 1},
};

// 모터 A 엔코더 부호 (직교). 모터를 + 로 돌렸을 때 count 가 늘도록 맞춘다 (2026-10-18: 배선 변경 후 − 였음)
#define ENC_A_DIR   (-1)

// 모터 B 엔코더 부호 (single). 모터를 + 로 돌렸을 때 count 가 늘도록 시험 후 맞춘다
#define ENC_B_DIR   (+1)

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

  if (p->single)
  {
    // A상 양쪽 엣지만 센다 (x2). 방향은 모터 명령의 부호
    int32_t duty = motorGet(p - enc_tbl);

    if (duty > 0) p->dir = +1 * ENC_B_DIR;
    if (duty < 0) p->dir = -1 * ENC_B_DIR;

    cur = (uint8_t)gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin);
    if (cur != (p->state & 1))
    {
      p->count += p->dir;
    }
    p->state = cur;
    return;
  }

  cur  = (uint8_t)((gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin) << 1) |
                    gpio_pin_get_raw(p->pin_b.port, p->pin_b.pin));
  step = step_tbl[(p->state << 2) | cur];

  if (step == 0 && cur != p->state)
    p->err++;

  p->count += step * ENC_A_DIR;
  p->state  = cur;
}

bool encInit(void)
{
  bool ret = true;

  for (int i=0; i<ENC_MAX_CH; i++)
  {
    enc_tbl_t *p = &enc_tbl[i];

    int e[2] = {0, 0};

    e[0] = gpio_pin_configure_dt(&p->pin_a, GPIO_INPUT);
    if (p->single)
    {
      p->state = (uint8_t)gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin);
    }
    else
    {
      e[1] = gpio_pin_configure_dt(&p->pin_b, GPIO_INPUT);
      p->state = (uint8_t)((gpio_pin_get_raw(p->pin_a.port, p->pin_a.pin) << 1) |
                            gpio_pin_get_raw(p->pin_b.port, p->pin_b.pin));
    }

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
                enc_tbl[i].single ? -1 : gpio_pin_get_raw(enc_tbl[i].pin_b.port, enc_tbl[i].pin_b.pin));
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

  if (args->argc >= 2 && args->isStr(0, "watch") == true)
  {
    uint8_t ch = (args->argc == 3 && args->isStr(2, "b") == true) ? 1 : 0;

    uint32_t pre_time = millis();

    while (cliKeepLoop())
    {
      if (millis() - pre_time >= (uint32_t)args->getData(1))
      {
        pre_time = millis();
        cliPrintf("count %d, err %u\n", (int)encGetCount(ch), (unsigned)enc_tbl[ch].err);
      }
    }
    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("enc info\n");
    cliPrintf("enc clear\n");
    cliPrintf("enc watch time_ms [b]\n");
  }
}


#endif


#endif
