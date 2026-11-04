# 에셋 목록

파일과 코드는 ID로 부른다. 한글 이름은 이 표에서만 바꾼다. 경로에는 영어 소문자·숫자·`_`만 쓴다(MSVC에서 한글 경로를 파일 함수로 열 때 생기는 문제 방지).

## 폴더 (종류별, 파일 이름 = ID)

| 폴더 | 파일 이름 | 내용 |
|---|---|---|
| `front/` | `<ID>.png` | GPT 정면 이미지 (흰 배경, Tripo 입력) |
| `glb/` | `<ID>.glb` | Tripo 원본 (손대지 않음) |
| `obj/` | `<ID>.obj`, `<ID>.mtl` | 게임이 읽는 모델. `tools/glb2obj.py`로 GLB에서 변환: +Y 위, 정면 +Z, 원점 = 받침 바닥 중앙, 높이 1, 삼각형, `v/vt/vn` 인덱스 같음, vt는 OpenGL 기준(아래가 0) |
| `textures/` | `<ID>.png` | `obj/<ID>.obj`의 텍스처(1024², 위쪽 행부터 저장 → `stbi_set_flip_vertically_on_load(true)`로 읽음). 판 텍스처(`grass.png` 등)도 여기 |
| `cards/` | 아래 2D 표 | 카드 틀·뒷면·카드 그림·완성 카드 |
| `ui/` | 아래 2D 표 | 팩·아이콘·유물·로고 |
| `concept/` | | GPT 콘셉트 그림(시트, 화면 목업). 게임이 읽지 않음 |
| `concept/old/` | | 바뀌기 전 그림. 지우지 않고 보관 |

- Tripo 여러 시점 입력이 필요하면 `front/<ID>_side.png`, `front/<ID>_back.png`.

## 목록

진행: 이미지 → 3D 생성 → OBJ 변환 → 게임 적용

- Tripo 설정: 스마트 메시, 조명 제거, 생성 1개. 결과가 모두 삼각형 5,200~5,600·높이 1·원점 맞음이라 Blender 정리는 생략.
- 모델을 다시 뽑으면 `python -I ../tools/glb2obj.py . <ID>`(assets 폴더에서) 후 텍스처 JPG를 PNG로 바꾼다.

| ID | 이름 | 클래스·등급 | 진행 | 메모 |
|---|---|---|---|---|
| `mage_1` | 꼬마 마법사 | 마법사 일반 | OBJ 변환 | 삼각형 5,226. 낮은 종형 받침, 동색 띠는 바닥 1줄. 팔·지팡이 정상 |
| `mage_2` | 번개 마녀 | 마법사 희귀 | OBJ 변환 | 불꽃 마녀에서 변경. 삼각형 5,532. 번개 지팡이·지그재그 모자 띠 정상 |
| `mage_3` | 대마법사 | 마법사 영웅 | OBJ 변환 | 삼각형 5,409. 수염·머리 뒤쪽 자연스러움|
| `archer_1` | 꼬마 궁수 | 궁수 일반 | OBJ 변환 | Tripo 스마트 메시: 삼각형 5,604, 텍스처 1024 JPG, 높이 1·원점 받침 바닥 중앙. 활시위 얇은 면이라 양면 그리기 필요. 미리보기 `concept/preview_archer_1.png` |
| `archer_2` | 석궁수 | 궁수 희귀 | OBJ 변환 | 삼각형 5,428. 석궁·시위 정상|
| `archer_3` | 숲의 엘프 | 궁수 영웅 | OBJ 변환 | 서리 엘프에서 변경. 삼각형 5,639. 활시위 살아 있음(양면 그리기) |
| `warrior_1` | 꼬마 창병 | 전사 일반 | OBJ 변환, 카드 그림 | 삼각형 5,382. 투구 아래 뒤쪽 머리카락이 양옆으로 갈래처럼 튀어나옴(뒤에서만 보임)|
| `warrior_2` | 기사 | 전사 희귀 | OBJ 변환 | Tripo 스마트 메시 + 조명 제거: 삼각형 5,403, 금속 반사 없음. 벌린 입이 입체 없이 색으로만 붙음. 게임 크기(약 90px)에선 안 보여 그대로 씀. 투구 뒤쪽은 AI 추측 |
| `warrior_3` | 용기사 | 전사 영웅 | OBJ 변환 | 삼각형 5,250. 왼팔이 어깨 갑옷 덩어리로 뭉침(옆에서 보임)|
| `enemy_skeleton` | 해골 병사 | 적 (기본) | OBJ 변환 | 삼각형 5,266. 갈비뼈·손 형태 유지. 받침은 짙은 회갈색(완전 검정 아님) |
| `enemy_goblin` | 고블린 | 적 (빠름) | OBJ 변환 | 삼각형 5,481. 귀·모자 정상 |
| `enemy_orc` | 오크 | 적 (튼튼, 칩 3) | OBJ 변환 | 삼각형 5,288. 팔짱·어깨 가시 정상 |
| `boss_king` | 그림자 왕 | 보스 | OBJ 변환 | 삼각형 5,354. 얼굴 어둡지만 노란 눈·부리 형태 보임. 금띠 3줄 정상 |

