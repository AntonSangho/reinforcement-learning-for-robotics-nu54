# 개발 환경 세팅

## 1. 학습 환경 (Docker, Part 1~5)

원본 튜토리얼과 같은 Docker 이미지를 씁니다. 브라우저 데스크톱(WebTop) 안에서 JupyterLab, VS Code, TensorBoard를 엽니다.

### 준비

- Docker Engine이 실행 중이어야 합니다.

  ```sh
  sudo systemctl start docker        # 꺼져 있으면
  sudo systemctl enable docker       # (선택) 부팅 시 자동 시작
  docker info                        # 동작 확인
  ```

- 사용자가 `docker` 그룹에 있어야 `sudo` 없이 실행할 수 있습니다 (`groups`로 확인).

### 빌드와 실행

저장소 루트에서:

```sh
docker build -t rl-robotics -f Dockerfile.cpu .
docker run -it --rm -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
```

| 주소 | 용도 |
|---|---|
| <http://localhost:3000/> | WebTop 데스크톱 (JupyterLab, VS Code, 터미널) |
| <http://localhost:6006/> | TensorBoard (`/workspace/software` 아래 로그를 자동으로 읽음) |

- 호스트의 `workspace/`가 컨테이너의 `/workspace`에 마운트됩니다. 노트북에서 저장한 결과는 호스트에 그대로 남습니다.
- 컨테이너 안의 Python 가상환경은 `/opt/rl-env`입니다.
- 학습 로그와 체크포인트(`runs/`)와 영상(`videos/`)은 `.gitignore`에 들어 있어 커밋되지 않습니다. 보여줄 결과는 그래프·영상을 골라 블로그 초안이나 `docs/`에 따로 저장합니다.

### 스모크 테스트

```sh
docker run --rm --entrypoint /opt/rl-env/bin/python rl-robotics \
  -c "import mujoco, torch, gymnasium, onnx; print(mujoco.__version__, torch.__version__, gymnasium.__version__, onnx.__version__)"
```

### GPU

`Dockerfile.cpu`는 CPU용 PyTorch를 설치합니다. 정책 네트워크가 작은 MLP이고 시간 대부분을 MuJoCo 시뮬레이션이 쓰기 때문에 GPU 없이도 충분합니다.
속도를 높이려면 GPU보다 병렬 env 수(CPU 코어)를 늘리는 쪽이 효과가 큽니다.

## 2. 펌웨어 환경 (Zephyr, Part 3~6)

NU54-DK 펌웨어는 [nu54v-dk](https://github.com/AntonSangho/nu54v-dk)와 같은 환경을 씁니다. 자세한 내용은 그 저장소의 `docs/03_build_debug_env.md`를 봅니다.

| 항목 | 버전 |
|---|---|
| nRF Connect SDK | v3.4.1 (`~/ncs`) |
| 보드 타깃 | `nu54v_dk/nrf54l15/cpuapp` |
| 시리얼 | VCOM1 = cli (baram-term) |

Part 3을 시작할 때 `firmware/` 폴더를 만들고 이 문서에 빌드 방법을 추가합니다.

## 실측 기록

| 날짜 | 호스트 | 결과 |
|---|---|---|
| 2026-10-04 | Ubuntu, 20코어, 62 GB RAM, RTX 4060 Laptop | Docker 데몬이 꺼져 있어 빌드 대기 중 |
