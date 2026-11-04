from build_ui_assets import Art,OUT,SRC,S,INK,BLUE,GOLD
from PIL import Image,ImageDraw,ImageFont
import base64
# Native generated sprite: exact visible glyph height and fixed 64x96 cells.
sheet=Image.new('RGBA',(896,96));font=ImageFont.truetype('C:/Windows/Fonts/malgunbd.ttf',288)
for i,ch in enumerate('0123456789/×+:'):
 box=font.getbbox(ch,stroke_width=16);g=Image.new('RGBA',(box[2]-box[0]+32,box[3]-box[1]+32));d=ImageDraw.Draw(g)
 d.text((16-box[0],16-box[1]),ch,font=font,fill='white',stroke_width=16,stroke_fill=INK);g=g.crop(g.getbbox())
 w=min(54,round(g.width/g.height*76));g=g.resize((w,76),Image.Resampling.LANCZOS)
 sheet.alpha_composite(g,(i*64+(64-w)//2,10))
sheet.save(OUT/'ui/digits.png')
(SRC/'digits.svg').write_text('<svg xmlns="http://www.w3.org/2000/svg" width="896" height="96"><image width="896" height="96" href="data:image/png;base64,'+base64.b64encode((OUT/'ui/digits.png').read_bytes()).decode()+'"/></svg>',encoding='utf-8')
a=Art(1024,384)
a.text((512,190),'카드 디펜스',142,GOLD,INK,10)
a.text((512,185),'카드 디펜스',142,BLUE)
for x,flip in [(62,1),(962,-1)]:a.poly([(x,230),(x-30*flip,170),(x+18*flip,183),(x+42*flip,230)],'#91B86D',sw=7)
a.im.resize((1024,384),Image.Resampling.LANCZOS).save(OUT/'ui/logo.png')
(SRC/'logo.svg').write_text('<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="384" viewBox="0 0 1024 384">'+''.join(a.svg)+'</svg>',encoding='utf-8')