## 2D 그림

카드·아이콘·팩 등 화면 위 UI는 평면 외곽선 그림체(`concept/card_warrior_1.png` 기준), 판 위 말은 3D 장난감 그림체.

| 폴더 | 파일 | 내용 | 진행 |
|---|---|---|---|
| `cards/` | `frame_common.png`, `frame_rare.png`, `frame_hero.png` | 평면 카드 틀(동·은·금). 그림 칸·이름 칸·위아래 태그는 비움 | 완료 |
| `cards/` | `card_back.png` | 카드 뒷면 | 완료 |
| `cards/` | `art_<ID>.png` | 유닛 카드 그림 9장(평면 외곽선, 투명 배경) | 완료 |
| `cards/` | `art_spell_meteor.png`, `art_spell_drum.png`, `art_spell_chips.png` | 주문 카드 그림: 운석 주사위, 전투의 북, 칩 주머니 | 완료 |
| `cards/` | `art_promote.png` | 승격 카드 그림 | 완료 |
| `cards/` | `art_sys_start.png`, `art_sys_exit.png`, `art_sys_retry.png`, `art_sys_title.png` | 화면 전환 카드 그림: Start, Exit, Retry, Title | 완료 |
| `cards/` | `card_<ID>.png`, `card_spell_*.png`, `card_promote.png`, `card_sys_*.png` | 완성 카드: 틀 + 그림 + 이름·태그 글자 합성(튜터가 스크립트로) | 틀이 오면 |
| `ui/` | `pack_booster.png`, `pack_gold.png` | 팩 2종 | 완료 |
| `ui/` | `icon_chip.png` | 칩 아이콘 | 완료 |
| `ui/` | `relic_lucky_dice.png`, `relic_thick_pack.png`, `relic_hourglass.png`, `relic_crest_mage.png`, `relic_crest_archer.png`, `relic_crest_warrior.png` | 유물 6종 | 완료 |
| `ui/` | `logo.png` | 타이틀 로고 "카드 디펜스" | 완료 |
| `textures/` | `grass.png`, `path_stone.png`, `board_wood.png`, `table_wood.png` | 판 텍스처: 칸 풀밭, 길 돌, 판 테두리 나무, 작업대 | 완료 |
| `cards/` | `frame_system.png` | 화면 전환 카드(Start·Exit·Retry·Title) 틀, 파랑 테두리 | 완료 |
| `ui/` | `digits.png` | 숫자 시트 896×96: 64×96 칸 14개 `0~9 / × + :`, 흰 글자 + 갈색 외곽선 (코드가 색을 곱함) | 완료 |
| `ui/` | `button_end_turn.png` | "턴 종료" 버튼 | 완료 |
| `ui/` | `text_victory.png`, `text_defeat.png` | 결과 글자 "승리!", "패배" | 완료 |
| `ui/` | `toggle_bet_off.png`, `toggle_bet_on.png`, `text_pause.png` | 베팅 스위치, 일시정지 글자 (권장) | 완료 |

