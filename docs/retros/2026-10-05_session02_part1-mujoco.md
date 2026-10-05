# 회고: 세션 2 — Part 0 마무리, Part 1 시작 (2026-10-05)

- 범위: Issue [#1](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/1) 닫기 → Issue [#2](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/2) Part 1 절반
- 커밋: (이번 세션 끝에 사용자 요청 시)
- 다음 세션: #2 남은 항목(PID 외란·게인 실험, rl-notes 질문) → #2 닫기

## 1. 한 일

| 결과물 | 내용 |
|---|---|
| Part 0 완료 | WebTop·TensorBoard 접속 확인, Issue #1 닫음, 진행표 ✅ |
| 컨테이너 실행 방식 | `-d --rm --name rl-robotics -e PUID/PGID`. uid 911 쓰기 불가 문제와 `runs/` root 소유 문제 해결 |
| Part 1 | `test_motion.ipynb` 수동 조종, PID 노트북 복사본 실행(기본 게인으로 서 있음) |
| 학습 기록 | `rl-notes.md` Part 1 첫 답, `docs/experiments.md` 새로 만듦 |
| 정리 | 지난 세션의 기록 누락분(Docker 빌드, FreeCAD 교체, Part 1 요약)을 `00_progress.md`에 채움, 이미지 이름 규칙에 맞게 변경 |

## 2. 잘된 점

- **저장 전에 권한을 확인했다.** 복사본을 만들면서 컨테이너 사용자로 쓰기 시험을 해 보고, Jupyter에서 저장이 실패하기 전에 PUID 문제를 찾았다. Part 2 학습 로그(`runs/`) 문제도 같이 막았다.
- **에러를 화면이 아니라 파일로 확인했다.** `NameError`의 원인을 저장된 노트북의 실행 번호(셀 3과 4가 둘 다 `[3]`)로 찾았고, 같은 방법으로 실수로 들어간 `----`도 찾았다.
- **결과를 그냥 넘기지 않았다.** "계속 서 있다"는 결과에 대해, 제어 없이도 서 있다는 것(`test_motion` 스크린샷의 기울기 ≈ 0)을 근거로 외란 테스트가 필요하다는 질문을 남겼다.

## 3. 시간이 걸린 점과 원인

| 문제 | 원인 | 다음에는 |
|---|---|---|
| 지난 세션 작업이 기록 없이 남아 있었다 (커밋 안 됨, progress·회고 없음) | 세션을 마무리 절차 없이 끝냈다 | 중간에 끝내도 "세션 끝" 절차의 1~4는 한다. 커밋은 사용자에게 묻는다 |
| 원문 `docker run` 명령으로는 노트북을 저장할 수 없었다 | linuxserver 이미지의 기본 uid가 911 | 컨테이너를 띄우면 쓰기 시험을 먼저 한다 (지금은 명령에 PUID를 넣어 해결) |
| "4단계가 어디냐"는 질문이 나왔다 | 진행 순서를 표로만 주고, 다음 단계마다 파일 경로를 다시 짚지 않았다 | 단계를 넘길 때 **단계 이름 + 파일 경로 + 할 일**을 함께 쓴다 |
| `NameError: clamp` | 커널이 재시작된 뒤 중간 셀부터 실행했다 | 막히면 먼저 **Restart Kernel and Run All** |

## 4. 다음 세션 체크리스트

- [ ] 컨테이너 띄우기 (README의 `docker run -d ... -e PUID ...` 명령), WebTop 새로고침
- [ ] `docs/images/part0_simulation_with_docker.png` 이름 정하기 (내용은 Part 1 `test_motion` 화면, 제안: `part1-test-motion.png`)
- [ ] PID 기준값으로 **외란 테스트** → `experiments.md` 기준값 "외란" 칸
- [ ] 실험 A~E: 예측을 먼저 적고 실행
- [ ] `rl-notes.md` Part 1 다시 보기 (답은 직접, 아래는 힌트만)
  - 첫 질문에서 `actuator`, `sensor`가 아직 비어 있다
  - freejoint: 상자 하나가 공간에서 움직이는 방법을 모두 세어 보면 몇 가지일까? MuJoCo 문서의 `freejoint`와 `joint type="hinge"`를 비교해 보자
  - `<motor>`의 ctrl: XML 211행의 속성과 MuJoCo 문서의 actuator/motor 설명을 함께 본다. 출력은 힘(토크)일까, 속도일까?
  - 위 화살표 예측("앞")은 실제로 맞았나?
  - timestep: "잘게 자른다"까지는 적었다. 잘게 자르면 무엇이 좋아지고, 무엇을 잃을까? (정확도, 계산 시간, 발산)
  - PID의 한계: 실험 A~E를 마친 뒤 원문 표현이 아니라 **내 실험 결과로** 다시 써 보기
- [ ] 끝나면 Issue #2 닫기, 블로그 초안 `blog/drafts/part1-cad-to-mujoco.md`
