# 강화학습 학습 노트

각 Part를 진행하면서 아래 질문에 **스스로** 답을 적습니다. 정답을 찾아 옮기기보다, 코드를 실행하고 값을 바꿔 보며 확인한 내용을 씁니다.
여기 적은 답이 블로그 글의 "핵심 개념" 절의 재료가 됩니다.

## Part 1 — CAD → MuJoCo

- MJCF에서 `body`, `joint`, `geom`, `actuator`, `sensor`는 각각 무엇을 나타내나?
  - body: 뼈입니다. 질량과 관성을 가진 강체 하나이며 부모 body에 대한 상대 위치(pos)와 자세로 놓입니다. 자식 body는 부모의 좌표계 안에서 정의됩니다. 
  - joint : 관절입니다. body안에 적으면 "이 body가 부모에 대해 이렇게 움직일 수 있다"는 뜻입니다. joint가 없는 body는 부모에 용접된것과 같습니다. 
  - geom : 살입니다. 눈에 보이는 형상이자 충돌하는 형상이며 질량을 적으면 body의 질량과 관성이 여기서 계산됩니다. 

- 시뮬레이션 timestep(5 ms)을 바꾸면 무엇이 달라지나?
  - 답: 시뮬레이터는 연속된 시간을 잘게 잘라 한 칸씩 계산합니다. 그 칸의 크기는 timestep이고 이 값을 조정하게되면 시뮬레이터가 자르는 시간이 짧아지거나 길어지게됩니다. 
- PID만으로 균형을 잡을 수 있는데 왜 강화학습을 쓰려 하나? PID의 한계는?
  - 답: PID 제어기나 모델예측제어(MPC)와 같은 기존의 로봇 제어방법은 강력하지만, 정확한 모델에 의존하며 새로운 상황이 발생할 때마다 세심한 수동 조정이 필요합니다. 강화 학습은 이와 다른 접근 방식을 취합니다. 즉, 제어 정책을 수동으로 프로그래밍하는 대신, 로봇이 시뮬레이션된 환경과 상호 작용을 통해 제어 정책을 스스로 발견하여, 이전에 경험해 본적 없는 상황에서도 놀라울 정도로 견고한 행동 패턴을 점진적으로 학습합니다. 
  

1. chassis에는 freejoint가, 바퀴에는 일반 joint가 붙어 있습니다. 왜 서로 다른 종류를 썼을까요? freejoint는 자유도가 몇 개일까요?
답 : chassis에는 자유도가 있는 joint를 달수 있는것이고 바퀴에는 자유도가 없는 joint라고 하는거라 생각합니다. freejoint는 아직 자유도를 지정하지 않은거 아닐까요?   

2. 액추에이터 태그가 <motor>입니다. 그런데 노트북은 ctrl 값을 "motorspeed"라고 부릅니다. 211행의 속성을 보고 판단해 보세요. ctrl = 0.5는 속도일까요, 다른 물리량일까요?
답: 모르겠습니다 

3. 위 화살표를 몇 번 눌러 ctrl을 양수로 두면, 로봇 몸체는 앞과 뒤 중 어느 쪽으로 기울까요?
답 :  앞 
4. 손으로 균형을 몇 초나 잡을 수 있을까요?
답 : ??

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

### 관측 분석 (세션 15) — 원본 `balance_bot.ino` ↔ 시뮬레이션 `_get_obs()` ↔ 우리 로봇

비교할 파일: `workspace/software/03-sim-to-real/balance_bot/balance_bot.ino`, `workspace/nu54/02-balance-bot-with-ppo/balance_bot_env.py`(137줄 `_get_obs()`)

- Q1. 원본은 `accel.y`(앞), `accel.z`(위), `-gyro.x`를 씁니다. 우리 MPU-6050에서는 어느 축과 부호가 여기에 해당하나? (근거: `hardware.md` §5-6 재측정 2026-11-17)
  - 답: accel.ly는 accel_x , accel.z는 accel.z, sensor_imu_gyro
- Q2. 바퀴 속도는 5 ms 동안의 틱 차이로 계산합니다. 우리 엔코더(1496틱/회전, 원본은 420)로 바퀴가 1 rad/s로 돌면 5 ms에 몇 틱이 들어오나? 틱 1개 차이는 몇 rad/s인가? 이 양자화가 관측에 어떤 영향을 줄까?
  - 답: 잘모르겠습니다. 
- Q3. 시뮬레이션의 `wheel_vel_left`는 어느 바퀴인가? 실물에서 모터 A와 B 중 어느 쪽이 왼쪽인가? (`nu54.xml` 바퀴 body 위치, +X = 왼쪽, 실물은 배터리 커넥터 쪽이 앞)
  - 답: left는 A, right은 B 
- Q4. 원본은 `while (micros() ...)`로 루프를 5 ms에 맞춥니다. 추론이 5 ms보다 오래 걸리면 관측의 어떤 값이 학습 때와 달라지나? (상보필터 식에서 `TIMESTEP`이 곱해지는 곳)
  - 답: 모르겠습니다.
- Q5 (RUN 1·2 뒤). 실물에서 로봇이 덜덜 떨렸다. 시뮬레이션에서는 보지 못한 행동이다. 원인 후보를 적고, 각 후보를 **어떤 시험으로 가를 수 있는지** 적어 보자. (기록: `workspace/nu54/03-sim-to-real/results/`, `experiments.md` Part 3)
  - 생각할 거리: 시뮬레이션 `_get_obs()`의 바퀴 속도와 실물 바퀴 속도는 값의 모양이 같은가? (DRY 시험 2와 RUN 기록의 `vel_l`·`vel_r` 열)
  - 생각할 거리: 정책은 관측이 조금 바뀔 때 행동이 얼마나 바뀌나? (DRY 시험 2에서 바퀴만 돌렸을 때의 `act`)
  - 생각할 거리: 1단계가 2단계보다 더 떨린 것은 어떤 후보를 지지하나?
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
