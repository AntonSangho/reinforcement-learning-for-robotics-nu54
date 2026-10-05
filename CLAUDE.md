# NU54-DK 강화학습 셀프밸런싱 로봇

Shawn Hymel의 RL for Robotics 튜토리얼(Part 1~6)을 NU54-DK(nRF54L15)와 직접 만든 섀시로 따라간다.
**목표는 사용자가 강화학습을 스스로 익히는 것**이다. 결과를 대신 만들어 주기보다 이해를 돕는다.

## 세션 시작

1. `docs/00_progress.md`를 읽는다 (현재 단계, 결정 사항, 다음 할 일)
2. 이번 세션의 GitHub Issue 하나를 정한다 (`gh issue list`). 한 세션에 Issue 하나가 원칙.
   시뮬레이션(#2~#4)과 HW 브링업(#9)은 서로 의존하지 않으므로 번갈아 진행할 수 있다
3. 사전 조건을 점검한다
   - 학습: `docker info` (꺼져 있으면 사용자에게 `! sudo systemctl start docker` 요청), 컨테이너는 아래 "명령"의 `docker run`으로 띄운다 (PUID 없이 띄우면 Jupyter에서 `workspace/` 저장 불가)
   - 펌웨어: `~/ncs` v3.4.1, 보드 연결(`baram-ctl list`)
4. 최근 회고(`docs/retros/`)의 "다음 세션 체크리스트"를 확인한다

## 세션 끝

1. Issue 체크리스트 갱신 (`gh issue edit` / 완료 시 close)
2. README 진행표 상태 (⬜ 🟡 ✅)
3. `docs/00_progress.md`에 날짜별 기록 추가
4. 회고 `docs/retros/YYYY-MM-DD_sessionNN_<주제>.md` (형식은 기존 회고를 따른다)
5. 메모리와 이 파일을 개선한다
6. 커밋·push는 사용자가 요청할 때. 커밋 메시지에는 `Co-Authored-By: Claude` 서명을 넣는다 (nu54v-dk와 다름)

## 저장소 규칙

- 원본 폴더(`workspace/software/0N-*`, `workspace/mechanical/`, `Dockerfile.*`, `scripts/`)는 **수정하지 않는다** (upstream merge 용이). 고칠 때는 `workspace/nu54/`로 복사해서 고친다
- 내 섀시 모델·학습 코드: `workspace/nu54/` · 펌웨어: `firmware/` (HW 브링업 #9부터)
- 이미지: `docs/images/`, 영어 소문자·하이픈/밑줄 이름
- 블로그 초안: `blog/drafts/partN-*.md` (외부 블로그에 게시 후 front matter `published_url`과 README에 링크)
- 학습 노트 `docs/rl-notes.md`: **답을 대신 써 넣지 않는다.** 질문, 힌트, 실험 제안까지만 한다
- 모든 문서와 대화는 한국어. 코드 주석은 주변 코드 관례를 따른다

## 하드웨어 요약 (자세히: docs/hardware.md)

- MPU-6050(I2C 0x68, Qwiic J5 i2c21) · TB6612FNG · JGA25-370 12 V 170 rpm + 홀 엔코더 11 PPR(기어비 1:34 추정, 실측 필요) · 바퀴 80 mm · 3S 18650 · 5 V 벅
- **엔코더 전원은 3.3 V** (출력이 nRF54L15 핀에 바로 들어감, 5 V 입력 불가)
- **PWM은 P1 포트에서만** → 모터 PWM은 P1.10 / P1.14 (LED2/LED4 핀)
- 보드 기능에 묶이지 않은 핀은 P2.00~P2.06 (7개)뿐 → 핀 예산 확인
- **VCOM0를 쓰면 SWD가 죽는다** → CLI는 VCOM1

## 펌웨어 (HW 브링업 #9~)

- Zephyr, NCS v3.4.1, 보드 `nu54v_dk/nrf54l15/cpuapp`. 구조와 스크립트는 `~/projects/nu54v-dk`를 따른다 (그 저장소의 CLAUDE.md 참고)
- 앞 예제를 복사해 모듈을 하나씩 더한다. 모듈은 자기 CLI 명령으로 시험한다
- 보드 CLI 시험은 `baram-term` 스킬(`baram-ctl`)로 보낸다. 포트를 직접 열지 않는다
- 정책은 `workspace/software/rl/onnx_actor_to_c.py`로 `actor.h`를 만들어 쓴다 (순수 C)
- sim ↔ real 관측 일치는 `docs/hardware.md` §4 체크리스트로 확인한다

## 명령

```sh
docker build -t rl-robotics -f Dockerfile.cpu .
docker run -d --rm --name rl-robotics -e PUID=$(id -u) -e PGID=$(id -g) \
  -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
docker stop rl-robotics   # 끝낼 때 (--rm이라 컨테이너도 지워짐)
# WebTop http://localhost:3000 · TensorBoard http://localhost:6006
```
