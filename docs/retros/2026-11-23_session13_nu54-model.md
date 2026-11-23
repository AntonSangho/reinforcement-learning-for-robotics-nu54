# 세션 13 회고 — 내 섀시 모델링: 치수 → FreeCAD → MJCF → PPO 준비 (2026-11-23)

- 범위: Issue [#4](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/4) (Part 2.5). Part 2(#3)는 건너뛰기로 결정
- 커밋: 하지 않음 (사용자 요청 시)

## 1. 한 일

| 결과물 | 내용 |
|---|---|
| 결정 | Part 2(Bala2 학습)는 건너뜀, 모델링은 FreeCAD로 직접 (`00_progress.md` 결정 사항) |
| FreeCAD | 1.1.4 AppImage 설치, `freecadcmd`로 튜토리얼 스크립트 동작 확인 |
| 치수 | `hardware.md` §2-1. 원점 = 바퀴 축 중심, 높이는 바닥에서 재고 37.5 mm를 뺌 |
| 모델 | `workspace/nu54/FreeCAD/scripts/build_nu54_model.py` → `nu54/nu54.FCStd`, STL 3개 |
| 질량·관성 | `hardware.md` §2-2. 저울 없이 사양표 + 부피×밀도, 칼날 무게중심 실측. 관성은 부품별 합 |
| MJCF | `nu54/nu54.xml`. 모터 gear·damping·frictionloss를 사양과 실측에서 유도, MuJoCo로 무게중심·최고 속도 확인 |
| 노트북 | `workspace/nu54/01-*/test_motion_nu54`, `solution_simple_pid_nu54`, `02-*/train_with_ppo_curriculum_nu54` + env 복사 |
| Issue | #4 체크리스트 4개, 코멘트 4개. #3 건너뜀 코멘트 |

## 2. 잘된 점

- **모델을 스크립트로 만들었다.** 치수 상수만 바꾸면 다시 만들어진다. GUI는 사용자가 사진과 대조하는 데만 써서 1~2시간이 몇 분이 됐다.
- **튜토리얼 도구의 가정을 숫자로 검증했다.** `inertia_utils`를 그대로 쓰지 않고 부품별 계산과 비교해 pitch 관성이 1.8배 다른 것을 찾았다. Bala2에서는 기하 중심과 무게중심이 가까워서 드러나지 않던 가정이다.
- **모터 모델을 실측과 맞췄다.** 역기전력을 joint damping으로 넣고, ctrl 1의 최고 속도가 실측 10.8 rad/s와 같은지 시뮬레이션으로 확인했다. frictionloss가 최고 속도를 깎는 것도 이 확인에서 잡았다(9.4 → 감쇠 재조정).
- **느낌을 숫자로 확인했다.** "더 어렵다"는 손 조종 소감을 같은 입력의 넘어지는 시간으로 비교해서, 실제로는 3배 느리다는 것과 어렵게 느낀 원인(데드존, 키 반복)을 나눴다.
- **이상한 치수를 조립 관계로 걸렀다.** 기둥 높이 4 mm, IMU 왼쪽 93 mm(기둥 바깥), 배터리와 모터 겹침, 윗판 37 mm를 질문으로 확인해 모두 풀었다.

## 3. 시간이 걸린 점과 원인

| 문제 | 원인 | 다음에는 |
|---|---|---|
| 첫 헤드리스 실행이 아무것도 안 함 | `freecadcmd`는 스크립트의 `__name__`을 파일 이름으로 둠 | `if __name__ in ('__main__', '<파일명>')` (스크립트에 반영) |
| 인자 이름 틀림 | 자막의 "rpy"를 그대로 씀. 실제는 `rpy_degrees` | 함수 시그니처부터 읽기 |
| FreeCAD 앞 방향을 +Y로 잘못 적음 | 확인 전에 썼음. Bala2 파일의 `wheel_left`가 +X → 90° 회전하면 앞 = −Y | 좌표 규칙은 원본 파일의 Placement로 먼저 확인 |
| MJCF 일괄 치환 실패 1번 | 같은 문자열이 두 번 나옴(바퀴 2개), 파일이 CRLF | 치환 횟수를 assert, CRLF는 `newline='\r\n'`으로 유지 |
| 문서 갱신 명령이 2분 멈춤 | 명령 앞에 실수로 넣은 `cat >> /dev/null`이 입력을 기다림 | 백그라운드로 넘어가면 바로 TaskStop, 바뀐 것 없는지 확인 후 재실행 |
| 컨테이너가 `~/workspace`를 마운트 | 홈 폴더에서 `docker run` → `${PWD}`가 달라짐. Docker가 root 소유 빈 폴더를 만듦 | `docker run`은 저장소 루트에서. `docker inspect`로 마운트 확인 |
| `sudo rm` 안 됨 | 앱 터미널 패널은 sudo 막힘 (메모리에 있었음) | sudo가 필요하면 처음부터 일반 터미널(Ctrl+Alt+T)로 안내 |

## 4. 발표(11/25)까지의 범위 판단

영상 스크립트(`docs/youtube/`) 기준. Part 3 영상의 흐름: ONNX → C(`actor.h`) → IMU 보정 → 5 ms 루프(IMU, 상보필터 pitch, 엔코더 → rad/s, 45° 넘어짐 정지, 관측 4개 → actor → clamp → duty, 모터 방향 부호) → **처음엔 잠깐 서다가 굴러가 넘어짐** → 펌웨어 보정 4개(pitch 오프셋, 모터 부스트 ×1.5, 행동 데드밴드 0.02, 행동 저역통과 α 0.9)로 훨씬 잘 섬 → "이건 땜질이라 Part 4 DR로 간다"

| Part 3 영상 단계 | 우리 상태 |
|---|---|
| IMU 보정 | 됨 (자이로 바이어스 nvs, 가속도 오프셋, 세션 6·10). 직립 −2.7° 오프셋은 미확인 → 영상의 pitch 오프셋과 같은 문제 |
| 엔코더 → rad/s, 방향 부호 | 됨 (1496틱, 부호 맞춤). 5 ms에 1틱 = 0.84 rad/s 양자화 (Bala2 2.99보다 나음) |
| 45° 넘어짐 정지 | 됨 (`safety` 모듈) |
| 모터 부스트 (배터리 전압 처짐) | 모델에 미리 반영: gear·damping을 배터리 전압 실측 속도(10.8 rad/s @60 %)에 맞춤 |
| 데드밴드 | 모델에 정지마찰(ctrl 0.12)로 반영 → 정책이 이걸 알고 학습 |
| 5 ms 루프, `actor.h` | 없음 → `policy` 모듈 새로 (nRF54L15 M33+FPU라 추론은 0.1 ms 수준, I2C 읽기 약 0.4 ms) |
| 행동 저역통과, pitch 오프셋 | 실물에서 필요하면 CLI 파라미터로 |

| Part | 지금 | 11/25까지 |
|---|---|---|
| 1 CAD → MuJoCo | Bala2로 완료, `rl-notes` 답만 남음 | 됨 |
| 2 PPO 학습 | 건너뛰고 내 모델로 준비 완료 | **됨** (약 40분) |
| 2.5 내 섀시 모델 | 모델 완료, 질량은 추정 | **됨** |
| 3 실물 배포 | 위 표처럼 절반 이상 준비됨, `policy` 모듈만 없음 | **됨 (반나절) + 튜닝은 도전**. 영상도 첫 시도는 실패 |
| 4 Domain randomization | 원본 코드 있음 | Part 3에서 격차가 보일 때 (학습 1시간+) |
| 5 명령 추가 | — | 안 됨 (영상도 학습에 몇 시간) |
| 6 원격 조종 앱 | — | 안 됨 (발표에서는 계획으로) |

- 발표의 최소 결과: 내 로봇의 디지털 트윈에서 PPO 정책이 섬 (시뮬레이션 영상 + TensorBoard) + HW 브링업
- 목표 결과: 같은 정책이 실물에서 몇 초라도 섬. 실패해도 sim과 real의 관측값(pitch, 바퀴 속도, 행동)을 나란히 보여 주면 Part 3 영상과 같은 결론("sim2real 격차 → DR")을 직접 보인 것이 됨

## 5. 다음 세션 체크리스트

- [ ] 컨테이너는 **저장소 루트에서** 띄우기 (`docker inspect rl-robotics`로 `/workspace` 마운트 확인)
- [ ] 학습 전에 `docs/experiments.md`에 예측 적기 (질문 3개: 50초를 버틸지, 정책이 쓰는 ctrl 크기와 데드존 0.12의 관계, 2단계에서 균형이 잠깐 무너질지)
- [ ] `train_with_ppo_curriculum_nu54.ipynb` 1·2단계 실행 (약 40분), TensorBoard `nu54` 필터, 평가 영상 녹화(발표용), ONNX export
- [ ] (Part 3, #5) `onnx_actor_to_c.py`로 `actor.h` → 펌웨어 `policy` 모듈: 5 ms 루프에서 상보필터 pitch(α 0.99)·pitch_rate·바퀴 속도 → actor → duty(ctrl × 60 %). safety 모듈은 그대로 둠. 관측 일치는 `hardware.md` §4 체크리스트로
- [ ] 질문 (`rl-notes`에 답하지 말고 생각만): 무게중심이 높은 로봇이 왜 더 천천히 넘어질까 (`√(m g l / I)`, 연필과 빗자루). pitch 관성을 크게 잡은 모델에서 학습한 정책은 실물에서 어떻게 될까
- [ ] 저울이 생기면 바퀴·섀시 질량을 재서 `hardware.md` §2-2, `nu54.xml` 갱신
- [ ] (선택) 일반 터미널에서 `sudo rm -r ~/workspace`
