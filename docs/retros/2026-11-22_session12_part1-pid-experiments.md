# 세션 12 회고 — Part 1 PID 외란 실험 A~E (2026-11-22)

- 범위: Issue [#2](https://github.com/AntonSangho/reinforcement-learning-for-robotics-nu54/issues/2) 남은 항목 (세션 2 회고 §4, 세션 11 회고 §4 마지막 줄)
- 커밋: 사용자 요청으로 이 세션 끝에

## 1. 한 일

| 결과물 | 내용 |
|---|---|
| 학습 환경 복구 | 이미지가 지워져 있었음. 루트 디스크 11 → 30 GB 확보, `docker-buildx` 설치 후 BuildKit으로 재빌드 (19.7 GB), 컨테이너 실행 |
| 노트북 | `workspace/nu54/01-test-model-in-mujoco/solution_simple_pid.ipynb`에 키 반복 입력 무시 (`KEY_DEBOUNCE` 0.3 s) |
| 실험 | 기준 외란 + A~E, 예측(사용자) → 결과 → 해석. `docs/experiments.md` |
| 해석·정리 | 사용자 요청으로 Claude가 초안 (표시해 둠) |
| rl-notes 힌트 | Part 1 질문 5개에 MJCF 줄 번호와 힌트 (아래 §4). 답은 쓰지 않음 |
| Issue #2 | 체크리스트(PID 실험 완료, 선택 실험 F 추가), 결과 코멘트 |

## 2. 잘된 점

- **예측을 반응까지 받았다.** 처음 예측은 "KD를 줄이면 반응이 둔해진다"처럼 값의 의미만 있었다. "그래서 로봇은 어떻게 될까"를 선택지로 다시 받아서 결과와 비교할 수 있었다. 대부분 빗나갔고(1~3 N → 0.05 N, A는 오히려 버팀), 그 차이가 해석의 재료가 됐다.
- **이상한 결과는 화면 없이 재현해서 버그인지 먼저 가렸다.** "밀면 쭉 가다 넘어짐"을 노트북 버그가 아닌 제어기 동작으로 확인한 뒤 원인을 질문으로 돌려줬다. 답(추정 오류)은 말하지 않았고, 사용자가 `pitch (filtered)` ≈ 0과 실제 기울기를 비교해서 직접 봤다.
- **같은 힘으로 비교했다.** 기준이 가장 작은 단계(0.05 N)에서도 넘어져서 그 값을 고정하고, 외란 없이 버티는 시간도 A 이후 30초로 늘렸다.

## 3. 시간이 걸린 점과 원인

| 문제 | 원인 | 다음에는 |
|---|---|---|
| 이미지 재빌드 3번 (약 1시간) | 이미지가 지워져 있었고 디스크가 모자람. buildx가 없어 레거시 빌더가 `TARGETARCH` 빈 값, VOLUME `/config` 쓰기 소실로 실패 | 세션 시작 때 `docker images rl-robotics`도 확인. 빌드는 `docker buildx build --load`, 여유 35 GB 이상(이미지 20 + 캐시 15), 빌드 뒤 `docker builder prune -af` |
| 빌드 직후 루트 24 MB | BuildKit 캐시가 이미지와 거의 같은 크기로 남음 | 위와 같음 |
| push force가 저절로 바뀜 | 뷰어가 키 반복도 `key_callback`으로 넘기고, VNC에서 반복이 잘 생김 | 노트북에 0.3 s 디바운스 (해결) |
| JupyterLab "File Changed" 창 | 열어 둔 노트북을 Claude가 디스크에서 고침 | 노트북을 고치기 전에 알리고, Revert를 누르라고 안내 |
| A 결과의 조건이 애매함 | 기준을 다시 본 것과 KD = 0 결과를 한 메시지에 번호만 달아 보냄 | 결과를 받으면 어떤 설정인지 먼저 확인 (이번엔 질문으로 가림) |

## 4. 다음 세션 체크리스트

- [ ] 컨테이너 띄우기 (`docker images rl-robotics`부터 확인), `docs/experiments.md`의 Claude 초안 해석 검토·수정
- [ ] (선택) 실험 F: KP = 20 + 진짜 pitch. 버티면 B 해석("틀린 추정에 세게 반응")이 맞음
- [ ] `rl-notes.md` Part 1 답을 직접 쓰기. 힌트 (파일 `workspace/mechanical/FreeCAD/bala2-fire/bala2-fire-simplified.xml`)
  - actuator·sensor (199~240행): `<motor>`는 어느 joint에 붙나? 센서 7개 중 실제 로봇에 있는 것과 시뮬레이터만 아는 것은? (233행 위 주석, 실험 E)
  - freejoint 자유도 (115행 vs 157~162행 hinge `axis="0 1 0"`): 옮기기와 돌리기로 나눠 세기
  - `<motor>`의 ctrl (211행 위 주석): `gear * ctrl`의 단위는? E에서 "일정한 속도로 굴러감"과 연결
  - timestep (27행): 잘게 자르면 얻는 것과 잃는 것. 실험 D의 고리가 50 ms마다 돈다면?
  - PID의 한계: `experiments.md` 정리 세 줄을 내 말로
- [ ] Part 1 모델 구조 살펴보기 (FCStd → URDF → MJCF), Issue #2 닫기, 블로그 초안
- [ ] 루트 디스크 여유 9.4 GB: Part 2 학습 로그가 쌓이기 전에 더 정리할지 정하기
