"""Deterministic UI assets: exact RGBA dimensions plus editable SVG sources."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import math, html, json

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'assets'
SRC = OUT / 'design_sources'
SRC.mkdir(exist_ok=True)
INK='#3B2A20'; CREAM='#FFF0CF'; BLUE='#70AADC'; GOLD='#E8B931'; NAVY='#263E55'
S=3
class Art:
    def __init__(self,w,h):
        self.w,self.h=w,h; self.im=Image.new('RGBA',(w*S,h*S)); self.d=ImageDraw.Draw(self.im); self.svg=[]
    def rect(self,box,fill,stroke=INK,sw=4,r=0):
        x,y,x2,y2=box; self.d.rounded_rectangle(tuple(int(v*S) for v in box),radius=r*S,fill=fill,outline=stroke,width=sw*S)
        self.svg.append(f'<rect x="{x}" y="{y}" width="{x2-x}" height="{y2-y}" rx="{r}" fill="{fill or "none"}" stroke="{stroke or "none"}" stroke-width="{sw}"/>')
    def ellipse(self,box,fill,stroke=INK,sw=4):
        x,y,x2,y2=box; self.d.ellipse(tuple(int(v*S) for v in box),fill=fill,outline=stroke,width=sw*S)
        self.svg.append(f'<ellipse cx="{(x+x2)/2}" cy="{(y+y2)/2}" rx="{(x2-x)/2}" ry="{(y2-y)/2}" fill="{fill or "none"}" stroke="{stroke or "none"}" stroke-width="{sw}"/>')
    def poly(self,p,fill,stroke=INK,sw=4):
        pts=[(int(x*S),int(y*S)) for x,y in p]; self.d.polygon(pts,fill=fill)
        if stroke:self.d.line(pts+[pts[0]],fill=stroke,width=sw*S,joint='curve')
        self.svg.append(f'<polygon points="{" ".join(f"{x},{y}" for x,y in p)}" fill="{fill or "none"}" stroke="{stroke or "none"}" stroke-width="{sw}" stroke-linejoin="round"/>')
    def line(self,p,fill=INK,sw=4):
        self.d.line([(int(x*S),int(y*S)) for x,y in p],fill=fill,width=sw*S,joint='curve')
        self.svg.append(f'<polyline points="{" ".join(f"{x},{y}" for x,y in p)}" fill="none" stroke="{fill}" stroke-width="{sw}" stroke-linejoin="round" stroke-linecap="round"/>')
    def text(self,xy,t,size,fill=CREAM,stroke=None,sw=0):
        f=ImageFont.truetype('C:/Windows/Fonts/malgunbd.ttf',size*S)
        self.d.text((int(xy[0]*S),int(xy[1]*S)),t,font=f,anchor='mm',fill=fill,stroke_width=sw*S,stroke_fill=stroke)
        self.svg.append(f'<text x="{xy[0]}" y="{xy[1]}" text-anchor="middle" dominant-baseline="central" font-family="Malgun Gothic" font-weight="700" font-size="{size}" fill="{fill}" stroke="{stroke or "none"}" stroke-width="{sw*2}" paint-order="stroke">{html.escape(t)}</text>')
    def star(self,x,y,r,fill=GOLD):
        self.poly([(x+math.sin(i*math.pi/5)*r*(1 if i%2==0 else .45),y-math.cos(i*math.pi/5)*r*(1 if i%2==0 else .45)) for i in range(10)],fill)
    def save(self,path):
        dest=OUT/path; dest.parent.mkdir(exist_ok=True)
        if dest.exists(): raise FileExistsError(dest)
        self.im.resize((self.w,self.h),Image.Resampling.LANCZOS).save(dest)
        (SRC/(dest.stem+'.svg')).write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="{self.w}" height="{self.h}" viewBox="0 0 {self.w} {self.h}">'+''.join(self.svg)+'</svg>',encoding='utf-8')

def frame(name,color):
    a=Art(512,768); a.rect((10,10,502,758),color,sw=9,r=32)
    a.rect((25,25,487,743),CREAM,sw=4,r=23)
    a.rect((36,48,476,488),'#F6E4BE',sw=5,r=15)
    a.rect((36,516,476,625),CREAM,sw=5,r=14)
    a.line([(55,650),(457,650)],color,4)
    a.line([(55,712),(457,712)],color,4)
    for x in [27,485]:
        for y in [28,740]:a.ellipse((x-8,y-8,x+8,y+8),color,sw=3)
    if name=='hero':
        for x in [61,451]:a.poly([(x,22),(x+15,40),(x,58),(x-15,40)],GOLD,sw=3)
    a.save('cards/frame_'+name+'.png')

def chip(a,cx,cy,r):
    a.ellipse((cx-r,cy-r,cx+r,cy+r),'#D85046',sw=max(2,int(r*.08)))
    for i in range(8):
        t=i*math.pi/4; p=[]
        for rr,ang in [(r*.94,t-.15),(r*.94,t+.15),(r*.7,t+.15),(r*.7,t-.15)]:p.append((cx+math.cos(ang)*rr,cy+math.sin(ang)*rr))
        a.poly(p,'#FFF8E9',None,0)
    a.ellipse((cx-r*.58,cy-r*.58,cx+r*.58,cy+r*.58),'#D85046','#FFF8E9',max(2,int(r*.035)))

def shield(a,color):
    a.poly([(20,18),(64,8),(108,18),(103,77),(88,99),(64,119),(40,99),(25,77)],color,sw=6)
    a.line([(30,27),(64,19),(98,27)],'#FFF0CF',3)

def make():
    for n,c in [('common','#B87333'),('rare','#C0C7D0'),('hero',GOLD),('system',BLUE)]:frame(n,c)
    a=Art(512,768);a.rect((10,10,502,758),NAVY,sw=9,r=32);a.rect((30,30,482,738),None,GOLD,5,22);a.rect((48,48,464,720),None,'#7487A0',2,14)
    a.poly([(256,135),(421,384),(256,633),(91,384)],'#324D69',GOLD,5)
    a.star(256,384,116);a.ellipse((222,350,290,418),NAVY,GOLD,4)
    for x,y in [(70,72),(442,72),(70,696),(442,696)]:a.star(x,y,17)
    a.save('cards/card_back.png')
    a=Art(128,128);chip(a,64,64,57);a.save('ui/icon_chip.png')
    a=Art(896,96)
    # Each glyph has the same visible height and is centered in its exact 64x96 cell.
    for i,ch in enumerate('0123456789/×+:'):
        f=ImageFont.truetype('C:/Windows/Fonts/malgunbd.ttf',72*S); box=f.getbbox(ch,stroke_width=4*S)
        glyph=Image.new('RGBA',(box[2]-box[0]+8*S,box[3]-box[1]+8*S));gd=ImageDraw.Draw(glyph)
        gd.text((4*S-box[0],4*S-box[1]),ch,font=f,fill='white',stroke_width=4*S,stroke_fill=INK)
        bb=glyph.getbbox();glyph=glyph.crop(bb); ratio=min(54*S/glyph.width,76*S/glyph.height)
        glyph=glyph.resize((round(glyph.width*ratio),round(glyph.height*ratio)),Image.Resampling.LANCZOS)
        a.im.alpha_composite(glyph,(i*64*S+(64*S-glyph.width)//2,(96*S-glyph.height)//2))
    a.save('ui/digits.png')
    a=Art(320,112);a.rect((5,7,315,107),NAVY,sw=6,r=24);a.rect((14,16,306,97),None,'#5E7E98',3,17);a.text((160,55),'턴 종료',42);a.save('ui/button_end_turn.png')
    for name,label,c in [('victory','승리!',GOLD),('defeat','패배','#A39483'),('pause','일시정지',CREAM)]:
        a=Art(1024,256);a.text((512,128),label,170,c,INK,7);a.save('ui/text_'+name+'.png')
    a=Art(1024,384);a.text((512,190),'카드 디펜스',142,GOLD,INK,10);a.text((512,185),'카드 디펜스',142,GOLD)
    for x,flip in [(62,1),(962,-1)]:a.poly([(x,230),(x-30*flip,170),(x+18*flip,183),(x+42*flip,230)],'#91B86D',sw=7)
    a.save('ui/logo.png')
    for on in [False,True]:
        a=Art(256,96);a.rect((4,5,252,91),GOLD if on else NAVY,sw=5,r=39);a.text((92,47),'최대 베팅',25,INK if on else CREAM)
        a.rect((166,25,235,72),'#8A6A2B' if on else '#536375',sw=3,r=23);x=211 if on else 190;a.ellipse((x-18,30,x+18,66),CREAM,sw=3);a.save('ui/toggle_bet_'+('on' if on else 'off')+'.png')
    for gold in [False,True]:
        a=Art(384,512);base=GOLD if gold else BLUE;dark='#B48225' if gold else '#477AAE'
        a.rect((29,24,355,488),base,sw=8,r=15);a.poly([(29,70),(355,70),(355,442),(29,442)],dark,sw=3)
        for y in [38,473]:
            for x in range(40,350,16):a.line([(x,y-8),(x,y+8)],dark,3)
        a.poly([(71,145),(192,112),(313,145),(300,299),(252,355),(192,397),(132,355),(84,299)],'#F4DB8C' if gold else '#D8EAEE',sw=8)
        a.star(192,246,78,base);a.line([(50,89),(50,420)],'#FFF0CF',5);a.save('ui/pack_'+('gold' if gold else 'booster')+'.png')
    for cls,col in [('mage','#7773C5'),('archer','#6B9D50'),('warrior','#C95245')]:
        a=Art(128,128);shield(a,col)
        if cls=='mage':a.star(64,61,29,'#A7DBF6')
        if cls=='archer':a.line([(44,37),(82,87)],CREAM,6);a.poly([(74,68),(87,93),(64,78)],CREAM,sw=2);a.line([(79,31),(43,96)],CREAM,5)
        if cls=='warrior':a.poly([(64,26),(76,42),(69,73),(78,77),(78,84),(68,84),(68,98),(60,98),(60,84),(50,84),(50,77),(59,73),(52,42)],CREAM,sw=3)
        a.save('ui/relic_crest_'+cls+'.png')
    a=Art(128,128);a.rect((23,17,105,108),CREAM,sw=6,r=19)
    for x,y in [(43,38),(85,38),(64,62),(43,87),(85,87)]:a.ellipse((x-6,y-6,x+6,y+6),INK,None,0)
    a.save('ui/relic_lucky_dice.png')
    a=Art(128,128)
    for dx,dy in [(15,0),(7,6),(0,12)]:a.rect((20+dx,12+dy,96+dx,105+dy),BLUE,sw=4,r=6)
    a.star(58,68,25,CREAM);a.save('ui/relic_thick_pack.png')
    a=Art(128,128);a.poly([(34,26),(94,26),(84,48),(68,62),(84,79),(94,103),(34,103),(44,79),(60,62),(44,48)],'#B8DBDF',sw=4)
    a.poly([(42,34),(86,34),(64,56)],GOLD,None,0);a.poly([(64,77),(85,98),(43,98)],GOLD,None,0)
    for x in [25,103]:a.line([(x,20),(x,108)],'#A87947',7)
    for y in [16,103]:a.rect((16,y,112,y+12),'#BC9067',sw=4,r=4)
    a.save('ui/relic_hourglass.png')
    a=Art(512,512);a.rect((90,53,331,458),'#BC9067',sw=14,r=28);a.rect((112,77,307,458),'#344836',sw=8,r=16);a.poly([(119,81),(265,116),(265,448),(119,456)],'#A66844',sw=10);a.ellipse((228,253,246,273),GOLD,sw=3);a.poly([(300,236),(366,236),(366,186),(448,281),(366,371),(366,319),(300,319)],BLUE,sw=10);a.save('cards/art_sys_exit.png')
    a=Art(512,512);a.poly([(86,240),(256,88),(428,240)],'#B86C4A',sw=13);a.rect((116,232,397,440),CREAM,sw=12,r=9);a.rect((225,307,300,440),'#BC9067',sw=8,r=8);a.rect((142,275,199,335),BLUE,sw=7,r=5);a.line([(107,453),(411,453)],'#91B86D',13);a.save('cards/art_sys_title.png')
    a=Art(512,512);a.rect((170,174,342,349),CREAM,sw=11,r=25)
    for x,y in [(208,213),(300,213),(254,262),(208,308),(300,308)]:a.ellipse((x-12,y-12,x+12,y+12),INK,None,0)
    pts=[(256+170*math.cos(t),256+170*math.sin(t)) for t in [(-.7+i*5/70) for i in range(71)]];a.line(pts,INK,40);a.line(pts,BLUE,25);a.poly([(145,69),(91,167),(199,152)],BLUE,sw=10);a.save('cards/art_sys_retry.png')

if __name__=='__main__':make()
