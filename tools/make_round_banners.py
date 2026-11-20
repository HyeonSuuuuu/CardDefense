"""라운드 특성 배너: Data/modifiers.csv의 name·desc로 assets/ui/round_<id>.png를 만든다.

사용: python -I tools/make_round_banners.py   (프로젝트 폴더에서)
글꼴은 Windows의 맑은 고딕 굵게(malgunbd.ttf).
"""
import csv
import os
import sys

from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV = os.path.join(ROOT, 'CardDefense', 'CardDefense', 'Data', 'modifiers.csv')
OUT = os.path.join(ROOT, 'assets', 'ui')
FONT = r'C:\Windows\Fonts\malgunbd.ttf'

SIZE = (640, 120)
BACK = (36, 30, 38, 225)
EDGE = (232, 163, 61, 255)
TITLE = (255, 214, 120, 255)
DESC = (240, 230, 214, 255)


def main():
    if not os.path.exists(FONT):
        sys.exit('글꼴이 없음: ' + FONT)
    with open(CSV, encoding='utf-8') as f:
        lines = [line for line in f if line.strip() and not line.startswith('#')]
    title_font = ImageFont.truetype(FONT, 44)
    desc_font = ImageFont.truetype(FONT, 28)
    for row in csv.DictReader(lines):
        image = Image.new('RGBA', SIZE, (0, 0, 0, 0))
        draw = ImageDraw.Draw(image)
        draw.rounded_rectangle((2, 2, SIZE[0] - 3, SIZE[1] - 3), radius=26, fill=BACK, outline=EDGE, width=4)
        draw.text((SIZE[0] / 2, 42), row['name'], font=title_font, fill=TITLE, anchor='mm')
        draw.text((SIZE[0] / 2, 90), row['desc'], font=desc_font, fill=DESC, anchor='mm')
        path = os.path.join(OUT, 'round_' + row['id'] + '.png')
        image.save(path)
        print('saved', os.path.basename(path))


if __name__ == '__main__':
    main()
