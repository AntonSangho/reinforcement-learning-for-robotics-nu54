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
docker run -d --rm --name rl-robotics -e PUID=$(id -u) -e PGID=$(id -g) \
  -p 3000:3000 -p 6006:6006 -v "${PWD}/workspace:/workspace" --shm-size=2g rl-robotics
docker stop rl-robotics   # 끝낼 때 (--rm이라 컨테이너도 지워짐)
```

| 주소 | 용도 |
|---|---|
| <http://localhost:3000/> | WebTop 데스크톱 (JupyterLab, VS Code, 터미널) |
| <http://localhost:6006/> | TensorBoard (`/workspace/software` 아래 로그를 자동으로 읽음) |

- 호스트의 `workspace/`가 컨테이너의 `/workspace`에 마운트됩니다. 노트북에서 저장한 결과는 호스트에 그대로 남습니다.
- 컨테이너 안의 Python 가상환경은 `/opt/rl-env`입니다.
- **`-e PUID=$(id -u) -e PGID=$(id -g)`를 꼭 넣습니다.** 원문 명령(`-it`, PUID 없음)으로 띄우면 컨테이너 사용자 `abc`가 uid 911이 되어, 호스트 사용자 소유인 `workspace/` 파일을 Jupyter에서 저장할 수 없습니다 (Permission denied).
- `workspace/software/runs`는 entrypoint가 root로 만듭니다. 소유자가 root면 학습 로그를 쓸 수 없으므로 한 번 바꿔 둡니다: `docker exec rl-robotics chown $(id -u):$(id -g) /workspace/software/runs`
- `-d`로 띄우면 터미널을 차지하지 않습니다. 상태는 `docker ps`, 로그는 `docker logs rl-robotics`로 봅니다.
- 학습 로그와 체크포인트(`runs/`)와 영상(`videos/`)은 `.gitignore`에 들어 있어 커밋되지 않습니다. 보여줄 결과는 그래프·영상을 골라 블로그 초안이나 `docs/`에 따로 저장합니다.

### 스모크 테스트

```sh
docker run --rm --entrypoint /opt/rl-env/bin/python rl-robotics \
  -c "import mujoco, torch, gymnasium, onnx; print(mujoco.__version__, torch.__version__, gymnasium.__version__, onnx.__version__)"
```

### GPU

`Dockerfile.cpu`는 CPU용 PyTorch를 설치합니다. 정책 네트워크가 작은 MLP이고 시간 대부분을 MuJoCo 시뮬레이션이 쓰기 때문에 GPU 없이도 충분합니다.
속도를 높이려면 GPU보다 병렬 env 수(CPU 코어)를 늘리는 쪽이 효과가 큽니다.

## 2. CAD 환경 (FreeCAD, Part 1 모델 보기 · Part 2.5 내 섀시)

원본 모델 `bala2-fire-simplified.FCStd`는 **FreeCAD 1.1**로 저장되어 있습니다. Ubuntu 22.04 apt 패키지(0.19)로 열면 `Lost link to ... Origin`, `No extension found of type ...` 경고가 나오고, 다시 계산하거나 저장하면 모델이 깨질 수 있습니다.

```sh
sudo apt remove freecad freecad-common freecad-python3 libfreecad-python3-0.19   # 옛 버전 제거
# freecad.org에서 AppImage를 받아서
mv ~/Downloads/FreeCAD_1.1.4-Linux-x86_64-py311.AppImage ~/Applications/
chmod +x ~/Applications/FreeCAD_1.1.4-Linux-x86_64-py311.AppImage
ln -sfn ~/Applications/FreeCAD_1.1.4-Linux-x86_64-py311.AppImage ~/.local/bin/freecad
```

- AppImage 실행에는 `libfuse2`가 필요합니다 (22.04에는 기본 설치됨).
- GUI 없이 스크립트 실행: `~/Applications/FreeCAD_1.1.4-*.AppImage freecadcmd script.py`
- Part 1은 이미 만들어진 STL과 MJCF만 쓰므로 FreeCAD 없이도 진행할 수 있습니다.

## 3. 펌웨어 환경 (Zephyr, Part 3~6)

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
| 2026-10-04 | 같은 호스트, Docker 29.4.1 | 빌드 성공 약 21분, 이미지 19.4 GB. 스모크 테스트 통과: mujoco 3.7.0, torch 2.11.0+cpu, gymnasium 1.2.3, onnx 1.21.0, onnxruntime 1.25.1. Bala2 MJCF 로드 후 200 step(1 s) 정상 |
| 2026-10-04 | 같은 호스트 | FreeCAD 0.19(apt) → 1.1.4 AppImage로 교체, `bala2-fire-simplified.FCStd` 경고 없이 열림 |
| 2026-10-05 | 같은 호스트 | `docker run -d --name rl-robotics ...`로 실행, WebTop(3000)·TensorBoard 2.20.0(6006) 브라우저 접속 확인. 로그의 `dbus ... login1/PolicyKit1 Permission denied`는 무시해도 됨. PUID 없이 띄우면 uid 911이라 `workspace/` 쓰기 불가 → `-e PUID/PGID`로 해결, `runs/` 소유자 root → chown |
