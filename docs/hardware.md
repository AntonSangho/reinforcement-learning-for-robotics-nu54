# 하드웨어: NU54-DK 셀프밸런싱 로봇

> Part 2.5(내 섀시 모델링) 전에 이 문서의 빈칸을 채웁니다. 여기 적은 값이 MJCF 모델과 펌웨어 상수에 그대로 들어갑니다.

## 1. 부품 (BOM)

| 구분 | 부품 | 모델명 / 사양 | 수량 | 비고 |
|---|---|---|---|---|
| 컨트롤러 | NU54-DK | nRF54L15 (NCRB54N01VC) | 1 | [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) |
| 모터 | | 기어비: , 정격 전압: , 무부하 속도(rpm): , 스톨 토크: | 2 | |
| 엔코더 | | CPR(모터축): , 바퀴 1회전당 틱: | 2 | |
| 모터 드라이버 | | 채널 수: , 입력 방식(PWM+DIR / IN1·IN2): | 1 | |
| IMU | | I2C 주소: , 축 방향: | 1 | Qwiic J5 연결 |
| 배터리 | | 전압: , 용량: | 1 | |
| 바퀴 | | 지름(mm): | 2 | |
| 섀시 | | 재질: | 1 | |

## 2. 측정값 (MJCF에 들어갈 값)

- [ ] 전체 질량 (g)
- [ ] 섀시(바퀴 제외) 질량과 무게중심 높이 (바퀴 축 기준, mm)
- [ ] 바퀴 1개 질량 (g), 지름·폭 (mm)
- [ ] 바퀴 간 거리 (트랙 폭, mm)
- [ ] 모터 무부하 속도 (rad/s, 배터리 전압에서 실측)
- [ ] 모터 스톨 토크 (N·m, 데이터시트 또는 추정)
- [ ] PWM duty → 바퀴 속도 관계 (데드존 포함)

## 3. 배선 / 핀맵

핀은 nu54v-dk 보드 DTS(`firmware/boards/nucode/nu54v_dk`)에서 고릅니다.

| 신호 | NU54-DK 핀 | 연결 대상 | 비고 |
|---|---|---|---|
| I2C SDA / SCL | P1.02 / P1.03 (i2c21, Qwiic J5) | IMU | PMIC(0x6A)와 같은 버스 |
| 모터 L PWM | | | PWM20은 P1 포트에서만 동작 |
| 모터 L DIR | | | |
| 모터 R PWM | | | |
| 모터 R DIR | | | |
| 엔코더 L A / B | | | QDEC 또는 GPIO 인터럽트 |
| 엔코더 R A / B | | | |
| 모터 전원 | — | 배터리 → 드라이버 VM | 보드 전원과 분리, GND 공통 |

## 4. Sim ↔ Real 일치 체크리스트

원본 펌웨어(`workspace/software/03-sim-to-real/balance_bot/balance_bot.ino`)의 상수와 관측 계산을 기준으로 합니다.

- [ ] 관측 순서 `[pitch, pitch_rate, wheel_vel_left, wheel_vel_right]`
- [ ] 단위: pitch는 rad, pitch_rate와 바퀴 속도는 rad/s
- [ ] pitch 부호 (앞으로 기울면 +? −?) — 시뮬레이션과 실제가 같은지 확인
- [ ] 보완필터 α = 0.99 (학습 env의 `alpha`와 같게)
- [ ] 제어 주기 5 ms (`TIMESTEP = 0.005`, MJCF timestep과 같게)
- [ ] 엔코더 틱 → rad 변환 (`ENC_TICKS_PER_REV`)
- [ ] 모터·엔코더 방향 부호 (`MOTOR_DIR_*`, `ENC_DIR_*`)
- [ ] 넘어짐 판정 각도와 재시작 조건

## 5. 알려진 보드 제약

- **VCOM0를 쓰면 SWD가 죽는다** → CLI와 텔레메트리는 VCOM1 (nu54v-dk README의 "알려진 문제")
- **PWM20은 P1 포트에서만 동작** (nu54v-dk `docs/roadmap.md`의 선택 예제 `pwm`)
