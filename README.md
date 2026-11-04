# 카드 디펜스 (CardDefense)

2026-2학기 컴퓨터 그래픽스 최종 프로젝트. OpenGL 3.3 Core로 만든 1인용 카드 타워 디펜스 게임이다.
카드를 판에 내려놓으면 피규어 말이 떨어져 칸에 놓이고, 10라운드 동안 몰려오는 몹을 막는다.

- 팀: 이현수, 최명준
- 기획: [`docs/`](docs/) (제안서, 밸런스, 작업계획 등)
- 코드는 코딩 시험 때 작성한 코드(직육면체 메시, 계층 변환, 카메라, 키 입력)에서 출발해 기능을 하나씩 늘려 간다.

## 폴더 구조

| 경로 | 내용 |
|---|---|
| `CardDefense/` | Visual Studio 솔루션(`CardDefense.slnx`)과 소스 |
| `CardDefense/CardDefense/main.cpp` | 윈도우 생성, 입력 콜백, 메인 루프 |
| `CardDefense/CardDefense/Game.h/.cpp` | 게임 상태(`struct Game`)와 로직. 렌더링 코드 없음 |
| `CardDefense/CardDefense/Renderer.h/.cpp` | `Game`을 읽어 렌더링. 셰이더·메시·카메라 보유. 게임 로직 변경 없음 |
| `CardDefense/CardDefense/Common.h/.cpp` | `Mesh`·`Object`·`Camera`, 셰이더 로드, 계층 변환(`World`) |
| `CardDefense/CardDefense/shader.vert/.frag` | GLSL 셰이더 (`#version 330 core`) |
| `CardDefense/CardDefense/Util/` | 필요할 때 가져다 쓸 유틸: CSV 읽기, 이징, 마우스 피킹(미구현), BMP 스크린샷 |
| `CardDefense/CardDefense/Data/` | 유닛·몹·라운드 등 수치 CSV |
| `assets/` | 게임 리소스(카드·모델·텍스처·UI)와 디자인 시안. 설명은 [`assets/assets.md`](assets/assets.md) |
| `tools/` | 에셋 변환·시안 생성용 Python 스크립트 |
| `docs/` | 기획 문서 |

## 빌드 환경

- Visual Studio 2026 (플랫폼 도구 집합 v145), x64
- C++ 언어 표준은 Visual Studio 기본값 (C++14)
- GLEW, GLFW, GLM — 저장소에 포함되어 있지 않다. Visual Studio가 찾는 기본 include/lib 경로에 설치해야 한다.
- 링크: `opengl32.lib`, `glew32.lib`, `glfw3.lib`

## 실행

1. `CardDefense/CardDefense.slnx`를 Visual Studio로 연다.
2. x64 / Debug 또는 Release로 빌드하고 실행한다.
3. 작업 디렉터리는 프로젝트 폴더(`CardDefense/CardDefense/`)여야 한다. 셰이더와 CSV(`Data/`)는 이 폴더 기준이고, 리소스는 `../../assets/`에 있다.
4. Debug는 GLFW DLL(`glfw3dll.lib` + `glfw3.dll`), Release는 정적 GLFW(`glfw3.lib`)로 링크한다.

## 저장소에 없는 것

- 빌드 결과물(`x64/`, `.pch` 등)
- `assets/design_preview/*.svg`: PNG를 내장한 디자인 시안이라 크다(합계 약 200MB). 같은 시안의 PNG·JPG 미리보기는 포함되어 있다.
