"""유물 설명 리본: Data/relics.csv의 이름과 효과로 assets/ui/tip_<id>.png를 만든다 (아이콘에 마우스를 올리면 뜸).

사용: python -I tools/make_relic_tips.py   (프로젝트 폴더에서)
글꼴은 Windows의 맑은 고딕 굵게(malgunbd.ttf).
"""
import csv
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV = os.path.join(ROOT, 'CardDefense', 'CardDefense', 'Data', 'relics.csv')
OUT = os.path.join(ROOT, 'assets', 'ui')
FONT = r'C:\Windows\Fonts\malgunbd.ttf'

SIZE = (560, 130)
NOTCH = 34          # 양 끝 V자 홈 깊이
RIBBON = (178, 52, 48, 255)
SHADE = (122, 30, 30, 255)
EDGE = (250, 214, 130, 255)
TITLE = (255, 236, 190, 255)
DESC = (255, 250, 240, 255)
CLASS_NAMES = {'mage': '마법사', 'archer': '궁수', 'warrior': '전사'}


def effect_text(row):
    amount = row['amount']
    effect = row['effect']
    if effect == 'rare_chance':
        return f'팩에서 희귀 이상 확률 +{amount}%p'
    if effect == 'pack_extra_card':
        return f'카드팩에서 카드 +{amount}장'
    if effect == 'round_time':
        return f'라운드 제한 시간 +{amount}초'
    if effect == 'class_damage':
        return f'{CLASS_NAMES[row["target_class"]]} 말 피해 +{amount}%'
    return effect


def fit(draw, text, size):
    """양 끝 홈 안쪽 폭에 들어갈 때까지 글자 크기를 줄인다."""
    room = SIZE[0] - 2 * NOTCH - 40
    while size > 14:
        font = ImageFont.truetype(FONT, size)
        left, _, right, _ = draw.textbbox((0, 0), text, font=font)
        if right - left <= room:
            return font
        size -= 1
    return ImageFont.truetype(FONT, size)


def ribbon(draw, box, fill):
    x0, y0, x1, y1 = box
    mid = (y0 + y1) / 2
    draw.polygon([(x0, y0), (x1, y0), (x1 - NOTCH, mid), (x1, y1), (x0, y1), (x0 + NOTCH, mid)], fill=fill)


def main():
    if not os.path.exists(FONT):
        sys.exit('글꼴이 없음: ' + FONT)
    with open(CSV, encoding='utf-8') as f:
        lines = [line for line in f if line.strip() and not line.startswith('#')]
    title_font = ImageFont.truetype(FONT, 38)
    w, h = SIZE
    for row in csv.DictReader(lines):
        image = Image.new('RGBA', SIZE, (0, 0, 0, 0))
        draw = ImageDraw.Draw(image)
        # 뒤에 접힌 그림자 띠, 앞에 리본, 위아래 금색 선
        ribbon(draw, (4, 14, w - 4, h - 4), SHADE)
        ribbon(draw, (0, 6, w, h - 12), RIBBON)
        draw.line((NOTCH + 8, 14, w - NOTCH - 8, 14), fill=EDGE, width=3)
        draw.line((NOTCH + 8, h - 20, w - NOTCH - 8, h - 20), fill=EDGE, width=3)
        draw.text((w / 2, 46), row['name'], font=title_font, fill=TITLE, anchor='mm')
        draw.text((w / 2, 86), effect_text(row) + ' (겹치면 더함)', font=fit(draw, effect_text(row) + ' (겹치면 더함)', 26), fill=DESC, anchor='mm')
        path = os.path.join(OUT, 'tip_' + row['id'] + '.png')
        image.save(path)
        print('saved', os.path.basename(path))


if __name__ == '__main__':
    main()
