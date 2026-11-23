# 진행 기록

이 문서는 팀원과 공유하는 핸드오프 문서입니다. **새 작업을 시작하기 전에 이 문서부터 읽고**, 작업이 끝나면 맨 아래에 기록을 추가합니다.
Part별 상태 요약은 [README 진행표](../README.md#진행-상황)에, 세부 체크리스트는 각 Part의 GitHub Issue에 있습니다.

## 지금 상태

- 현재 Part: **2.5 (내 섀시 모델링, CAD → MJCF)**, Issue #4 · Part 1(#2)은 `rl-notes.md` 답만 남음 · **HW 브링업 #9는 안전 펌웨어(duty 60 %, 45° 정지), 보드 전원 VEXT 이전, 트랙 폭·바퀴 지름까지 완료**
- 다음 할 일: 내 모델 PPO 학습(`workspace/nu54/02-balance-bot-with-ppo/train_with_ppo_curriculum_nu54.ipynb`, 약 40분) → ONNX → Part 3 펌웨어 `policy` 모듈(200 Hz)로 실물 시험. 발표까지의 범위는 회고 13 §4
- 팀 일정: 10/14 오프라인 모임(모터 도착, 부품 확인, HW 담당과 기구 정보 논의) · **11/25 최종 발표**

## 결정 사항

| 날짜 | 결정 | 이유 |
|---|---|---|
| 2026-10-04 | 하드웨어는 NU54-DK + 직접 만든 섀시 | 원본 Bala2 Fire 대신 보유한 보드·부품 사용 |
| 2026-10-04 | 펌웨어는 Zephyr (NCS v3.4.1), [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) 구조를 따름 | 같은 보드의 브링업 코드(uart/cli/i2c/nvs/ble_nus)를 재사용 |
| 2026-10-04 | Part 1\~2는 원본 Bala2 모델로 먼저 진행, Part 2.5에서 내 섀시 모델 | 개념을 먼저 익히고, 모델링 오차와 학습 문제를 분리 |
| 2026-10-04 | 원본 폴더(`workspace/software`, `workspace/mechanical`, `Dockerfile.*`)는 수정하지 않음 | `upstream` 업데이트를 merge하기 쉽게 |
| 2026-10-04 | 블로그는 외부 블로그에 게시, 저장소에는 `blog/drafts/` 초안 | 팀원은 저장소에서 바로 읽고, 게시본 URL은 README에 링크 |
| 2026-10-04 | ~~Part 6 원격 조종은 BLE + Web Bluetooth~~ → 2026-10-06 Flutter 앱으로 변경 | nRF54L15에는 WiFi가 없음 |
| 2026-10-04 | 하드웨어 브링업을 Issue #9로 분리, Part 1\~2와 나란히 진행 | 강화학습 결과와 무관해서 병렬 가능, Part 2.5 측정값도 여기서 나옴 (회고 1 §4-1) |
| 2026-10-04 | 커밋 메시지에 `Co-Authored-By: Claude` 서명을 넣음 | nu54v-dk와 달리 이 저장소는 서명 유지 |
| 2026-10-05 | 컨테이너는 `-d --rm --name rl-robotics -e PUID/PGID`로 실행 | 터미널을 차지하지 않고, Jupyter에서 `workspace/` 저장이 됨 |
| 2026-10-05 | 값을 바꾸는 실험은 `docs/experiments.md`에 예측 → 결과 → 해석으로 기록 | 개념 답(`rl-notes.md`)과 실험 기록을 분리 |
| 2026-10-06 | 팀 3명이 담당을 나눔: 나는 시뮬레이션·학습, HW 담당은 기구(Blender, 3D 출력)·회로, 펌웨어는 다 같이 | 4주차 멘토 회의. 각자 저장소를 만들고 서로 collaborator로 참여 → 이 저장소는 내 학습 트랙 |
| 2026-10-06 | Part 6 조종 앱은 Flutter (BLE) | 팀 계획. Web Bluetooth는 튜토리얼을 따른 것이었음 |
| 2026-10-06 | 여러 대 중 로봇 선택은 QR 코드 먼저, NFC는 보류 | 멘토 추천. NFC 핀 P1.02/P1.03이 IMU의 I2C(Qwiic)와 겹침 |
| 2026-10-06 | 전용 PCB 보류 | PCB 제작 지원은 1등 팀만 |
| 2026-10-12 | 엔코더는 P2 폴링(100 µs 타이머, ×4)으로 시작, 최종 핀은 2번째 엔코더 때 | P2는 GPIO 인터럽트 불가 (`gpio2`에 `gpiote-instance` 없음 → -ENOTSUP). P1에는 자유 핀이 거의 없음 |
| 2026-10-12 | 펌웨어 플래시는 J-Link(`DISABLE_SWD` ON)로 | 온보드 DAP(pyOCD)는 이 보드에서 `No ACK`로 안 붙었음 (원인 미확정) |
| 2026-11-22 | Docker 이미지는 BuildKit(`docker buildx build --load`)으로 빌드 | 레거시 빌더는 `TARGETARCH`가 비고, webtop의 VOLUME `/config`에 RUN으로 쓴 파일을 버려서 62/80단계에서 실패 (회고 12) |
| 2026-11-23 | Part 2(Bala2로 PPO 학습)는 건너뛰고 Part 2.5 내 섀시 모델링부터 | Bala2 학습은 영상으로 확인했고, 11/25 발표 전에 내 로봇 모델이 더 중요. 학습은 내 MJCF로 처음 돌린다 |
| 2026-11-23 | 섀시 모델링은 내가 FreeCAD로 함 (HW 담당의 Blender 파일은 쓰지 않음) | 튜토리얼의 `mesh_export.py`·`inertia_utils.py` 흐름을 그대로 따라 익히기 위해 |
| 2026-10-06 | 블로그 초안(`blog/`)은 git에 넣지 않음 (`.gitignore`, 기존 Part 초안도 추적 해제) | 블로그 내용은 외부 블로그에만 둠. README에는 게시 URL만 링크 |

## 기록

### 2026-10-04 — Part 0 세팅

- `upstream` remote 추가 (`ShawnHymel/reinforcement-learning-for-robotics`)
- README 한국어 섹션·진행표, `docs/`, `blog/drafts/`, Issue 템플릿 추가
- 원본 하드웨어 분석: Bala2 Fire는 IMU가 Fire 코어(MPU6886)에, 모터·엔코더는 베이스의 STM32(I2C 0x3A)에 있음 → NU54-DK에서는 IMU, 모터 드라이버, 엔코더 입력을 모두 새로 구성해야 함
- `rl/onnx_actor_to_c.py`가 만드는 `actor.h`는 순수 C(`tanhf`만 사용)라서 Zephyr에서 그대로 쓸 수 있음

- README를 프로젝트 중심의 한국어 README로 다시 씀 (원본 튜토리얼 내용 제거, 출처·라이선스만 남김)
- BOM 기록: MPU-6050, TB6612FNG(칩 확인), JGA25-370 + 홀 엔코더 11 PPR, 바퀴 80 mm, 3S 18650, 5 V 벅 → [hardware.md](hardware.md)
- 남은 것: Docker 데몬이 꺼져 있어 이미지 빌드·스모크 테스트 못 함 (sudo 필요)
- 하드웨어 브링업을 Issue #9로 분리하고 Part 3(#5)에서 해당 항목을 뺌
- 회고: [retros/2026-10-04_session01_setup.md](retros/2026-10-04_session01_setup.md), 프로젝트 `CLAUDE.md` 추가

### 2026-10-04 — Docker 빌드, FreeCAD 교체, Part 1 요약 (기록 누락분)

- Docker 이미지 빌드 약 21분, 19.4 GB. 스모크 테스트 통과 (mujoco 3.7.0, torch 2.11.0+cpu, gymnasium 1.2.3, onnx 1.21.0)
- 원본 FCStd가 FreeCAD 1.1로 저장되어 있어 apt의 0.19 대신 1.1.4 AppImage 사용 → [setup.md](setup.md) §2
- `workspace/nu54/FreeCAD/scripts/mesh_export.py`: 원본 복사본, 출력 기본 위치를 `workspace/nu54/`로 바꾸고 원본 폴더에 쓰기를 막음
- Part 1 원문 요약 [summaries/part1-cad-to-mujoco.md](summaries/part1-cad-to-mujoco.md)

### 2026-10-05 — Part 0 완료, Part 1 시작

- 컨테이너를 백그라운드로 실행 (`docker run -d --name rl-robotics ...`), WebTop·TensorBoard 접속 확인 → Issue #1 닫음
- 컨테이너 권한 문제 해결: PUID 없이 띄우면 uid 911이라 `workspace/`에 저장 불가 → `-e PUID=$(id -u) -e PGID=$(id -g)`. `software/runs`(root 소유)는 chown. 실행 명령을 README·CLAUDE.md·setup.md에 반영
- Part 1: `test_motion.ipynb` 수동 조종, `rl-notes.md` Part 1 질문에 첫 답 작성
- PID 노트북을 `workspace/nu54/01-test-model-in-mujoco/`에 복사해 실행, 기본 게인(KP 7, KD 0.5)으로 서 있음 확인
- 실험 기록 문서 [experiments.md](experiments.md) 추가 (예측 → 결과 → 해석)
- 회고: [retros/2026-10-05_session02_part1-mujoco.md](retros/2026-10-05_session02_part1-mujoco.md)

### 2026-10-06 — 4주차 블로그, 팀 계획 반영

- 4주차 블로그 글 초안 작성 (`blog/drafts/week4-feedback-and-plan.md`, 붙여 넣기용 `.txt`). 멘토 회의 내용, 고친 계획, 시뮬레이션·학습 마일스톤(11/25 기준), 지금까지 한 일
- NFC 보류 이유 확인: nRF54L15는 NFC를 지원하지만 NFC 핀(P1.02/P1.03)을 IMU용 I2C로 씀 (`nu54v-dk/docs/06_i2c.md`)
- 블로그 초안은 git에서 뺌: `.gitignore`에 `blog/`, 기존 Part 초안 추적 해제, README·CLAUDE.md·Issue 템플릿과 Issue #2·#3·#5\~#8 문구 수정
- Part 6을 Flutter 앱 + QR 선택으로 변경: README 진행표·흐름, Issue #8 제목·체크리스트
- 모터(JGA25-370 엔코더 모터) 4개 선주문, 10/14 전 도착 예정
- 게시: https://blog.naver.com/atonrq/224433383333 → README "주간 블로그" 표

<!-- 새 기록은 이 아래에 추가 -->

### 2026-10-12 — HW 브링업: 모터 구동 증명 (Issue #9)

- 모터 도착. DP30-03C(6 V, 전류 제한 0.5 A)로 TB6612 단독 시험 → 회전, 0.04 A. 사진 `docs/images/hw-motor-test-6v.jpg`
- `firmware/` 새로 만듦: nu54v-dk(`b794911`)의 boards·scripts·ncs_config 복사, `projects/motor` = `module` 예제에서 led·shtc3를 빼고 `motor`(PWM20 20 kHz, P1.10, 방향 P2.00/P2.01), `enc`(P2.04/P2.05, 타이머 폴링 ×4) 모듈 추가. CLI: `motor a|b duty [ms]`, `motor stop|brake|info`, `enc info|clear|watch`
- 시험: 6 V 무부하, duty 20\~100 % 정회전, −40 % 역회전, 전류 0.04\~0.05 A. 엔코더 부호가 모터 방향과 일치(+7078 / −7175 틱, 3 s, 40 %, 약 9.9 rad/s)
- **`ENC_TICKS_PER_REV` = 1496 (기어비 1:34) 잠정**: 10바퀴 실측 14701 / 14955, 2차가 이론값과 0.03 % 일치 (1차는 손으로 덜 돌린 것으로 추정)
- 알아낸 것: ① 헤더 P2는 두 열(왼쪽 P2.xx, 오른쪽 P1.xx), `P1.0`(=P1.00)은 크리스털 핀 ② P2는 GPIO 인터럽트 불가 ③ `DISABLE_SWD` ON + J-Link일 때는 `JLinkExe`로 플래시하고, 이때 VCOM1(CLI)은 `if03` 포트 (J-Link를 꽂으면 `/dev/ttyACM*` 번호가 바뀜)
- Issue #9에 단계마다 코멘트, 체크리스트 갱신. README 진행표 HW 🟡
- 회고: [retros/2026-10-12_session03_hw-motor.md](retros/2026-10-12_session03_hw-motor.md)

### 2026-10-13 — 5주차 블로그

- 5주차 블로그 글 초안 작성 (`blog/drafts/week5-motor-bringup.txt`, 붙여 넣기용). 모터 구동과 엔코더 읽기 과정, 사용한 부품, 연결 정보(핀·커넥터 선 색), 테스트 코드 GitHub 링크(커밋 `06b6685` 기준), 막힌 것과 다음 계획, 멘토 질문
- 비어 있는 것: 모터 구동 영상, 엔코더 사진, 다른 팀원의 진행, 10/14 모임 논의
- 게시: https://blog.naver.com/atonrq/224440807065 → README 진행표 HW 줄에 링크

### 2026-10-15 — 전원부 검증 (Issue #10)

- Issue [#10](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/10) 생성: STELA POWER BOARD V2.1(TPS54560) + 3S2P 4400 mAh 배터리. 회로도가 없어 측정으로 구조를 읽음. TPS54560은 데이터시트 확인 (8핀 SO PowerPAD, EN 1.2 V 문턱·플로팅이면 켜짐, EN 절대 최대 8.4 V)
- 알아낸 것: ① 배터리 전압은 정상, 출력이 안 나온 이유는 **스위치 커넥터 J2가 열려 있어서** (`OL`). 두 핀을 단락하면 `라즈베리파이` 단자에 5 V대가 유지됨 ② 단락을 풀 때 출력이 서서히 내려가는 것은 출력 커패시터 방전이라 정상 ③ `인디게이터` 단자는 스위치 ON일 때 12 V (켜짐 표시용으로 추정, 충전 표시 아님) ④ 충전 IC는 안 보임. `충전`은 외부 충전기용 통로로 추정
- 충전 시험(12 V / 0.5 A, DP30-03C): 11.8 → 11.9 V로 미세 상승, 확정 못 함 (전압 차이가 작은 시험). 방전 후 재시험 예정
- 배터리 라벨의 440 mAh는 4400 mAh의 오기였음 (48.84 Wh와 일치)
- `hardware.md` §1·§5-4 갱신 (커밋 `b40a982`), #9에 #10 링크
- 회고: [retros/2026-10-15_session04_hw-power.md](retros/2026-10-15_session04_hw-power.md)

### 2026-10-17 — IMU 모듈과 pitch 부호 (Issue #11)

- I2C 인식(0x68)까지 확인한 뒤 Issue [#11](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/11)로 분리. 헤더 점퍼는 보드 표기 `P1.2`에 SCL, `P1.3`에 SDA (문서와 반대, 원인 미확정)
- `firmware/projects/motor`에 `imu` 모듈 추가: MPU-6050, DLPF 3 / 200 Hz / 자이로 ±500 dps / 가속도 ±2 g, 14바이트 한 번에 읽기, 자이로 바이어스 보정(RAM). CLI `imu info|read|watch|cal|clear`
- 시험: 칩 면을 위로 놓으면 정지 가속도 Z ≈ +1083 mg. 앞(핀 쪽)이 아래면 ax +, 자이로 Y −. 가속도 각도 38\~41°와 자이로 적분 42° 일치
- **결론: `pitch = +atan2(ax, az)`, `pitch_rate = −gy`** (시뮬레이션의 "앞으로 기울면 +"에 맞춤, 원본 펌웨어의 `−atan2`, `−gyro.x`와 다름). `hardware.md` §4·§5-6 반영, #11·#9 코멘트
- 남은 것: 자이로 바이어스가 보정할 때마다 달라짐(예열·반복 보정으로 안정성 확인 후 nvs 저장), 가속도 크기 1.02\~1.09 g (둘 다 #9에서 이어감). SDA/SCL 원인 확인은 생략하기로 하고 #11 닫음
- baram-term은 Claude가 직접 실행 (`.venv/bin/baram-term <if03 포트>`, 외부 제어는 이미 켜져 있었음)

### 2026-10-17 — 자이로 바이어스·가속도 오프셋 (Issue #9, 세션 6)

- 자이로 바이어스 3회 보정(30초 간격, 정지 고정, 칩 27.5 °C): 모두 −26 172 26 raw → 이전의 들쭉날쭉한 값은 자세를 옮긴 직후 보정한 탓
- 가속도 6면 평균: 오프셋 X +5 / Y +14 / Z +78 mg, 스케일 1.000 / 0.997 / 1.005 g → 1.02\~1.09 g의 원인은 Z 오프셋. 손으로 세운 면은 5\~12° 기울어 몇 mg 오차가 있음
- `motor` 프로젝트에 `nvs` 모듈(nu54v-dk에서 복사, Settings+ZMS) 추가. `imu`: 가속도 오프셋 상수 적용(`IMU_ACCEL_OFFSET_MG`), `imu cal`이 바이어스를 nvs에 저장하고 부팅 때 불러옴, `imu clear`는 저장값도 삭제. J-Link로 플래시, Z+ 면 |g| 1081 → 1000.4 mg, 리셋 후 바이어스 복원 확인
- 알아낸 것: `imu watch <ms>`의 인자는 기간이 아니라 출력 간격이고 키 입력이 올 때까지 돈다 (`baram-ctl send "imu watch 20" --timeout 2` 후 `send q`로 끊음)
- SDA/SCL 원인 확인은 하지 않기로 함 (사용자 결정). 회고: [retros/2026-10-17_session06_hw-imu-calibration.md](retros/2026-10-17_session06_hw-imu-calibration.md)

### 2026-10-18 — 모터 B 연결, PWM 핀 변경 (Issue #9, 세션 7)

- 핀 예산: STBY는 이미 3.3 V 고정이라 P2.04를 못 풀고 자유 핀은 P2.06 하나 → **모터 B 엔코더는 A상만(P2.06, ×2)**, 방향은 `motorGet()` 부호. `enc` 모듈에 single 채널 추가 (`ENC_B_DIR`), `enc watch <ms> [b]`
- 배선 사고: PWMA가 `1.0`(크리스털) → `1.1` → … 로 옮겨 다니며 못 찾음. 원인: **P1.10(LED2)이 헤더에 나와 있지 않다** (핀맵 이미지). 어제 어디에 꽂았는지 기록이 없었음. LED2 밝기 변화로 펌웨어는 정상임을 가른 뒤, 헤더를 멀티미터로 재다가 `1.11`·`1.13`이 3.3 V → 풀업(PMIC_INT, SW1)이지 PWM이 아님
- PWMB를 `1.13`(SW1 핀)에 꽂아 duty 고정 100 %로 돌았음 (duty 40 %와 100 %의 속도가 비슷해서 이상했음). `1.14`로 옮겨야 함
- **PWMA를 P1.10 → P1.12(헤더 `1.12`)로 변경**. 이후 duty에 따라 속도가 바뀜. 6 V, 70 %에서 A 5.92 V / 0.05 A
- 모터 A 엔코더 부호가 −로 나옴(배선을 다시 한 뒤) → `ENC_A_DIR = −1`. 두 바퀴는 같은 방향으로 돎
- 모터 B 10바퀴 실측 7546틱 (이론 748×10 = 7480, +0.9 %) → 기어비 1:34 확정에 가까움
- 3초, 6 V: 40 % → A +3063 (약 41 rpm), B +1103 (약 30 rpm). B가 약 70 % 속도
- 두 모터 동시 구동 (6 V, 40 %): 5.93 V / 0.03 A, 간섭 없음, 같은 방향
- **데드존 (6 V, 정지 시작 2초)**: A 30\~35 %, B 10\~15 %. B는 5 %마다 약 +110틱으로 선형. 같은 duty에서 두 바퀴가 다르게 움직이는 구간이 있음 → 실제 로봇에서는 duty 보정 필요
- **12 V 무부하 (5초 평균, duty 100 %)**: A 18.4 rad/s (176 rpm), B 18.8 rad/s (180 rpm), 전류 0.11 A. 정격 170 rpm과 일치
- 남은 것: 데드존 1 % 단위·역회전, 모터 A/B 바꿔 꽂아 A의 큰 데드존 원인(모터/배선) 구분, 스톨 전류, 배터리 전압에서 속도, 바퀴·섀시 질량

### 2026-10-18 — 핀 예산 재검토, 핀맵 확정 (Issue #9, 세션 8)

- 사용자가 회로도 PDF(`docs/NU54_DK_2026-07-13T15_16_09_UTC_SCH.pdf`)와 핀맵 이미지(`docs/images/NU54V-DK_pinoutfrom_baram.png`)를 추가. 헤더는 **P4(왼쪽)와 P2(오른쪽) 두 개**. **P1.10(LED2)은 P4-8번**에 있었고, 옛 이미지가 P4의 `1.10`을 빠뜨린 것이 세션 7 혼란의 원인
- 핀 예산: 자유 핀 7개(P2.00\~P2.06) 외에 LED 핀(P1.10, P1.14, P2.09, P2.07)과 버튼 핀(P1.13, P1.09, P1.08, P0.04, 입력 전용)을 쓸 수 있음. 피할 핀: P1.11·P2.08·P2.10(PMIC), P1.12(VBAT_MON + 100 nF), I2C, 시리얼 선. 그래서 "모터 B 엔코더는 A상만"은 핀이 모자라서가 아니었음
- 변경: **PWMA P1.12 → P1.10(P4-8)**, **모터 B B상(초록) P2.07(P2-21) 추가**. `enc.c`의 single 경로 삭제, 두 모터 모두 2상 ×4. 부호 `dir`: A +1, B −1 (+ 명령에서 count가 늘도록)
- 배선 시험: 처음에는 `motor b`로 두 바퀴가 같이 돎(선이 섞임) → 사용자가 TB6612 선을 표대로 다시 꽂아 해소. 이후 안 돈 것은 3번 모두 **파워서플라이가 꺼져 있었거나 선이 빠진 탓**이었음. 파란 선은 엔코더 전원 +(3.3 V)이고 모터 구동과 무관
- **데드존 재측정 (6 V, 정지 시작 2초)**: A·B 모두 5\~10 %(이전 A 30\~35 %, B 10\~15 %). 10 %에서 A 236 / B 272틱, 이후 5 %마다 약 +240틱 선형, 50 %에서 A 2195 / B 2237틱으로 속도도 거의 같음. A의 큰 데드존은 P1.12의 100 nF 부하 때문일 가능성이 큼 (모터·배선 교체는 하지 않아 단정은 못 함)
- `hardware.md` §3에 **확정 핀맵 표**(MCU 핀·헤더 핀 번호) 추가, 핀 예산·체크리스트 갱신
- 남은 것: 모터 B 10바퀴 ×4 틱(이론 1496/회전), 데드존 1 % 단위·역회전, 스톨 전류, 배터리 전압에서 속도, 바퀴·섀시 질량

### 2026-10-18 — 실측: 10바퀴, 데드존, 스톨 전류, 배터리 속도 (Issue #9, 세션 9)

- **모터 B 10바퀴(손, ×4)**: −15121틱 = 1512틱/회전, 이론(1496) 대비 +1.08 % (`err` 0). 어제 ×2도 +0.9 %라 `ENC_TICKS_PER_REV` = 1496 유지, 속도 환산 오차 약 1 %
- **데드존 1 % 단위·역회전 (6 V, 정지 시작 2초)**: 정회전 A 16 % / B 17 %, 역회전 A·B 14 %. 12\~18 %는 정지마찰 탓에 같은 duty에서도 움직였다 멈추는 불규칙 구간. 20 %는 정·역 대칭(A +734/−724, B +743/−745). 모델에는 데드존 ≈15 % 사용. **같은 날 오전의 5\~10 %와 모순, 원인 미확정** (파워서플라이는 6 V). P1.12의 100 nF 가설은 약해짐
- **스톨 전류 (6 V, 제한 1 A, 손으로 잡음)**: 50 % 0.2 A, 70 % 0.4 A(43틱), 100 % 0.8 A(987틱, 미끄러짐). 완전 스톨이 아니어서 6 V ≥0.8 A (하한), 12 V 환산 ≥1.6 A로 TB6612 지속 1.2 A 초과. PWM 평균 전류를 duty로 나눈 첫 추정은 틀려서 정정함. 펌웨어에서 duty 상한·넘어짐 시 모터 정지 필요(Part 3)
- **배터리(11.77 → 11.68 V) 무부하 100 %, 5초**: A 18.2 / B 18.4 rad/s (12 V 파워서플라이 대비 −1\~2 %)
- 한 모터 구동 중 다른 모터 엔코더가 수십\~수천 틱 움직임(진동으로 축이 돎, 무부하 값에는 영향 없음)
- 미완: 질량·무게중심·트랙 폭(저울이 없어 보류), 완전 스톨 값·스톨 토크, USB+VBAT 5 V 동시 연결 역류 확인
- 회고: [retros/2026-10-18_session09_hw-measurements.md](retros/2026-10-18_session09_hw-measurements.md)

### 2026-11-17 — 배선도대로 회로 검증: 모터 구동 시험 (Issue #9, 세션 10)

- 사용자가 Fritzing 배선도(`fritzing/nucode_selfbalancing_wired.fzz`, `wire_sketch.py`)를 만듦. 내용은 `hardware.md` §3 확정 핀맵과 같음. 펌웨어는 세션 9 그대로 (다시 플래시하지 않음), J-Link 연결, CLI는 VCOM1
- 시험 순서(각 2초): A ±30 % → B ±30 % → A·B 동시 +50 %. 판정: 틱 크기·부호, 다른 채널 0, 두 바퀴 방향(눈)
- **6 V → DC-DC → VM**: A ±1190 / B ±1230틱. **배터리 → DC-DC → VM**: A ±2610 / B ±2645, 50 % 동시 A 5546 / B 4596(약 92 rpm). 6 V 대비 ×2.2라서 VM 약 11\~12 V로 추정 (재지 않음). A·B 차 2 % 이내
- 빵판을 옮긴 뒤 다시 시험: 옮기기 전과 1 % 이내. 한 번 더 다시 연결한 뒤에도 한 번에 정상 (30 %에서 1\~2 % 낮음, 배터리 전압 저하로 추정)
- 겪은 문제: ① **DC-DC 스위치 J2 OFF 3번** (두 모터 모두 ±10틱 이하, 100 %에서 0틱) ② 파워서플라이를 VM에 직결했을 때 VM 3.7 V(접촉 불량), 이때 B가 A의 1/3로 느렸으나 다시 연결하자 사라짐 ③ 빵판 이동 뒤 **AIN1–AIN2 단락** → A만 0틱. `motor a 50`을 켜 두고 잰 TB6612 전압이 PWMA 1.6 V(정상), AIN1·AIN2 둘 다 1.6 V → 도통 시험으로 확인 ④ 고치면서 **AIN1 ↔ AIN2 뒤바뀜** → A 부호 반대, 두 바퀴 반대 방향 → 배선을 표대로 되돌림
- **최종 배선도** [images/wire_261117.png](images/wire_261117.png) (사용자 작성): DC-DC 12 V → TB6612 VM, 5 V → NU54-DK VBAT, 모터 A·B 엔코더 선 포함. `hardware.md` §3에 링크
- `hardware.md` §3 모터 전원 줄(배터리 → STELA J2 → VM)과 배선 점검 순서 추가. Issue #9 배선 항목 체크, 단계마다 코멘트 4개
- 부품 재배치하며 IMU 재배선: 처음엔 `i2c scan`에 0x68 없음(PMIC 0x6A만) → 다시 꽂자 WHO_AM_I 0x68. `imu` 모듈은 부팅 때만 초기화하므로 리셋 후 `init 1` (자이로 보정값 nvs에서 불러옴)
- **pitch 축·부호 재측정** (섀시 장착, 앞 = 배터리 커넥터 쪽, 손으로 기울임): 직립 (−45, −24, 949) mg, 앞으로 ax −990, 뒤로 ax +428(26.5°). 앞으로 기울이는 동안 자이로 Y +, 가속도 각 변화 27.3° ↔ 자이로 적분 27.0°. 10-17과 부호가 둘 다 반대 → **`pitch = −atan2(ax, az)`, `pitch_rate = +gy`로 시뮬레이션 식과 같음** (`hardware.md` §4, §5-6). 남음: 직립 −2.7° 원인(손 자세/장착 오프셋), 자이로 X 잔차 −0.9 dps → `imu cal` 다시
- 회고: [retros/2026-11-17_session10_hw-wiring-verify.md](retros/2026-11-17_session10_hw-wiring-verify.md)

### 2026-11-22 — 안전 펌웨어, VM·치수 실측, 보드 전원 VEXT로 이전 (Issue #9, 세션 11)

- Issue #9 본문의 pitch 식을 세션 10 결과(`−atan2(ax, az)`, `+gy`)로 고침
- **안전 펌웨어** (`firmware/projects/motor`): duty 상한 `HW_MOTOR_DUTY_MAX` 100 → **60 %** (12 V 스톨 ≥1.6 A, TB6612 지속 1.2 A → 1.2/1.6 ≈ 75 %인데 하한이라 여유), `motor max n`. 새 ap 모듈 `safety`(스레드 우선순위 4): 10 ms마다 `pitch = −atan2(ax, az)`, |pitch| > **45°** 3번 연속(30 ms) 또는 IMU 10번 연속 실패면 모터 정지·잠금, `safety info|reset|on|off`. `imu.c` raw 버퍼에 뮤텍스. 기준값은 사용자가 고름 (원본 펌웨어 0.79 rad와 같음, 시뮬레이션 종료는 30°)
- 시험: 부팅 때 누워 있으면 95°로 잠기고 `motor a 30` 거부. 두 모터 30 % 구동 중 앞으로 **+45.8°**, 뒤로 **−46.4°**에서 정지. 앞으로 기울면 + (시뮬레이션 부호와 같음)
- **VM**: 정지 배터리 11.733 V / VM 11.714 V, 60 % 두 모터 무부하 VM 11.674 V → STELA DC-DC가 배터리 전압을 거의 그대로 넘김, **VM ≈ 배터리**. 60 % 무부하 약 10.8 rad/s (A·B 차 0.3 %)
- **치수(자)**: 바퀴 바깥 끝 사이 285 mm, 바퀴 폭 40 mm → **트랙 폭 245 mm**, **바퀴 지름 75 mm** (사양 80 mm보다 6 % 작음, 반지름 0.0375 m). 사진 `images/*_261122.jpg`
- **보드 전원 입력 (회로도 시트 3)**: USB VBUS와 VEXT는 쇼트키(D2, D1) OR라 역류 없음. 그런데 그동안 5 V를 넣던 **VBAT는 BQ25186(1셀 충전 IC)의 배터리 핀** — USB·J-Link 연결 시 4.984 V (D4 ESD9B5.0 5.0 V 바로 아래). 사용자가 **STELA 5 V를 VEXT로 옮김** → 5 V만으로 부팅 OK, USB 연결 시 VBAT 4.097 V(충전 IC 출력), 모터 구동 중 리셋 없음
- `hardware.md` §1·§2·§3·§5-2·§5-4·§5-5 갱신, Issue #9 코멘트 2개 + 체크리스트. 배선도 `wire_261117.png`는 아직 VBAT로 그려져 있음
- 회고: [retros/2026-11-22_session11_hw-safety-power.md](retros/2026-11-22_session11_hw-safety-power.md)

### 2026-11-22 — Part 1 PID 외란 실험 A\~E (Issue #2, 세션 12)

- `rl-robotics` 이미지가 없어져 있었음, 루트 디스크 여유 11 GB. 캐시(약 9 GB)·Downloads 설치 파일(5.3 GB) 삭제, `build_qemu`·`linux-rpi`·`linux-stable`을 `/mnt/shared/projects`로 옮기고 심볼릭 링크 → 30 GB
- 빌드: 레거시 빌더(buildx 없음)로 두 번 실패 (`TARGETARCH` 빈 값, VOLUME `/config` 쓰기 소실) → 사용자가 `docker-buildx` 0.30.1 설치 → 성공(19.7 GB). 빌드 직후 BuildKit 캐시 15 GB로 루트 24 MB까지 떨어짐 → `docker builder prune -af`, 여유 9.4 GB
- 노트북 복사본에 키 반복 입력 무시(`KEY_DEBOUNCE` 0.3 s) 추가: WebTop에서 키 하나가 여러 번 들어가 힘이 저절로 바뀌었음
- **기준(ALPHA 0.99, KP 7, KD 0.5)은 0.05 N(0.1초)에도 굴러가다 넘어짐** (예측 1\~3 N). 굴러갈 때 `pitch (filtered)` ≈ 0인데 실제로는 기울어 있음
- A KD = 0: 0.05 N은 버팀, 30초 넘으면 넘어짐 · B KP = 20: 외란 없이 넘어짐 · C KP = 2: 서 있음, 0.05 N에 대개 넘어짐 · D ALPHA = 0.5: 바로 넘어짐 · **E 진짜 pitch: 넘어지지 않음, 세게 밀면 일정 속도로 계속 굴러감**
- 해석·정리는 사용자 요청으로 Claude가 초안을 씀 (표시해 둠, 사용자 검토 필요). `rl-notes.md`는 손대지 않음. 질문별 힌트(MJCF 줄 번호)는 회고 12 §4
- 사용자 질문 "왜 내 로봇이 아니라 Bala2로?" → 10/4 결정(개념 먼저, 모델링 오차와 분리) 설명. 숫자(0.05 N, 게인)는 내 로봇에 옮겨지지 않고 개념(추정 오차, 가속도계에 섞이는 바퀴 가속, PD에 위치·속도 없음)이 옮겨짐
- 회고: [retros/2026-11-22_session12_part1-pid-experiments.md](retros/2026-11-22_session12_part1-pid-experiments.md)

### 2026-11-23 — 내 섀시 모델링: 치수 → FreeCAD → MJCF → PPO 준비 (Issue #4, 세션 13)

- 결정: Part 2(Bala2 PPO)는 건너뛰고 Part 2.5부터, 모델링은 FreeCAD로 직접 (결정 사항 표). #3에 코멘트
- FreeCAD 1.1.4 AppImage를 이 데스크톱에 설치 (`~/Applications`, `freecad` 링크). `freecadcmd`로 Bala2 바퀴 관성이 MJCF 값과 같게 나오는지 확인
- 섀시 치수 실측 (자, 원점 = 바퀴 축 중심) → `hardware.md` §2-1. IMU 칩 (앞 45, 왼쪽 4.5, 위 37.5) mm, 보드 축 = 몸체 축
- `workspace/nu54/FreeCAD/scripts/build_nu54_model.py`로 상자·원기둥 모델 생성 → `nu54/nu54.FCStd`. 좌표는 Bala2와 같은 +X 왼쪽 / **−Y 앞**. 섀시는 고체 하나여야 함(Compound면 무게중심·관성 없음). 사용자가 GUI로 사진과 대조
- `mesh_export`로 STL 3개 (`rpy_degrees=(0,0,90)`)
- 질량(저울 없음): 모터 사양표(JGA25-370-1260, 105 g, 스톨 0.275 N·m) + 부피×밀도 → 섀시 917 g, 바퀴 45 g. 칼날 무게중심 30 mm(바퀴 포함) → 섀시 33 mm. **`inertia_utils`는 기하 중심(71.6 mm)에서 평행이동만 해서 pitch 관성을 1.8배 크게 잡음 → 부품별 합 사용** (사용자 결정). `hardware.md` §2-2
- `nu54/nu54.xml`: gear 0.165(ctrl 1 = duty 60 %), damping 0.0134(역기전력, 최고 속도 실측 10.8 rad/s에 맞춤), frictionloss 0.02(데드존), armature 5.8e-4(추정). MuJoCo에서 무게중심 30 mm, ctrl 1 → 10.82 rad/s 확인
- `test_motion_nu54` 손 조종: 사용자는 Bala2보다 어렵다고 느꼈지만 시뮬레이션으로는 3배 느리게 넘어짐(ctrl 0.2에서 0.45 s 대 0.15 s). 첫 키 0.1이 데드존 안이고 디바운스가 없어서로 추정
- PPO 준비: `workspace/nu54/02-balance-bot-with-ppo/` (env는 카메라만 바꿈, 노트북은 `/workspace/software`로 chdir해 TensorBoard에 보이게). 스모크 학습 정상, 약 1700 steps/s → 전체 약 40분
- 컨테이너가 홈 폴더에서 떠서 `~/workspace`(root 소유 빈 폴더)를 마운트하고 있었음 → 저장소에서 다시 띄움. `~/workspace` 삭제는 앱 터미널에서 sudo가 막혀 남아 있음
- 영상 스크립트 `docs/youtube/` 추가 (Part 1\~6). Part 3은 영상도 첫 배포가 실패하고 펌웨어 보정 4개로 섰음 → 우리는 그중 모터 부스트·데드밴드를 모델에 미리 넣음 (회고 13 §4)
- 회고: [retros/2026-11-23_session13_nu54-model.md](retros/2026-11-23_session13_nu54-model.md)