- 크기·우선순위·세부 지시: `../docs/카드디펜스-리소스목록.md` (Codex 작업 지시용)

- 옛 `concept/sheet_relics.png`는 예전 이름(광역·중간·단일 문장)과 클래스 색이 안 맞아 새로 뽑는다.

## 콘셉트 그림

| 파일 | 내용 | 원래 이름 |
|---|---|---|
| `concept/sheet_units.png` | 말 9종 시트 (현재 기준) | 캐릭터시트.png |
| `concept/sheet_enemies.png` | 옛 적 4종(주사위·칩, 폐기된 디자인) | 적 피규어 시트.png |
| `concept/sheet_relics.png` | 유물 아이콘 (옛 이름) | 유물 아이콘 시트.png |
| `concept/screen_title.png` | 시작 화면 목업 | 카드 디펜스 시작 화면-2.png |
| `concept/screen_battle.png` | 전투 화면 목업 (제안서에 사용) | 카드 디펜스 4×4 배치 전장-6.png |
| `concept/card_warrior_1.png` | 꼬마 창병 카드 (옛 디자인, 카드 그림체 기준) | 꼬마 창병 유닛 카드-6.png |
| `concept/card_warrior_1_archer_1.png` | 꼬마 창병·꼬마 궁수 카드 | 꼬마 창병과 꼬마 궁수 카드-5.png |
| `concept/card_spell_meteor.png` | 운석 주사위 카드 | 불꽃 운석 주사위 카드-4.png |
| `concept/old/card_archer_3_frost.png` | 서리 엘프 카드 (이름 바뀜) | 서리 엘프 보상 카드와 꼬마 부대-4.png |
| `concept/old/sheet_units_chess_v1.png` | 첫 체스말 시트 (받침 색 통일 전) | 체스클래스도감 |
| `concept/old/frame_common_metal.png`, `_raw.png` | 금속 질감 동색 틀 (평면 틀로 바꿔서 보관) | 청동 프레임의 빈 카드 UI 템플릿.png |
| `concept/preview_<ID>.png` | 3D 모델 4방향 미리보기 (튜터 렌더) | |
| `concept/old/card_warrior_1_3d_test.png` | 3D 이미지로 합성한 시험 카드 | (튜터 합성) |

## AI 사용 기록

제안서 4장·보고서용. 작업할 때마다 한 줄.

| 날짜 | ID | 도구 | 한 일 | 직접 수정한 것 |
|---|---|---|---|---|
| 2026-10-23 | `archer_1` | Tripo (유료, 스마트 메시) | 정면 이미지 1장 → GLB | |
| 2026-10-23 | `warrior_2` | Tripo (유료, 스마트 메시, 조명 제거) | 정면 이미지 1장 → GLB | |
| 2026-10-23 | `mage_1`, `mage_3`, `archer_2`, `archer_3`, `warrior_1`, `warrior_3` | Tripo (유료, 스마트 메시, 조명 제거) | 정면 이미지 1장 → GLB | |
| 2026-10-23 | `mage_2`, `enemy_skeleton`, `enemy_goblin`, `enemy_orc`, `boss_king` | Tripo (유료, 스마트 메시) | 정면 이미지 1장 → GLB | |
| 2026-11-05 | 신규 2D 리소스 40종 | Codex 내장 ImageGen, Python/Pillow·SVG, Blender, Figma | 캐릭터 그림 6종·주문/승격 4종·시작 그림 1종·반복 텍스처 4종 생성, UI 25종 제작, 기존 OBJ로 화면 3종 합성 및 Figma 배치 | 규격·RGBA·숫자 14칸 검증, 텍스처 2×2 반복 검토, Figma 한글 글꼴 보정 |
