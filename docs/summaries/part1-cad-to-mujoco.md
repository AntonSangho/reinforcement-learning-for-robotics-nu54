# Part 1 원문 요약 — CAD에서 MuJoCo 시뮬레이터로

- 원문: [Reinforcement Learning for Robotics Part 1: CAD to MuJoCo Simulator](https://www.digikey.com/en/maker/tutorials/2026/reinforcement-learning-for-robotics-part-1-cad-to-mujoco-simulator) (Shawn Hymel, DigiKey)
- Issue: [#2](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/2)
- 이 문서는 원문 내용을 정리하고, 저장소의 실제 파일에서 확인한 값과 NU54-DK 섀시와 다른 점을 덧붙인 것이다.
  개념 질문의 답은 [rl-notes.md](../rl-notes.md)에 직접 적는다.

## 한 줄 요약

로봇을 재고, CAD로 단순화해서, MuJoCo가 읽는 MJCF 파일로 옮긴 뒤, 손으로 균형을 잡아 보며 "이걸 자동으로 배우게 하자"는 동기를 얻는 편이다. 강화학습 자체는 아직 나오지 않는다.

## 흐름

```
로봇 실측 → FreeCAD 단순 모델(3 body) → STL + 관성 계산 → MJCF(XML) → MuJoCo에서 수동 조종 → (과제) PID로 균형
```

## 1. 왜 강화학습인가

- PID나 MPC는 사람이 모델을 세우고 게인을 손으로 맞춘다. 강화학습은 환경과 상호작용하면서 제어 정책을 스스로 찾는다.
- 예시로 Boston Dynamics Spot, Disney BDX 드로이드가 강화학습을 제어에 쓴다는 점을 든다.
- 학습은 시뮬레이션에서 하고, 결과를 실물에 옮긴다(Part 3).

## 2. 사전 지식과 하드웨어

| 분야 | 원문 추천 |
|---|---|
| 확률·통계 | Khan Academy |
| 신경망 | Coursera Deep Learning Specialization 앞 3과목 |
| PyTorch | *Deep Learning with PyTorch* (Stevens, Antiga, Viehmann) |
| 강화학습 기초 | University of Alberta RL Specialization (Coursera), Sutton & Barto 교재 |
| 기타 | Arduino/ESP32 기초, FreeCAD 기초 |

원본 하드웨어는 M5Stack BALA2 Fire 키트(K014-E)다.

## 3. 물리 시뮬레이터 고르기

| 시뮬레이터 | 원문 평가 |
|---|---|
| Gazebo | 성숙하고 ROS와 잘 붙지만, 강화학습용 병렬화가 약함 |
| PyBullet | 가볍고 Python 친화적, GPU 지원 제한 |
| **MuJoCo** | **선택.** 강화학습 업계 표준, Python 연동이 깔끔, 문서·커뮤니티 좋음 |
| Unity ML-Agents | GPU 지원이 있으나 입지가 줄어듦 |
| NVIDIA Isaac Sim | 가장 강력하지만 비공개, NVIDIA GPU 필요 |
| Genesis | 떠오르는 선택지, AI 장면 생성 |

MuJoCo는 CPU만으로 돌아가서 누구나 따라 할 수 있다는 점도 선택 이유다.

## 4. 로봇 실측

자, 캘리퍼스, 소형 저울로 아래 값을 잰다.

- 섀시 크기(폭·깊이·높이), 바퀴 지름·폭
- 섀시 기준 바퀴 축 위치
- 섀시와 바퀴 각각의 질량 (kg으로 환산)
- 섀시 무게중심 — 실험으로 균형점을 찾아서 잼
- IMU 위치 (Bala2는 LCD 아래)

## 5. FreeCAD에서 내보내기

- 모델은 **chassis, wheel_left, wheel_right 세 덩어리**로 단순화한다. 타이어 무늬는 그리지 않는다. MuJoCo 충돌 계산이 볼록(convex) 형상을 전제로 하기 때문이다.
- 스크립트 (`workspace/mechanical/FreeCAD/scripts/`)
  - `mesh_export.py`: STL로 내보내면서 90° 회전 (FreeCAD는 Y가 앞, MuJoCo는 X가 앞)
  - `inertia_utils.py`: 밀도가 고르지 않은 실제 부품을 고려해서 관성 모멘트를 계산
- FreeCAD Python 콘솔에서 스크립트 경로를 `sys.path`에 넣고 실행한다. 결과물은 `chassis.stl`, `wheel_left.stl`, `wheel_right.stl`.

### 이 저장소에서 따라 할 때

원문처럼 원본 `meshes/` 폴더로 내보내면 원본 STL을 덮어쓴다 (FreeCAD 버전에 따라 삼각형 분할이 달라져 git에 변경으로 잡힌다).
대신 수정본 `workspace/nu54/FreeCAD/scripts/mesh_export.py`를 쓴다. 출력 폴더를 생략하면 `workspace/nu54/FreeCAD/<문서 이름>/meshes/`로 내보내고, `workspace/mechanical/` 안으로 내보내려 하면 거부한다.

```python
>>> from pathlib import Path
>>> import sys
>>> sys.path.insert(0, '/home/anton/projects/reinforcement-learning-for-robotics-nu54/workspace/nu54/FreeCAD/scripts')
>>> import mesh_export
>>> mesh_export.export_bodies(labels=['chassis', 'wheel_left', 'wheel_right'], rpy_degrees=(0, 0, 90))
```

- 결과: `workspace/nu54/FreeCAD/bala2_fire_simplified/meshes/` (Bala2 연습 결과는 `.gitignore`로 커밋하지 않음)
- 원본 `mesh_export`를 이미 import했다면 FreeCAD를 다시 시작하거나 `sys.modules.pop('mesh_export')` 후 다시 import한다 (같은 이름이라 먼저 불러온 모듈이 남는다).
- Bala2 모델은 대칭이라 yaw +90과 −90의 결과가 같다. 내 섀시에서는 부호에 따라 앞뒤가 바뀐다.

## 6. MJCF 파일 만들기

MJCF는 MuJoCo의 XML 형식이다. URDF보다 명시적이고, 센서·액추에이터·timestep까지 한 파일에 담는다.
파일: [`workspace/mechanical/FreeCAD/bala2-fire/bala2-fire-simplified.xml`](../../workspace/mechanical/FreeCAD/bala2-fire/bala2-fire-simplified.xml)

### 저장소 파일에서 확인한 값

| 항목 | 값 | 비고 |
|---|---|---|
| timestep | 0.005 s (5 ms, 200 Hz) | 실물 제어 루프 주기와 맞춰야 함 |
| integrator | `implicitfast` | |
| 구조 | chassis ← `freejoint` ← world, 바퀴 ← `hinge`(축 `0 1 0`) ← chassis | 섀시 원점 = 바퀴 축 중심, 지면에서 22.5 mm(바퀴 반지름) |
| 섀시 | 0.127 kg, 무게중심 축 위 17 mm | 관성은 FreeCAD 스크립트 출력값 |
| 바퀴 | 0.017 kg × 2, 좌우 ±40 mm (트랙 80 mm) | |
| 마찰 | `0.8 0.005 0.0001` (미끄럼·비틀림·구름) | 고무–바닥 추정값, Part 4에서 랜덤화 |
| 바퀴 관절 | damping 0.001, frictionloss 0.001, armature 4.5e-5 kg·m² | armature = 모터 관성 5e-8 × 기어비 30² (바퀴 관성의 약 10배) |
| 액추에이터 | `motor`, `ctrlrange -1~1`, `gear 0.05` | 토크 = gear × ctrl → 최대 ±0.05 N·m (N20 모터 1:30 추정) |
| 센서 | `imu_accel`, `imu_gyro`, `imu_orientation`(framequat), 바퀴 `jointpos`·`jointvel` × 2 | IMU site는 섀시 원점에서 (-8, 22.5, 40) mm |

- `imu_orientation`은 실물 IMU(MPU6886)에는 없는 **정답 자세**다. 학습을 돕는 용도로만 쓴다.
- 바퀴 무늬를 그리지 않은 차이는 마찰 계수로 메운다. 이런 차이가 Sim-to-Real gap의 한 예다.
- 참고: [MuJoCo Modeling 문서](https://mujoco.readthedocs.io/en/stable/modeling.html)

## 7. MuJoCo에서 시험

- Docker 환경의 JupyterLab(localhost:3000)에서 [`01-test-model-in-mujoco/test_motion.ipynb`](../../workspace/software/01-test-model-in-mujoco/test_motion.ipynb)를 연다.
- 조작

  | 입력 | 동작 |
  |---|---|
  | ↑ / ↓ | 양쪽 모터 명령 +0.1 / −0.1 (최대 ±1.0) |
  | 마우스 | 카메라 회전·확대 |
  | `w` | 와이어프레임 (IMU site가 파란 상자로 보임) |
  | Backspace | 시뮬레이션 리셋 (명령도 0으로) |

- 50 step마다 가속도, 자이로, 자세, 바퀴 위치·속도를 출력한다.
- 손으로 균형을 잡기가 매우 어렵다는 걸 직접 느끼는 게 목적이다.

## 8. 과제: PID로 균형 잡기

- 시뮬레이션에서 PID 제어기로 로봇을 세운다. 위치가 흘러가는 것은 괜찮다. 고전적인 역진자 문제로 본다.
- 원문 참고 영상: "What is a PID Controller?", "How to Tune a PID Controller for an Inverted Pendulum" (Shawn Hymel)
- 풀이 노트북 `solution_simple_pid.ipynb`가 있다. **먼저 직접 짜 보고** 나서 비교한다.

## 9. 다음 편 (Part 2)

CleanRL을 고친 버전으로 PPO 에이전트를 학습해 스스로 균형을 잡게 한다. curriculum learning으로 쉬운 것부터 단계적으로 배우게 한다.

## 원문과 저장소가 다른 곳

| 위치 | 원문 / 주석 | 실제 |
|---|---|---|
| MJCF 경로 | `/workspace/mechanical/freecad/bala2fire/` | `/workspace/mechanical/FreeCAD/bala2-fire/` (대소문자·하이픈 주의) |
| STL 내보내기 위치 | 원본 `meshes/` 폴더 | 원본을 덮어쓰므로 `workspace/nu54/` 수정본 사용 (§5) |
| FreeCAD 버전 | (명시 없음) | `.FCStd`가 1.1로 저장됨. 0.19 등 옛 버전에서는 경고가 나고 깨질 수 있음 ([setup.md](../setup.md)) |
| `test_motion.ipynb` 마지막 셀 주석 | "MJCF timestep time (2 ms)" | MJCF의 timestep은 5 ms |
| 노트북 변수 이름 | `MOTOR_SPEED_*`, "motor speed" | MJCF의 `motor`는 **토크** 액추에이터 (`gear × ctrl` N·m) |

## NU54-DK 섀시와 다른 점 (Part 2.5, #9에서 채울 값)

| 항목 | Bala2 (MJCF) | NU54 섀시 | 출처 |
|---|---|---|---|
| 바퀴 반지름 | 22.5 mm | 40 mm | [hardware.md](../hardware.md) |
| 트랙 폭 | 80 mm | 측정 필요 | #9 |
| 섀시 질량 / 무게중심 높이 | 0.127 kg / 17 mm | 측정 필요 | #9 |
| 바퀴 질량 | 0.017 kg | 측정 필요 | #9 |
| 모터·기어 | N20, 1:30, ±0.05 N·m | JGA25-370, 1:34 추정, 토크 확인 필요 | #9 |
| armature | 5e-8 × 30² | 모터 관성 × 기어비² (추정 필요) | #9 |
| IMU | MPU6886, LCD 아래 | MPU-6050, 위치 측정 필요 | #9 |
| 엔코더 | 바퀴 1회전 420틱 | 748틱(×2) 또는 1496틱(×4) | [hardware.md](../hardware.md) |

## 생각해 볼 점 (답은 rl-notes.md에)

- 시뮬레이션의 `ctrl`은 토크인데, 실물의 TB6612 PWM duty는 무엇에 가까운가? 이 차이가 Part 3에서 어떤 문제를 일으킬까?
- 바퀴가 Bala2보다 거의 두 배 크고 모터가 훨씬 강하다. 같은 정책이나 같은 PID 게인을 그대로 쓸 수 있을까?
- 실물에 없는 `imu_orientation`을 학습에 쓰면, 실물에서는 무엇으로 대신해야 할까?
