# 실험 기록

값을 바꿔 가며 돌려 본 실험을 Part별로 남깁니다. 개념 질문의 답은 [rl-notes.md](rl-notes.md)에 적고, 이 문서에는 **무엇을 바꿨고 어떻게 됐는지**만 적습니다.

## 기록 규칙

1. **한 번에 값 하나만** 바꾼다. 나머지는 기준값 그대로 둔다
2. 실행 **전에** "예측" 칸을 먼저 채운다. 틀려도 지우지 않는다
3. 실행 후 "결과"에 본 것을, "해석"에 한 줄로 왜 그런지 적는다
4. 스크린샷은 `docs/images/partN-<내용>.png`로 저장하고 결과 칸에 링크한다

---

## Part 1 — PD 제어기 기준선

- 노트북: `workspace/nu54/01-test-model-in-mujoco/solution_simple_pid.ipynb` (원본 복사본)
- 모델: `workspace/mechanical/FreeCAD/bala2-fire/bala2-fire-simplified.xml` (timestep 5 ms)
- 제어: `motor = clamp(KP * pitch + KD * pitch_rate)`, |pitch| > 30°이면 모터 정지
- 외란: MuJoCo 창에서 몸체 더블클릭 → Ctrl + 마우스 오른쪽 드래그

### 기준값

| ALPHA | KP | KD | pitch 출처 | 결과 |
|---|---|---|---|---|
| 0.99 | 7.0 | 0.5 | 상보 필터 | 외란 없이 계속 서 있음 ([스크린샷](images/part1-pid-standing.png)). 외란: |

### 실험

| # | 날짜 | 바꾼 값 | 예측 (실행 전) | 결과 | 해석 |
|---|---|---|---|---|---|
| A | | KD = 0 | | | |
| B | | KP = 20 | | | |
| C | | KP = 2 | | | |
| D | | ALPHA = 0.5 | | | |
| E | | ground-truth pitch 켜기 (주석 해제) | | | |

빈 줄을 더 만들어 직접 생각한 실험(예: KP와 KD를 함께 키우기, ALPHA = 1.0, `TIP_THRESHOLD` 바꾸기)을 추가해도 됩니다.

### 정리 (실험을 마친 뒤)

- 가장 잘 버틴 게인:
- 게인을 맞추며 어려웠던 점:
- 이 경험이 rl-notes.md의 "PID의 한계" 질문과 어떻게 이어지나:
