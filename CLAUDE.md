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

사용자가 중간에 끝내도 1~5는 한다 (세션 1 뒤 기록 누락 사례). 마지막에 다음 세션 시작 방법을 알려 준다.

1. Issue 체크리스트 갱신 (`gh issue edit` / 완료 시 close)
2. README 진행표 상태 (⬜ 🟡 ✅)
3. `docs/00_progress.md`에 날짜별 기록 추가
4. 회고 `docs/retros/YYYY-MM-DD_sessionNN_<주제>.md` (형식은 기존 회고를 따른다)
5. 메모리와 이 파일을 개선한다
6. 커밋·push는 사용자가 요청할 때. 커밋 메시지에는 `Co-Authored-By: Claude` 서명을 넣는다 (nu54v-dk와 다름)

## 저장소 규칙

- 원본 폴더(`workspace/software/0N-*`, `workspace/mechanical/`, `Dockerfile.*`, `scripts/`)는 **수정하지 않는다** (upstream merge 용이). 고칠 때는 `workspace/nu54/`로 복사해서 고친다
- 내 섀시 모델·학습 코드: `workspace/nu54/` · 펌웨어: `firmware/` (HW 브링업 #9부터)
- 내 로봇 모델: `workspace/nu54/FreeCAD/scripts/build_nu54_model.py`(치수 상수) → `nu54/nu54.FCStd`·`meshes/` → `nu54/nu54.xml`. 값의 근거는 `hardware.md` §2. FreeCAD 좌표는 Bala2와 같이 +X 왼쪽 / **−Y 앞** (export `rpy_degrees=(0,0,90)`으로 MuJoCo 앞 = +X). 섀시 관성은 `inertia_utils`가 아니라 부품별 합 (무게중심이 기하 중심과 멀면 크게 틀림)
- FreeCAD는 `~/Applications/FreeCAD_1.1.4-*.AppImage` (`freecad`). 헤드리스는 `<AppImage> freecadcmd script.py`, 이때 `__name__`은 `'__main__'`이 아니라 파일 이름
- 학습 노트북 복사본은 `/workspace/software`로 chdir한다 (`train()`은 `runs/`를 작업 폴더에 쓰고, TensorBoard는 `/workspace/software`만 봄)
- `runs/`는 gitignore → 남길 결과(CSV·best model·`actor.onnx`+`.data`)는 `workspace/nu54/0N-*/results/`로 복사한다. 학습 끝 알림은 Claude가 `*_final.cleanrl_model` 생성을 백그라운드로 기다려 알린다
- 물리 timestep은 MJCF `<option timestep>`(nu54 5 ms)이다. 노트북의 `timestep=`은 실시간 표시용 sleep. 원본 `evaluate()`는 래퍼 때문에 `max_steps`를 못 읽어 `num_steps`(2048)에서 끊으므로 `replace(config, num_steps=10_000)`로 평가한다 (세션 14). ONNX와 같은 행동은 `agent.actor_mean`(평균), `evaluate()` 영상은 샘플 행동
- 이미지: `docs/images/`, 영어 소문자·하이픈/밑줄 이름
- 블로그 초안: `blog/drafts/` (**git에 넣지 않음**, `.gitignore`). 붙여 넣기용 `.txt`도 같이 만든다. 게시 후 URL은 README 진행표에 링크
- 학습 노트 `docs/rl-notes.md`: **답을 대신 써 넣지 않는다.** 질문, 힌트, 실험 제안까지만 한다. 사용자 답에 대한 힌트는 회고의 다음 세션 체크리스트에 적는다
- 값을 바꾸는 실험은 `docs/experiments.md`에 예측(실행 전) → 결과 → 해석. 노트북은 `workspace/nu54/0N-*/`에 복사해서 고친다
- 진행 단계를 안내할 때는 단계 이름 + 파일 경로 + 할 일을 함께 쓴다
- 모든 문서와 대화는 한국어. 코드 주석은 주변 코드 관례를 따른다

## 하드웨어 요약 (자세히: docs/hardware.md)

- MPU-6050(I2C 0x68, Qwiic J5 i2c21) · TB6612FNG · JGA25-370 12 V 170 rpm + 홀 엔코더 11 PPR(기어비 1:34, 1496틱/회전) · 바퀴 실측 지름 75 mm(사양 80), 트랙 폭 245 mm · 3S 18650 · STELA DC-DC (VM ≈ 배터리 전압)
- **엔코더 전원은 3.3 V** (출력이 nRF54L15 핀에 바로 들어감, 5 V 입력 불가)
- **보드 5 V는 VEXT로** (쇼트키 OR라 USB와 같이 꽂아도 됨). **VBAT는 충전 IC의 1셀 배터리 핀**이라 5 V를 넣지 않는다 (2026-11-22 이전)
- **PWM은 P1 포트에서만** → 모터 PWM은 P1.10(P4-8) / P1.14(P4-12) (LED2/LED4 핀). P1.12(VBAT_MON, 100 nF)는 쓰지 않는다
- 헤더는 **P4(왼쪽)와 P2(오른쪽)** 두 개. 핀 위치는 `docs/images/NU54V-DK_pinoutfrom_baram.png`와 회로도 시트 9가 기준 (옛 `nu-54dk_pinmap.png`는 P4의 `1.10`이 빠져 있음). 자유 핀은 P2.00~P2.06 7개 + LED 핀(P1.10, P1.14, P2.09, P2.07) + 입력 전용 버튼 핀. 확정 핀맵: `docs/hardware.md` §3
- **VCOM0를 쓰면 SWD가 죽는다** → CLI는 VCOM1

## 펌웨어 (HW 브링업 #9~)

- Zephyr, NCS v3.4.1, 보드 `nu54v_dk/nrf54l15/cpuapp`. 구조와 스크립트는 `~/projects/nu54v-dk`를 따른다 (그 저장소의 CLAUDE.md 참고)
- 앞 예제를 복사해 모듈을 하나씩 더한다. 모듈은 자기 CLI 명령으로 시험한다
- **플래시 방법은 보드 스위치에 따라 다르다.** `DISABLE_SWD` ON이면 온보드 DAP(`fw flash`)가 `No ACK`로 안 붙는다 → J-Link 케이블을 꽂고 `JLinkExe -device nRF54L15_M33 -if SWD -speed 4000`으로 `loadfile`. J-Link를 꽂으면 `/dev/ttyACM*` 번호가 바뀌므로 CLI는 `/dev/serial/by-id/...-if03`(VCOM1)으로 고른다. **baram-term은 `.../baram-term/.venv/bin/baram-term <포트>`로 직접 띄운다** (VCOM0에 붙이지 않는다)
- 헤더 P2는 두 열(왼쪽 P2.xx, 오른쪽 P1.xx). `P1.0`(=P1.00)은 32 kHz 크리스털이라 신호에 쓰지 않는다 (PWMA는 `P1.10`). P2 포트는 GPIO 인터럽트가 안 된다 (엔코더는 타이머 폴링)
- 모터·엔코더 시험 전에 파워서플라이 ON과 **STELA DC-DC 스위치 J2 ON**을 확인한다 (카운트 0이면 전원부터. 한 모터만 0이면 `motor x 50`을 켜 두고 TB6612 핀 전압, `hardware.md` §3 배선 점검 순서. 모터가 안 돌거나 J-Link 플래시가 `Target voltage too low`면 전원·케이블부터 묻는다)
- 보드 CLI 시험은 `baram-term` 스킬(`baram-ctl`)로 보낸다. 포트를 직접 열지 않는다
- 사용자가 손·눈으로 하는 실측(스톨 전류 등)은 이유를 먼저 설명하고, 명령을 보내기 직전에 알린다. `motor x duty ms`는 그 시간 동안 CLI를 막으므로 구동 시간 뒤에 `enc info`를 읽는다. 스톨은 전류보다 `enc` 틱으로 판단한다
- **`safety` 모듈이 누워 있으면(|pitch| > 45°) 모터를 잠근다.** 벤치 시험은 세워 들고 `safety reset`, 또는 `safety off`(리셋하면 다시 ON). duty 상한 60 %(`motor max n`). trip 확인은 `safety info`의 `count`로 (`baram-ctl read` 버퍼에 옛 trip 줄이 남음)
- 정책은 `workspace/software/rl/onnx_actor_to_c.py`로 `actor.h`를 만들어 쓴다 (순수 C)
- sim ↔ real 관측 일치는 `docs/hardware.md` §4 체크리스트로 확인한다

## 명령

```sh
docker build --build-arg TARGETARCH=$(dpkg --print-architecture) -t rl-robotics -f Dockerfile.cpu .
docker run -d --rm --name rl-robotics -e PUID=$(id -u) -e PGID=$(id -g) \
  -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
docker stop rl-robotics   # 끝낼 때 (--rm이라 컨테이너도 지워짐)
# WebTop http://localhost:3000 · TensorBoard http://localhost:6006
```

- `docker run`은 **저장소 루트에서** (`${PWD}`). 홈에서 띄우면 root 소유 빈 `~/workspace`가 마운트된다 (세션 13). 확인: `docker inspect rl-robotics --format '{{range .Mounts}}{{.Source}}{{end}}'`
- 시뮬레이션 확인 스크립트는 `docker exec -u abc rl-robotics /opt/rl-env/bin/python ...`로 돌린다 (호스트에는 mujoco 없음). **`-u abc`를 빼면 root로 돌아** `runs/`·`__pycache__`가 root 소유가 되고 Jupyter 학습이 `PermissionError`로 멈춘다 (세션 14). 고칠 때는 `docker exec -u root rl-robotics chown -R 1000:1000 <경로>`
