# 카드 디펜스 (CardDefense)

2026-2학기 컴퓨터 그래픽스 최종 프로젝트. OpenGL 3.3 Core로 만든 1인용 카드 타워 디펜스 게임이다.
카드를 판에 내려놓으면 피규어 말이 떨어져 칸에 놓이고, 10라운드 동안 몰려오는 몹을 막는다.

- 팀: 이현수, 최명준
- 기획: [`docs/`](docs/) (제안서, 밸런스 등)
- **일정: [`docs/카드디펜스-작업계획.md`](docs/카드디펜스-작업계획.md)**, 진행 상황은 [마일스톤](https://github.com/HyeonSuuuuu/CardDefense/milestones)·[이슈](https://github.com/HyeonSuuuuu/CardDefense/issues)
- 코드는 코딩 시험 때 작성한 코드(직육면체 메시, 카메라, 키 입력)에서 출발해 기능을 하나씩 늘려 간다.

## 내 작업 보기

1. 받기: `git pull`
2. 이번 주 할 일: [마일스톤](https://github.com/HyeonSuuuuu/CardDefense/milestones)에서 이번 주를 연다. 마감일 순서는 이슈 제목 앞의 `[날짜]`.
3. 내 이슈만 보기: [이현수](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EC%9D%B4%ED%98%84%EC%88%98) · [최명준](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EC%B5%9C%EB%AA%85%EC%A4%80) · [공동](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EA%B3%B5%EB%8F%99)
4. 코드에서 내 자리 찾기: `TODO(이름)` 검색 (예: `Game.cpp`의 `TODO(최명준)`)
5. 끝나면 커밋 메시지에 `fix #이슈번호` → push하면 이슈가 닫힌다.
6. 전체 흐름과 규칙(점검일, 밀릴 때 빼는 순서): [`docs/카드디펜스-작업계획.md`](docs/카드디펜스-작업계획.md)

## 폴더 구조

| 경로 | 내용 |
|---|---|
| `CardDefense/` | Visual Studio 솔루션(`CardDefense.slnx`)과 소스 |
| `CardDefense/CardDefense/main.cpp` | 윈도우 생성, 입력 콜백, 메인 루프 |
| `CardDefense/CardDefense/Game.h/.cpp` | 게임 상태(`struct Game`)와 로직. 렌더링 코드 없음 |
| `CardDefense/CardDefense/Renderer.h/.cpp` | `Game`을 읽어 렌더링. 셰이더·메시·카메라 보유. 게임 로직 변경 없음 |
| `CardDefense/CardDefense/Common.h/.cpp` | `Mesh`·`Camera`, 셰이더 로드, 메시 생성(축·큐브), uniform 설정. 정점 = 위치·법선·UV |
| `CardDefense/CardDefense/shader.vert/.frag` | GLSL 셰이더 (`#version 330 core`) |
| `CardDefense/CardDefense/Util/` | 필요할 때 가져다 쓸 유틸: OBJ·텍스처 읽기(stb_image), CSV 읽기, 이징, 마우스 피킹(미구현), BMP 스크린샷 |
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
3. 빌드하면 셰이더, `Data/`, `assets/obj`·`assets/textures`가 exe 폴더(`x64/Debug` 또는 `x64/Release`)로 복사되고, VS 실행 위치도 exe 폴더다. 코드의 경로는 exe 폴더 기준이다 (`shader.vert`, `Data/...`, `assets/obj/...`).
4. Debug는 GLFW DLL(`glfw3dll.lib` + `glfw3.dll`), Release는 정적 GLFW(`glfw3.lib`)로 링크한다.

## 저장소에 없는 것

- 빌드 결과물(`x64/`, `.pch` 등)
- `assets/design_preview/*.svg`: PNG를 내장한 디자인 시안이라 크다(합계 약 200MB). 같은 시안의 PNG·JPG 미리보기는 포함되어 있다.
