# 진행 기록

이 문서는 팀원과 공유하는 핸드오프 문서입니다. **새 작업을 시작하기 전에 이 문서부터 읽고**, 작업이 끝나면 맨 아래에 기록을 추가합니다.
Part별 상태 요약은 [README 진행표](../README.md#진행-상황)에, 세부 체크리스트는 각 Part의 GitHub Issue에 있습니다.

## 지금 상태

- 현재 Part: **1 (CAD → MuJoCo 시뮬레이터)**, Issue #2 · **HW 브링업 #9는 모터·엔코더 A까지 완료**
- 다음 할 일: Part 1 PID 외란 테스트와 실험 A~E ([experiments.md](experiments.md)), `rl-notes.md` 남은 질문 → Issue #2 닫기. HW(#9)는 데드존·12 V·B 채널·IMU가 남음 (회고 3 §4)
- 팀 일정: 10/14 오프라인 모임(모터 도착, 부품 확인, HW 담당과 기구 정보 논의) · **11/25 최종 발표**

## 결정 사항

| 날짜 | 결정 | 이유 |
|---|---|---|
| 2026-10-04 | 하드웨어는 NU54-DK + 직접 만든 섀시 | 원본 Bala2 Fire 대신 보유한 보드·부품 사용 |
| 2026-10-04 | 펌웨어는 Zephyr (NCS v3.4.1), [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) 구조를 따름 | 같은 보드의 브링업 코드(uart/cli/i2c/nvs/ble_nus)를 재사용 |
| 2026-10-04 | Part 1~2는 원본 Bala2 모델로 먼저 진행, Part 2.5에서 내 섀시 모델 | 개념을 먼저 익히고, 모델링 오차와 학습 문제를 분리 |
| 2026-10-04 | 원본 폴더(`workspace/software`, `workspace/mechanical`, `Dockerfile.*`)는 수정하지 않음 | `upstream` 업데이트를 merge하기 쉽게 |
| 2026-10-04 | 블로그는 외부 블로그에 게시, 저장소에는 `blog/drafts/` 초안 | 팀원은 저장소에서 바로 읽고, 게시본 URL은 README에 링크 |
| 2026-10-04 | ~~Part 6 원격 조종은 BLE + Web Bluetooth~~ → 2026-10-06 Flutter 앱으로 변경 | nRF54L15에는 WiFi가 없음 |
| 2026-10-04 | 하드웨어 브링업을 Issue #9로 분리, Part 1~2와 나란히 진행 | 강화학습 결과와 무관해서 병렬 가능, Part 2.5 측정값도 여기서 나옴 (회고 1 §4-1) |
| 2026-10-04 | 커밋 메시지에 `Co-Authored-By: Claude` 서명을 넣음 | nu54v-dk와 달리 이 저장소는 서명 유지 |
| 2026-10-05 | 컨테이너는 `-d --rm --name rl-robotics -e PUID/PGID`로 실행 | 터미널을 차지하지 않고, Jupyter에서 `workspace/` 저장이 됨 |
| 2026-10-05 | 값을 바꾸는 실험은 `docs/experiments.md`에 예측 → 결과 → 해석으로 기록 | 개념 답(`rl-notes.md`)과 실험 기록을 분리 |
| 2026-10-06 | 팀 3명이 담당을 나눔: 나는 시뮬레이션·학습, HW 담당은 기구(Blender, 3D 출력)·회로, 펌웨어는 다 같이 | 4주차 멘토 회의. 각자 저장소를 만들고 서로 collaborator로 참여 → 이 저장소는 내 학습 트랙 |
| 2026-10-06 | Part 6 조종 앱은 Flutter (BLE) | 팀 계획. Web Bluetooth는 튜토리얼을 따른 것이었음 |
| 2026-10-06 | 여러 대 중 로봇 선택은 QR 코드 먼저, NFC는 보류 | 멘토 추천. NFC 핀 P1.02/P1.03이 IMU의 I2C(Qwiic)와 겹침 |
| 2026-10-06 | 전용 PCB 보류 | PCB 제작 지원은 1등 팀만 |
| 2026-10-12 | 엔코더는 P2 폴링(100 µs 타이머, ×4)으로 시작, 최종 핀은 2번째 엔코더 때 | P2는 GPIO 인터럽트 불가 (`gpio2`에 `gpiote-instance` 없음 → -ENOTSUP). P1에는 자유 핀이 거의 없음 |
| 2026-10-12 | 펌웨어 플래시는 J-Link(`DISABLE_SWD` ON)로 | 온보드 DAP(pyOCD)는 이 보드에서 `No ACK`로 안 붙었음 (원인 미확정) |
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
- 블로그 초안은 git에서 뺌: `.gitignore`에 `blog/`, 기존 Part 초안 추적 해제, README·CLAUDE.md·Issue 템플릿과 Issue #2·#3·#5~#8 문구 수정
- Part 6을 Flutter 앱 + QR 선택으로 변경: README 진행표·흐름, Issue #8 제목·체크리스트
- 모터(JGA25-370 엔코더 모터) 4개 선주문, 10/14 전 도착 예정

<!-- 새 기록은 이 아래에 추가 -->

### 2026-10-12 — HW 브링업: 모터 구동 증명 (Issue #9)

- 모터 도착. DP30-03C(6 V, 전류 제한 0.5 A)로 TB6612 단독 시험 → 회전, 0.04 A. 사진 `docs/images/hw-motor-test-6v.jpg`
- `firmware/` 새로 만듦: nu54v-dk(`b794911`)의 boards·scripts·ncs_config 복사, `projects/motor` = `module` 예제에서 led·shtc3를 빼고 `motor`(PWM20 20 kHz, P1.10, 방향 P2.00/P2.01), `enc`(P2.04/P2.05, 타이머 폴링 ×4) 모듈 추가. CLI: `motor a|b duty [ms]`, `motor stop|brake|info`, `enc info|clear|watch`
- 시험: 6 V 무부하, duty 20~100 % 정회전, −40 % 역회전, 전류 0.04~0.05 A. 엔코더 부호가 모터 방향과 일치(+7078 / −7175 틱, 3 s, 40 %, 약 9.9 rad/s)
- **`ENC_TICKS_PER_REV` = 1496 (기어비 1:34) 잠정**: 10바퀴 실측 14701 / 14955, 2차가 이론값과 0.03 % 일치 (1차는 손으로 덜 돌린 것으로 추정)
- 알아낸 것: ① 헤더 P2는 두 열(왼쪽 P2.xx, 오른쪽 P1.xx), `P1.0`(=P1.00)은 크리스털 핀 ② P2는 GPIO 인터럽트 불가 ③ `DISABLE_SWD` ON + J-Link일 때는 `JLinkExe`로 플래시하고, 이때 VCOM1(CLI)은 `if03` 포트 (J-Link를 꽂으면 `/dev/ttyACM*` 번호가 바뀜)
- Issue #9에 단계마다 코멘트, 체크리스트 갱신. README 진행표 HW 🟡
- 회고: [retros/2026-10-12_session03_hw-motor.md](retros/2026-10-12_session03_hw-motor.md)

### 2026-10-13 — 5주차 블로그

- 5주차 블로그 글 초안 작성 (`blog/drafts/week5-motor-bringup.txt`, 붙여 넣기용). 모터 구동과 엔코더 읽기 과정, 사용한 부품, 연결 정보(핀·커넥터 선 색), 테스트 코드 GitHub 링크(커밋 `06b6685` 기준), 막힌 것과 다음 계획, 멘토 질문
- 비어 있는 것: 모터 구동 영상, 엔코더 사진, 다른 팀원의 진행, 10/14 모임 논의
- 게시: https://blog.naver.com/atonrq/224440807065 → README 진행표 HW 줄에 링크
