"""완성 카드 합성: 카드 틀 + 카드 그림 + 이름·태그 글자 → assets/cards/card_<ID>.png

사용: python -I tools/compose_cards.py   (프로젝트 폴더에서)
- 유닛 9장: 등급별 틀(frame_common/rare/hero), 이름은 units.csv의 name
- 화면 전환 4장: frame_system, Start/Exit/Retry/Title
- 주문 3장: 등급별 틀, 이름은 spells.csv의 name
- 승격 1장: 영웅 틀 (골드 부스터에서만 나옴)
글꼴은 Windows의 맑은 고딕 굵게(malgunbd.ttf)를 쓴다.
"""
import csv
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CARDS = os.path.join(ROOT, 'assets', 'cards')
UNITS_CSV = os.path.join(ROOT, 'CardDefense', 'CardDefense', 'Data', 'units.csv')
SPELLS_CSV = os.path.join(ROOT, 'CardDefense', 'CardDefense', 'Data', 'spells.csv')
SPELL_ART = {'meteor_dice': 'art_spell_meteor.png', 'battle_drum': 'art_spell_drum.png', 'chip_pouch': 'art_spell_chips.png'}
FONT = r'C:\Windows\Fonts\malgunbd.ttf'

# 틀(512×768) 안의 칸 위치: 그림 칸, 이름 칸, 아래 두 줄 사이 태그 칸
ART_BOX = (40, 50, 472, 485)
NAME_BOX = (40, 518, 472, 622)
TAG_BOX = (40, 652, 472, 710)
TEXT_COLOR = (42, 42, 48, 255)

CLASS_NAMES = {'mage': '마법사', 'archer': '궁수', 'warrior': '전사'}
TIER_NAMES = ['일반', '희귀', '영웅']
TIER_FRAMES = ['frame_common.png', 'frame_rare.png', 'frame_hero.png']

SYSTEM_CARDS = [
    ('sys_start', 'Start', '시작'),
    ('sys_exit', 'Exit', '종료'),
    ('sys_retry', 'Retry', '다시 하기'),
    ('sys_title', 'Title', '타이틀로'),
]


def fit_font(draw, text, box, start_size):
    """칸 너비에 들어갈 때까지 글자 크기를 줄인다."""
    size = start_size
    while size > 10:
        font = ImageFont.truetype(FONT, size)
        left, top, right, bottom = draw.textbbox((0, 0), text, font=font)
        if right - left <= (box[2] - box[0]) - 24:
            return font
        size -= 2
    return ImageFont.truetype(FONT, size)


def draw_centered(draw, text, box, start_size):
    font = fit_font(draw, text, box, start_size)
    cx = (box[0] + box[2]) / 2
    cy = (box[1] + box[3]) / 2
    draw.text((cx, cy), text, font=font, fill=TEXT_COLOR, anchor='mm')


def compose(frame_name, art_name, name, tag, out_name):
    card = Image.open(os.path.join(CARDS, frame_name)).convert('RGBA')
    art = Image.open(os.path.join(CARDS, art_name)).convert('RGBA')

    # 그림은 정사각형이라 그림 칸의 짧은 변에 맞추고 가운데에 놓는다.
    w = ART_BOX[2] - ART_BOX[0]
    h = ART_BOX[3] - ART_BOX[1]
    size = min(w, h) - 16
    art = art.resize((size, size), Image.LANCZOS)
    x = ART_BOX[0] + (w - size) // 2
    y = ART_BOX[1] + (h - size) // 2
    card.alpha_composite(art, (x, y))

    draw = ImageDraw.Draw(card)
    draw_centered(draw, name, NAME_BOX, 60)
    draw_centered(draw, tag, TAG_BOX, 34)
    card.save(os.path.join(CARDS, out_name))
    print('saved', out_name)


def main():
    if not os.path.exists(FONT):
        sys.exit('글꼴이 없음: ' + FONT)

    with open(UNITS_CSV, encoding='utf-8') as f:
        lines = [line for line in f if line.strip() and not line.startswith('#')]
    for row in csv.DictReader(lines):
        tier = int(row['tier'])
        tag = CLASS_NAMES[row['class']] + ' · ' + TIER_NAMES[tier]
        compose(TIER_FRAMES[tier], 'art_' + row['id'] + '.png', row['name'], tag, 'card_' + row['id'] + '.png')

    with open(SPELLS_CSV, encoding='utf-8') as f:
        lines = [line for line in f if line.strip() and not line.startswith('#')]
    for row in csv.DictReader(lines):
        tier = int(row['rarity'])
        compose(TIER_FRAMES[tier], SPELL_ART[row['id']], row['name'], '주문 · ' + TIER_NAMES[tier], 'card_' + row['id'] + '.png')

    compose('frame_hero.png', 'art_promote.png', '승격', '말 등급 +1', 'card_promote.png')

    for card_id, name, tag in SYSTEM_CARDS:
        compose('frame_system.png', 'art_' + card_id + '.png', name, tag, 'card_' + card_id + '.png')


if __name__ == '__main__':
    main()
