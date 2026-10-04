# 진행 기록

이 문서는 팀원과 공유하는 핸드오프 문서입니다. **새 작업을 시작하기 전에 이 문서부터 읽고**, 작업이 끝나면 맨 아래에 기록을 추가합니다.
Part별 상태 요약은 [README 진행표](../README.md#진행-상황)에, 세부 체크리스트는 각 Part의 GitHub Issue에 있습니다.

## 지금 상태

- 현재 Part: **0 (저장소·개발 환경 세팅)**
- 다음 할 일: Docker 이미지 빌드·스모크 테스트 → Part 1 시작

## 결정 사항

| 날짜 | 결정 | 이유 |
|---|---|---|
| 2026-10-04 | 하드웨어는 NU54-DK + 직접 만든 섀시 | 원본 Bala2 Fire 대신 보유한 보드·부품 사용 |
| 2026-10-04 | 펌웨어는 Zephyr (NCS v3.4.1), [nu54v-dk](https://github.com/AntonSangho/nu54v-dk) 구조를 따름 | 같은 보드의 브링업 코드(uart/cli/i2c/nvs/ble_nus)를 재사용 |
| 2026-10-04 | Part 1~2는 원본 Bala2 모델로 먼저 진행, Part 2.5에서 내 섀시 모델 | 개념을 먼저 익히고, 모델링 오차와 학습 문제를 분리 |
| 2026-10-04 | 원본 폴더(`workspace/software`, `workspace/mechanical`, `Dockerfile.*`)는 수정하지 않음 | `upstream` 업데이트를 merge하기 쉽게 |
| 2026-10-04 | 블로그는 외부 블로그에 게시, 저장소에는 `blog/drafts/` 초안 | 팀원은 저장소에서 바로 읽고, 게시본 URL은 README에 링크 |
| 2026-10-04 | Part 6 원격 조종은 BLE + Web Bluetooth | nRF54L15에는 WiFi가 없음 |

## 기록

### 2026-10-04 — Part 0 세팅

- `upstream` remote 추가 (`ShawnHymel/reinforcement-learning-for-robotics`)
- README 한국어 섹션·진행표, `docs/`, `blog/drafts/`, Issue 템플릿 추가
- 원본 하드웨어 분석: Bala2 Fire는 IMU가 Fire 코어(MPU6886)에, 모터·엔코더는 베이스의 STM32(I2C 0x3A)에 있음 → NU54-DK에서는 IMU, 모터 드라이버, 엔코더 입력을 모두 새로 구성해야 함
- `rl/onnx_actor_to_c.py`가 만드는 `actor.h`는 순수 C(`tanhf`만 사용)라서 Zephyr에서 그대로 쓸 수 있음

<!-- 새 기록은 이 아래에 추가 -->
