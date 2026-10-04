# NU54-DK 강화학습 셀프밸런싱 로봇

**NU54-DK(nRF54L15)** 보드와 직접 만든 섀시로 두 바퀴 셀프밸런싱 로봇을 만들고,
MuJoCo 시뮬레이터에서 **강화학습(PPO)으로 학습한 정책**을 실제 로봇에 올리는 프로젝트입니다.
과정을 단계별로 진행하면서 강화학습의 기본 개념을 직접 익히고, 배운 내용을 블로그 글로 정리합니다.

## 목표

- 강화학습의 기본 개념(MDP, 보상 설계, PPO, Sim2Real, Domain Randomization)을 코드를 직접 돌려 보며 익힌다
- 시뮬레이션에서 학습한 정책을 NU54-DK 기반의 실제 로봇에서 균형을 잡게 한다
- 최종적으로 BLE로 조종하는 RC 밸런스 봇을 완성한다

## 산출물

| 산출물 | 위치 |
|---|---|
| 학습·시뮬레이션 코드 | `workspace/nu54/` |
| NU54-DK 펌웨어 (Zephyr) | `firmware/` |
| 단계별 블로그 글 | 초안 `blog/drafts/` → 외부 블로그에 게시 (아래 진행표에 링크) |
| 진행 기록과 학습 노트 | `docs/` |

## 진행 상황

| 단계 | 내용 | 상태 | Issue | 블로그 |
|---|---|---|---|---|
| 0 | 저장소·개발 환경 세팅 | 🟡 | [#1](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/1) | — |
| 1 | CAD → MuJoCo 시뮬레이터 | ⬜ | [#2](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/2) | [초안](blog/drafts/part1-cad-to-mujoco.md) |
| 2 | PPO로 균형 잡기 학습 | ⬜ | [#3](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/3) | [초안](blog/drafts/part2-train-with-ppo.md) |
| 2.5 | 내 섀시 모델링(CAD → MJCF)과 재학습 | ⬜ | [#4](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/4) | (2편 또는 3편에 포함) |
| 3 | Sim → Real: NU54-DK에 정책 배포 | ⬜ | [#5](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/5) | [초안](blog/drafts/part3-sim-to-real.md) |
| 4 | Domain Randomization | ⬜ | [#6](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/6) | [초안](blog/drafts/part4-domain-randomization.md) |
| 5 | 명령(전진·후진·회전) 추가 | ⬜ | [#7](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/7) | [초안](blog/drafts/part5-commands.md) |
| 6 | RC 밸런스 봇 (BLE + Web Bluetooth) | ⬜ | [#8](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/8) | [초안](blog/drafts/part6-rc-balance-bot.md) |

⬜ 시작 전 · 🟡 진행 중 · ✅ 완료 — 날짜별 기록과 다음 할 일: [docs/00_progress.md](docs/00_progress.md)

## 전체 흐름

```
[1] CAD 모델 → MuJoCo 시뮬레이터
        ↓
[2] Gymnasium 환경 + PPO 학습 (원본 Bala2 모델로 개념 익히기)
        ↓
[2.5] 내 섀시를 측정·모델링해서 다시 학습
        ↓
[3] 정책(actor)을 C 헤더로 변환 → NU54-DK에서 5 ms 주기로 추론·모터 제어
        ↓
[4] Domain Randomization으로 시뮬레이션과 실제의 차이 줄이기
        ↓
[5] 목표 속도·회전 명령을 따르도록 학습
        ↓
[6] 스마트폰·PC 브라우저에서 BLE로 조종
```

## 하드웨어

| 구분 | 부품 |
|---|---|
| 컨트롤러 | NU54-DK — nRF54L15 (Cortex-M33, BLE), [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) |
| IMU | GY-521 (MPU-6050, 6축 가속도·자이로, I2C) |
| 모터 드라이버 | TB6612FNG 듀얼 모터 드라이버 (채널당 1.2 A) |
| 모터 | JGA25-370 DC 기어드 모터 (12 V, 170 rpm) + 홀 엔코더 (11 PPR, A/B 2상) × 2 |
| 바퀴 | 고무 타이어, 지름 80 mm × 2 |
| 배터리 | 18650 3S (11.1 V, 완충 12.6 V) |
| 전원 | 12 V → 5 V 벅 컨버터 (보드 전원) |
| 섀시 | 직접 제작 |

부품 목록, 측정값, 핀맵: [docs/hardware.md](docs/hardware.md)

## 개발 환경

| 용도 | 환경 |
|---|---|
| 시뮬레이션·학습 | Docker 이미지 (Python 3.12, MuJoCo, Gymnasium, PyTorch, TensorBoard, JupyterLab) |
| 펌웨어 | nRF Connect SDK v3.4.1 (Zephyr), 보드 타깃 `nu54v_dk/nrf54l15/cpuapp` |
| 시리얼 터미널 | [baram-term](https://github.com/chcbaram/baram-term) |

### 빠른 시작 (학습 환경)

```sh
docker build -t rl-robotics -f Dockerfile.cpu .
docker run -it --rm -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
```

- <http://localhost:3000> — 브라우저 데스크톱 (JupyterLab, VS Code, 터미널)
- <http://localhost:6006> — TensorBoard

자세한 설치·실행 방법: [docs/setup.md](docs/setup.md)

## 폴더 구조

| 폴더 | 내용 |
|---|---|
| `workspace/nu54/` | 내 섀시 모델과 학습 코드 (2.5단계부터) |
| `firmware/` | NU54-DK Zephyr 펌웨어 (3단계부터) |
| `workspace/software/`, `workspace/mechanical/` | 참고용 원본 튜토리얼 코드와 Bala2 모델 (수정하지 않음) |
| `docs/` | 진행 기록, 환경 세팅, 하드웨어, 강화학습 학습 노트 |
| `blog/drafts/` | 블로그 초안 |

## 문서

| 문서 | 내용 |
|---|---|
| [docs/00_progress.md](docs/00_progress.md) | 진행 기록, 결정 사항, 다음 할 일 — **처음 보는 분은 여기부터** |
| [docs/setup.md](docs/setup.md) | 학습·펌웨어 개발 환경 세팅 |
| [docs/hardware.md](docs/hardware.md) | 부품, 측정값, 배선, 시뮬레이션과 실제의 일치 체크리스트 |
| [docs/rl-notes.md](docs/rl-notes.md) | 단계별로 스스로 답하는 강화학습 질문과 용어집 |

## 참고 자료와 라이선스

이 프로젝트는 Shawn Hymel의 *Reinforcement Learning for Robotics* 시리즈를 바탕으로 합니다.
원본은 M5Stack Bala2 Fire(ESP32)를 쓰고, 이 프로젝트는 NU54-DK와 직접 만든 섀시에 맞게 바꿉니다.

- 원본 저장소: [ShawnHymel/reinforcement-learning-for-robotics](https://github.com/ShawnHymel/reinforcement-learning-for-robotics)
- 영상: [YouTube 재생목록](https://www.youtube.com/watch?v=zsdceSTRBl4&list=PLYExBrZNJeQg&index=1)
- 글: DigiKey Maker 튜토리얼
  [1편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-1-cad-to-mujoco-simulator) ·
  [2편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-2-train-a-balance-bot-with-ppo) ·
  [3편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-3-deploy-ai-agent-to-a-robot) ·
  [4편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-4-domain-randomization) ·
  [5편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-5-adding-commands-to-the-agent) ·
  [6편](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-6-ai-powered-rc-balance-bot)

원본 코드와 이 저장소의 소프트웨어는 별도 표기가 없는 한 [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) 라이선스를 따릅니다.
