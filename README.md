# 카드 디펜스 (CardDefense)

2026-2학기 컴퓨터 그래픽스 최종 프로젝트. OpenGL 3.3 Core로 만든 1인용 카드 타워 디펜스 게임이다.
카드를 판에 내려놓으면 피규어 말이 떨어져 칸에 놓이고, 10라운드 동안 몰려오는 몹을 막는다.

- 팀: 이현수 (최명준은 2026-11-20 이탈, 이후 1인 진행)
- 기획: [`docs/`](docs/) (제안서, 밸런스 등)
- **일정: [`docs/카드디펜스-작업계획.md`](docs/카드디펜스-작업계획.md)**, 진행 상황은 [마일스톤](https://github.com/HyeonSuuuuu/CardDefense/milestones)·[이슈](https://github.com/HyeonSuuuuu/CardDefense/issues)
- 코드는 코딩 시험 때 작성한 코드(직육면체 메시, 카메라, 키 입력)에서 출발해 기능을 하나씩 늘려 간다.

## 내 작업 보기

1. 받기: `git pull`
2. 이번 주 할 일: [마일스톤](https://github.com/HyeonSuuuuu/CardDefense/milestones)에서 이번 주를 연다. 마감일 순서는 이슈 제목 앞의 `[날짜]`.
3. 내 이슈만 보기: [이현수](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EC%9D%B4%ED%98%84%EC%88%98) · [최명준](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EC%B5%9C%EB%AA%85%EC%A4%80) · [공동](https://github.com/HyeonSuuuuu/CardDefense/issues?q=is%3Aopen+label%3A%EA%B3%B5%EB%8F%99)
4. 끝나면 커밋 메시지에 `fix #이슈번호` → push하면 이슈가 닫힌다.
5. 전체 흐름과 규칙(점검일, 밀릴 때 빼는 순서): [`docs/카드디펜스-작업계획.md`](docs/카드디펜스-작업계획.md)

## 조작

| 입력 | 동작 |
|---|---|
| 손패 카드를 왼쪽 버튼으로 끌어 판에 놓기 | 유닛 카드: 안쪽 칸에 말 배치 (말이 있으면 교체). Start/Exit/Retry/Title 카드: 화면 전환 |
| 주문 카드를 판에 끌어 놓기 | 운석 주사위: 그 칸 주변 몹에 피해(전투 중) / 전투의 북: 주변 3×3 말 공격 속도 ×1.5, 8초 / 칩 주머니: 칩 받기 |
| 손패 카드를 팩에 끌어 놓기 | 카드 팔기 (1칩) |
| 팩 클릭 / B | 부스터 구매 (5칩, 정비 중) |
| 턴 종료 클릭 / Space | 전투 시작 |
| 오른쪽 버튼 드래그 / 휠 / R | 카메라 회전 / 줌 / 처음 시점 |
| T | 툰 셰이딩 ↔ 퐁 셰이딩 |
| G | 그림자(그림자 맵) 끄고 켜기 |
| F1 / F2 / F3 / F4 | 발표용: 다음 라운드로 / 칩 +50 / 보스 라운드(10)로 / 소환·피격·처치 이펙트 다시 보기 (게임 중에만) |
| ESC, Q | 종료 |

## 폴더 구조

| 경로 | 내용 |
|---|---|
| `CardDefense/` | Visual Studio 솔루션(`CardDefense.slnx`)과 소스 |
| `CardDefense/CardDefense/main.cpp` | 윈도우 생성, 입력 콜백, 메인 루프 |
| `CardDefense/CardDefense/Game.h/.cpp` | 게임 상태(`struct Game`)와 로직. 렌더링 코드 없음 |
| `CardDefense/CardDefense/Renderer.h/.cpp` | `Game`을 읽어 렌더링. 셰이더·메시·카메라 보유. 게임 로직 변경 없음 |
| `CardDefense/CardDefense/Common.h/.cpp` | `Mesh`·`Camera`, 셰이더 로드, 메시 생성(축·큐브), uniform 설정. 정점 = 위치·법선·UV |
| `CardDefense/CardDefense/shader.vert/.frag` | GLSL 셰이더 (`#version 330 core`) |
| `CardDefense/CardDefense/Util/` | 필요할 때 가져다 쓸 유틸: OBJ·텍스처 읽기(stb_image), CSV 읽기, 이징, 마우스 피킹, BMP 스크린샷 |
| `CardDefense/CardDefense/Data/` | 유닛·몹·라운드 등 수치 CSV |
| `assets/` | 게임 리소스(카드·모델·텍스처·UI)와 디자인 시안. 설명은 [`assets/assets.md`](assets/assets.md) |
| `tools/` | 에셋 변환·시안 생성용 Python 스크립트. `compose_cards.py`: 카드 틀 + 그림 + 이름 → `assets/cards/card_*.png` |
| `docs/` | 기획 문서 |
| `external/` | GLEW·GLFW·GLM 헤더(`include/gl/`), lib, DLL(`bin/`, 빌드할 때 exe 폴더로 복사됨) |

## 제출 묶음 만들기

1. Visual Studio에서 x64 / Release로 빌드한다.
2. `python tools/make_submission.py` → `제출/CardDefense-제출/`에 `실행파일/`(exe, 셰이더, Data, assets, GLEW·GLFW DLL, VC++ 런타임 DLL)과 `프로젝트/`(솔루션·소스, 게임 assets, external)가 생긴다.
3. 보고서·발표 자료를 넣고 압축해 e-class에 낸다.

## 검증 도구

- `tools/bot.h`: 아래 두 도구가 같이 쓰는 자동 플레이 봇
- 빌드: x64 Native Tools 명령 프롬프트에서 프로젝트 폴더로 이동 후
  - `cl /EHsc /O2 /utf-8 /ICardDefense\CardDefense /Itools /Iexternal\include tools\balance_sim.cpp CardDefense\CardDefense\Game.cpp CardDefense\CardDefense\Util\Csv.cpp`
  - `cl /EHsc /O2 /MD /utf-8 /ICardDefense\CardDefense /Itools /Iexternal\include tools\capture_playthrough.cpp CardDefense\CardDefense\Game.cpp CardDefense\CardDefense\Renderer.cpp CardDefense\CardDefense\Common.cpp CardDefense\CardDefense\Util\*.cpp /link /LIBPATH:external\lib user32.lib gdi32.lib shell32.lib`
- `tools/balance_sim.cpp`: 실제 `Game.cpp` 로직으로 자동 플레이 봇이 여러 판을 돌려 승률과 패배 라운드를 센다 (렌더링 없음). 수치를 바꾼 뒤 승률 확인용
- `tools/fuzz_test.cpp`: 무작위 조작 퍼즈 테스트. Debug 런타임으로 빌드: `cl /EHsc /Od /MDd /D_DEBUG /DGLFW_DLL /utf-8 /ICardDefense\CardDefense /Iexternal\include tools\fuzz_test.cpp CardDefense\CardDefense\Game.cpp CardDefense\CardDefense\Renderer.cpp CardDefense\CardDefense\Common.cpp CardDefense\CardDefense\Util\*.cpp /link /LIBPATH:external\lib opengl32.lib glew32.lib glfw3dll.lib`
- `tools/capture_playthrough.cpp`: 숨긴 창에서 봇이 한 판을 하며 보고서용 화면을 저장한다. 결과는 `docs/screenshots/`

## 빌드 환경

- Visual Studio 2026 (플랫폼 도구 집합 v145), x64
- C++ 언어 표준은 Visual Studio 기본값 (C++14)
- GLEW, GLFW 3.5, GLM은 `external/`에 들어 있다 (x64 헤더·lib·DLL). 따로 설치할 필요 없다.
- 링크: `opengl32.lib`, `glew32.lib`, `glfw3.lib`

## 실행

1. `CardDefense/CardDefense.slnx`를 Visual Studio로 연다.
2. x64 / Debug 또는 Release로 빌드하고 실행한다.
3. 빌드하면 셰이더, `Data/`, `assets/obj`·`assets/textures`, `glew32.dll`·`glfw3.dll`이 exe 폴더(`x64/Debug` 또는 `x64/Release`)로 복사되고, VS 실행 위치도 exe 폴더다. 코드의 경로는 exe 폴더 기준이다 (`shader.vert`, `Data/...`, `assets/obj/...`).
4. Debug는 GLFW DLL(`glfw3dll.lib` + `glfw3.dll`), Release는 정적 GLFW(`glfw3.lib`)로 링크한다.

## 저장소에 없는 것

- 빌드 결과물(`x64/`, `.pch` 등)
- `assets/design_preview/*.svg`: PNG를 내장한 디자인 시안이라 크다(합계 약 200MB). 같은 시안의 PNG·JPG 미리보기는 포함되어 있다.
