# Reinforcement Learning for Robotics — NU54-DK 셀프밸런싱 로봇

Shawn Hymel의 [Reinforcement Learning for Robotics](https://www.youtube.com/watch?v=zsdceSTRBl4&list=PLYExBrZNJeQg&index=1) 튜토리얼(Part 1~6)을 따라가면서
**NU54-DK(nRF54L15)** 보드와 **직접 만든 섀시**로 셀프밸런싱 로봇을 만듭니다.
원본 저장소: [ShawnHymel/reinforcement-learning-for-robotics](https://github.com/ShawnHymel/reinforcement-learning-for-robotics) (이 저장소는 그 fork 입니다)

- **목표**: 강화학습(MDP, PPO, domain randomization, sim2real)의 기본 개념을 직접 해 보며 익힌다
- **산출물**: Part별 블로그 글, 학습·펌웨어 코드, 진행 기록

## 진행 상황

| Part | 내용 | 상태 | Issue | 블로그 | 원문 |
|---|---|---|---|---|---|
| 0 | 저장소·개발 환경 세팅 | 🟡 | [#1](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/1) | — | — |
| 1 | CAD → MuJoCo 시뮬레이터 (원본 Bala2 모델) | ⬜ | [#2](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/2) | [초안](blog/drafts/part1-cad-to-mujoco.md) | [Part 1](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-1-cad-to-mujoco-simulator) |
| 2 | PPO로 밸런스 봇 학습 | ⬜ | [#3](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/3) | [초안](blog/drafts/part2-train-with-ppo.md) | [Part 2](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-2-train-a-balance-bot-with-ppo) |
| 2.5 | 내 섀시 모델링 (CAD → MJCF) 후 재학습 | ⬜ | [#4](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/4) | (Part 2 또는 3에 포함) | — |
| 3 | Sim → Real: NU54-DK(Zephyr)에 정책 배포 | ⬜ | [#5](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/5) | [초안](blog/drafts/part3-sim-to-real.md) | [Part 3](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-3-deploy-ai-agent-to-a-robot) |
| 4 | Domain Randomization | ⬜ | [#6](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/6) | [초안](blog/drafts/part4-domain-randomization.md) | [Part 4](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-4-domain-randomization) |
| 5 | 명령(Command) 추가 | ⬜ | [#7](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/7) | [초안](blog/drafts/part5-commands.md) | [Part 5](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-5-adding-commands-to-the-agent) |
| 6 | RC 밸런스 봇 (BLE + Web Bluetooth) | ⬜ | [#8](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/8) | [초안](blog/drafts/part6-rc-balance-bot.md) | [Part 6](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-6-ai-powered-rc-balance-bot) |

⬜ 시작 전 · 🟡 진행 중 · ✅ 완료 — 날짜별 기록과 다음 할 일은 [docs/00_progress.md](docs/00_progress.md)

## 원본과 다른 점

| | 원본 | 이 저장소 |
|---|---|---|
| 컨트롤러 | M5Stack Fire (ESP32) | NU54-DK (nRF54L15, Cortex-M33) |
| 섀시·모터 | M5Stack Bala2 베이스 (STM32, I2C 0x3A) | 직접 만든 섀시 — [docs/hardware.md](docs/hardware.md) |
| IMU | Fire 내장 MPU6886 | Qwiic I2C IMU (외부) |
| 펌웨어 | Arduino (M5Unified) | Zephyr (nRF Connect SDK v3.4.1), [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) 구조 |
| 원격 조종 (Part 6) | ESP32 WiFi 웹서버 | BLE + Web Bluetooth |

Part 1~2는 원본 Bala2 모델로 그대로 따라 하고, Part 2.5에서 내 섀시 모델을 만들어 다시 학습합니다.

## 폴더 안내

| 폴더 | 내용 |
|---|---|
| `workspace/software/`, `workspace/mechanical/` | 원본 튜토리얼 코드·모델 (**수정하지 않음**, upstream 업데이트를 받기 위해) |
| `workspace/nu54/` | 내 섀시 모델과 학습 코드 (Part 2.5부터) |
| `firmware/` | NU54-DK Zephyr 펌웨어 (Part 3부터) |
| `docs/` | 진행 기록, 환경 세팅, 하드웨어, 강화학습 학습 노트 |
| `blog/drafts/` | 블로그 초안 (외부 블로그에 게시) |

개발 환경 세팅: [docs/setup.md](docs/setup.md) · 학습 노트: [docs/rl-notes.md](docs/rl-notes.md)

---

## Original README

This repository holds the development environment and demos used in the Reinforcement Learning for Robotics video series, which can be found [here](https://www.youtube.com/watch?v=zsdceSTRBl4&list=PLYExBrZNJeQg&index=1).

> If you are looking for the workshop/webinar version of this demo, please use this repository: [github.com/ShawnHymel/workshop-reinforcement-learning-for-robotics](https://github.com/ShawnHymel/workshop-reinforcement-learning-for-robotics/)

<a href="https://www.youtube.com/watch?v=zsdceSTRBl4&list=PLYExBrZNJeQg&index=1">
  <img src=".images/rl-for-robotics-thumbnail-play.png" alt="Reinforcement Learning for Robotics" height="500">
</a>

## Installation

Download and install [Docker Desktop](https://www.docker.com/products/docker-desktop/). Make sure it is running before continuing to the next step.

Open a terminal and build the Docker image:

```sh
docker build -t rl-robotics -f Dockerfile.cpu .
```

Run the image:

```sh
docker run -it --rm -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
```

Notes:
 * Port 3000 is for the WebTop interface
 * Port 6006 is for TensorBoard
 * VS Code is memory hungry, so we bump the shared memory up to 2 GB

Browse to [http://localhost:3000/](http://localhost:3000/) to interact with WebTop.

## License

All software in this repository, unless otherwise noted, is licensed under the [Apache-2.0](https://www.apache.org/licenses/LICENSE-2.0) license.
