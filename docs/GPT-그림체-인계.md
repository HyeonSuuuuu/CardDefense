# GPT 인계: 카드 디펜스 그림체 통일

아래 "GPT에게 붙여넣을 글"을 그대로 복사하고, "같이 올릴 그림"을 첨부함. 결과 파일을 받아 오면 맨 아래 "받은 뒤 할 일"로 게임에 넣음.

## 같이 올릴 그림

- `assets/paint/board_art.png`: 그림체 기준 (네가 이미 그린 판 원화)
- `assets/concept/screen_current_prepare.png`, `assets/concept/screen_current_battle.png`: 지금 게임 화면 (판만 그림체가 다르고 카드·UI가 따로 놂)
- 다시 그릴 파일 원본 (아래 표의 파일, 한 번에 많이 올리기 어려우면 묶음별로)

## GPT에게 붙여넣을 글

```
너는 OpenGL로 만든 3D 카드 디펜스 게임의 2D 아트를 맡는다. 이전에 네가 첫 번째 첨부 그림(board_art.png, 판 원화)을 그렸다.
지금 게임 화면(screen_current_*.png)을 보면 판만 그 그림체이고, 카드·카드팩·버튼·아이콘은 납작한 벡터풍이라 따로 논다.
목표: 아래 파일들을 board_art.png와 같은 그림체로 다시 그려서 화면 전체를 하나의 그림체로 맞춘다.

그림체 기준 (board_art.png에서 가져온다)
- 손으로 칠한 판타지 보드게임 일러스트, 따뜻한 나무·양피지·금속 재질감
- 빛은 오른쪽 위에서 오는 따뜻한 빛, 부드러운 그림자
- 색: 진한 오크 나무 갈색, 이끼 낀 돌 베이지, 풀 초록, 포인트는 금색과 붉은 칩 색
- 너무 사실적이지 않게, 게임 화면에서 작게 봐도 형태가 바로 읽히도록 외곽을 분명하게

모든 파일 공통 규칙 (꼭 지킨다)
- 파일 이름과 픽셀 크기를 표 그대로. PNG, 투명 배경(표에 '불투명'이라고 한 것만 예외)
- 그림 안에 글자·숫자를 넣지 않는다. 이름·숫자는 게임이 따로 쓴다 (예외는 표에 적음)
- 원본의 형태와 구성(무엇이 어디에 있는지)은 유지하고 그림체만 바꾼다. 원본을 같이 첨부한다
- 한 번에 한 묶음씩 만들고, 묶음마다 결과를 보여 준 뒤 다음으로 넘어간다

묶음 1: 카드 (가장 눈에 띔)
| 파일 | 크기 | 내용 |
|---|---|---|
| frame_common.png | 512x768 | 일반 등급 카드 틀. 나무+양피지. 그림 칸(x40~472, y50~485)과 이름 칸(x40~472, y518~622), 태그 칸(x40~472, y652~710)은 비워 둔다(양피지 단색). 글자 없음 |
| frame_rare.png | 512x768 | 희귀 등급 틀. 위와 같은 칸 위치, 은색 장식 |
| frame_hero.png | 512x768 | 영웅 등급 틀. 같은 칸 위치, 금색 장식과 보석 |
| frame_system.png | 512x768 | 화면 전환 카드 틀(시작·종료 등). 같은 칸 위치, 짙은 남색+금색 |
| card_back.png | 512x768 | 카드 뒷면. 불투명. 가운데 방패 문장 |
| art_mage_1/2/3.png, art_archer_1/2/3.png, art_warrior_1/2/3.png | 512x512 | 유닛 카드 그림. 원본과 같은 캐릭터·자세·색, 그림체만 board_art에 맞춤. 투명 배경, 캐릭터만 |
| art_spell_meteor.png, art_spell_drum.png, art_spell_chips.png, art_promote.png | 512x512 | 주문 카드 그림: 불타는 운석 주사위, 전투 북, 칩 주머니, 승급(위로 향하는 금빛 화살과 별) |
| art_sys_start.png, art_sys_exit.png, art_sys_retry.png, art_sys_title.png | 512x512 | 화면 전환 카드 그림: 시작, 나가기, 다시 하기, 타이틀로. 글자 없이 상징으로 |

묶음 2: 카드팩·버튼
| 파일 | 크기 | 내용 |
|---|---|---|
| pack_booster.png | 384x512 | 파란 카드팩(일반 부스터). 은박 포장, 방패 문장 |
| pack_gold.png | 384x512 | 금색 카드팩(골드 부스터). 같은 모양에 금박 |
| button_end_turn.png | 320x112 | '턴 종료' 버튼 바탕. 나무 판에 금속 테두리. 글자 '턴 종료'를 가운데에 넣는다(예외) |
| toggle_bet_off.png / toggle_bet_on.png | 256x96 | '최대 베팅' 스위치. 왼쪽 반에 글자 '최대 베팅'(예외), 오른쪽에 둥근 스위치(off: 회색 왼쪽, on: 금색 오른쪽) |

묶음 3: 아이콘·유물 (각 128x128, 게임에서 약 40px로 보이므로 단순하고 굵게)
| 파일 | 내용 |
|---|---|
| icon_chip.png | 붉은 카지노 칩 (화폐) |
| relic_crest_mage.png / relic_crest_archer.png / relic_crest_warrior.png | 마법사·궁수·전사 문장 방패 (파랑·초록·빨강) |
| relic_hourglass.png | 모래시계 |
| relic_lucky_dice.png | 행운의 주사위 |
| relic_thick_pack.png | 두툼한 카드 묶음 |

묶음 4: 큰 글자 그림 (글자 그림이 본체라 예외)
| 파일 | 크기 | 내용 |
|---|---|---|
| logo.png | 1024x384 | 게임 로고 '카드 디펜스' |
| text_victory.png / text_defeat.png / text_pause.png | 1024x256 | '승리' / '패배' / '일시 정지' 글자 장식 |
```

## 다시 그리지 않는 것

- `card_*.png`(완성 카드): 틀 + 그림 + 이름 글자를 `tools/compose_cards.py`가 합성함. 묶음 1을 받으면 다시 돌림
- `tip_*.png`(유물 설명 리본), `digits.png`(숫자): 글꼴로 만드는 파일. 리본 색만 바꿀 거면 `tools/make_relic_tips.py`의 색 상수를 고침
- `assets/textures/<유닛>.png`: 3D 모델에 감싸는 펼친 텍스처라 AI가 다시 그리면 모델에 안 맞음. 말 그림체까지 바꾸려면 `front/<ID>.png`(정면 그림)를 새 그림체로 뽑고 Tripo로 모델을 다시 만드는 원래 과정을 밟아야 함 (시간이 많이 듦)

## 받은 뒤 할 일 (Claude에게 맡기면 됨)

1. 받은 파일을 같은 이름으로 `assets/cards/`, `assets/ui/`에 덮어씀 (기존 파일은 git에 남아 있어 되돌릴 수 있음)
2. 크기·투명 배경 검사, 틀의 빈 칸 위치가 `compose_cards.py`의 `ART_BOX`·`NAME_BOX`·`TAG_BOX`와 맞는지 확인
3. `python -I tools/compose_cards.py`로 완성 카드 다시 합성
4. 빌드 후 게임 화면으로 확인, 글자 색(`TEXT_COLOR`)이 새 틀에서 읽히는지 확인
