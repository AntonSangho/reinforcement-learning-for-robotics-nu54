# 강화학습 학습 노트

각 Part를 진행하면서 아래 질문에 **스스로** 답을 적습니다. 정답을 찾아 옮기기보다, 코드를 실행하고 값을 바꿔 보며 확인한 내용을 씁니다.
여기 적은 답이 블로그 글의 "핵심 개념" 절의 재료가 됩니다.

## Part 1 — CAD → MuJoCo

- MJCF에서 `body`, `joint`, `geom`, `actuator`, `sensor`는 각각 무엇을 나타내나?
  - 답:
- 시뮬레이션 timestep(5 ms)을 바꾸면 무엇이 달라지나?
  - 답:
- PID만으로 균형을 잡을 수 있는데 왜 강화학습을 쓰려 하나? PID의 한계는?
  - 답:

## Part 2 — PPO로 학습

- 이 문제를 MDP로 쓰면 상태(state), 행동(action), 보상(reward), 전이(transition)는 각각 무엇인가?
  - 답:
- observation이 `[pitch, pitch_rate, wheel_vel_left, wheel_vel_right]` 4개인 이유는? 빠진 정보는 없나?
  - 답:
- reward 항(alive, pitch, action, position, yaw)을 하나씩 빼거나 키우면 어떤 행동이 나오나? (실험 결과)
  - 답:
- actor와 critic은 각각 무엇을 배우나?
  - 답:
- GAE(λ)와 할인율(γ)은 무엇을 조절하나?
  - 답:
- PPO의 clip은 무엇을 막으려는 장치인가?
  - 답:
- curriculum learning을 쓰면 학습이 왜 쉬워지나?
  - 답:
- TensorBoard에서 어떤 곡선을 보고 "학습이 잘 된다"고 판단하나?
  - 답:

## Part 2.5 — 내 섀시 모델

- 질량·무게중심·모터 토크가 실제와 다르면 학습된 정책은 어떻게 실패하나?
  - 답:

## Part 3 — Sim → Real

- 학습할 때와 실제 로봇에서 관측값이 어긋나는 경우는 어떤 것이 있나? (부호, 단위, 지연, 노이즈)
  - 답:
- 마이크로컨트롤러에서 추론 한 번에 걸리는 시간은? 5 ms 안에 들어오나?
  - 답:
- 시뮬레이션에서는 잘 되는데 실제로는 안 되는 이유를 하나 찾아서 고친 과정은?
  - 답:

## Part 4 — Domain Randomization

- 어떤 물리 파라미터를 무작위로 바꿨고, 범위는 어떻게 정했나?
  - 답:
- 랜덤화를 너무 크게 하면 무엇이 문제인가?
  - 답:

## Part 5 — 명령 추가

- 명령(목표 속도/회전)을 observation에 넣으면 정책이 무엇을 새로 배워야 하나?
  - 답:
- reward를 어떻게 바꿔야 명령을 따르게 되나?
  - 답:

## Part 6 — RC 밸런스 봇

- 사람의 입력처럼 학습 때 본 적 없는 명령 패턴이 들어오면 어떻게 되나?
  - 답:

## 용어집

| 용어 | 내 말로 정리 |
|---|---|
| Agent / Environment | |
| Policy (π) | |
| Value function (V) | |
| Advantage (A) | |
| Episode / Terminated / Truncated | |
| On-policy vs Off-policy | |
| Exploration vs Exploitation | |
| Sim2Real gap | |
| Domain Randomization | |
| Curriculum Learning | |
