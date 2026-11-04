"""Layered SVG boards for Figma, with matching local PNG review copies."""
from build_ui_assets import Art, OUT, SRC, S, INK, CREAM, BLUE, GOLD, NAVY
from PIL import Image
from pathlib import Path
import base64,re,json,html
DEST=OUT/'design_preview';DEST.mkdir(exist_ok=True)
def image(a,path,x,y,w,h,vector=False):
    p=OUT/path
    im=Image.open(p).convert('RGBA').resize((round(w*S),round(h*S)),Image.Resampling.LANCZOS)
    a.im.alpha_composite(im,(round(x*S),round(y*S)))
    v=SRC/(p.stem+'.svg')
    if vector and v.exists() and p.stem!='digits':
        raw=v.read_text(encoding='utf-8');body=raw[raw.index('>')+1:raw.rindex('</svg>')];ow,oh=Image.open(p).size
        a.svg.append(f'<g id="{p.stem}" transform="translate({x},{y}) scale({w/ow},{h/oh})">{body}</g>')
    else:
        data=base64.b64encode(p.read_bytes()).decode();a.svg.append(f'<image id="{p.stem}" x="{x}" y="{y}" width="{w}" height="{h}" href="data:image/png;base64,{data}"/>')
def save(a,name):
    a.im.resize((a.w,a.h),Image.Resampling.LANCZOS).save(DEST/(name+'.png'))
    (DEST/(name+'.svg')).write_text(f'<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="{a.w}" height="{a.h}" viewBox="0 0 {a.w} {a.h}">'+''.join(a.svg).replace(' href=',' xlink:href=').replace('font-family="Malgun Gothic"','font-family="Noto Sans KR"')+'</svg>',encoding='utf-8')
def card(a,id,label,rank,x,y,w=148):
    h=w*1.5;image(a,'cards/frame_'+rank+'.png',x,y,w,h,True)
    image(a,'cards/art_'+id+'.png',x+w*.085,y+w*.1,w*.83,w*.83)
    a.text((x+w/2,y+h*.744),label,round(w*.105),INK)
    a.text((x+w/2,y+h*.887),{'common':'일반','rare':'희귀','hero':'영웅','system':'드래그하여 사용'}[rank],round(w*.075),INK)
def screen(kind):
    a=Art(1600,900);a.rect((0,0,1600,900),'#A5754E',None,0)
    image(a,'textures/table_wood.png',0,76,1600,824)
    a.rect((0,0,1600,76),NAVY,None,0)
    a.text((120,38),'0 / 10' if kind=='title' else '5 / 10',34)
    a.text((800,26),'30' if kind!='battle' else '18',34)
    a.rect((620,54,980,63),'#152A3B',None,0,4);a.rect((620,54,980 if kind!='battle' else 836,63),BLUE,None,0,4)
    image(a,'ui/icon_chip.png',1425,12,48,48,True);a.text((1510,36),'0' if kind=='title' else '24',34)
    image(a,'design_preview/board_'+('battle' if kind=='battle' else 'prepare')+'.png',300,70,1000,580)
    a.rect((24,103,209,147),NAVY,sw=2,r=13);a.text((116,125),{'battle':'전투 진행 중','prepare':'정비 시간','title':'카드로 시작'}[kind],20)
    if kind!='title':
        a.text((110,195),'보유 유물',16,CREAM)
        for i,n in enumerate(['lucky_dice','crest_archer','hourglass']):image(a,'ui/relic_'+n+'.png',38+i*56,222,48,48,True)
        a.rect((1360,112,1573,202),CREAM,sw=3,r=16);a.text((1466,140),'적 잔여  11',22,INK);a.text((1466,175),'처치하면 칩 획득',14,INK)
    a.rect((0,658,1600,900),'#513A2C',None,0)
    a.line([(0,658),(1600,658)],'#D6B07C',4)
    image(a,'ui/pack_booster.png',32,692,100,133,True);image(a,'ui/pack_gold.png',138,692,100,133,True)
    a.text((82,851),'5 칩',19);a.text((187,851),'15 칩',19)
    if kind=='battle':
        a.rect((1344,720,1566,792),'#59616A',INK,4,20);a.text((1455,754),'턴 종료',30,'#BAC0C7');a.text((1455,697),'전투 종료 후 활성화',15,CREAM)
    else:image(a,'ui/button_end_turn.png',1340,715,230,81,True)
    image(a,'ui/toggle_bet_off.png',1347,810,214,80,True)
    if kind=='title':
        image(a,'ui/logo.png',478,115,644,242,True)
        a.rect((515,561,1085,620),NAVY,sw=3,r=16);a.text((800,589),'시작 카드를 판 위에 놓아주세요',26)
        card(a,'sys_start','시작','system',620,674,142);card(a,'sys_exit','종료','system',795,674,142)
    else:
        data=[('warrior_1','꼬마 창병','common'),('archer_2','석궁수','rare'),('mage_1','꼬마 마법사','common'),('spell_meteor','운석 주사위','common'),('mage_3','대마법사','hero'),('archer_3','숲의 엘프','hero')]
        for i,(id,label,rank) in enumerate(data):card(a,id,label,rank,287+i*166,676,145)
        if kind=='prepare':
            a.rect((539,583,1061,631),NAVY,sw=2,r=12);a.text((800,605),'카드를 배치하고 다음 턴을 시작하세요',21)
    save(a,'screen_'+kind)
def assetsheet():
    groups=[('01  카드 틀 · 뒷면',list((OUT/'cards').glob('frame_*.png'))+[OUT/'cards/card_back.png']),('02  유닛 카드 그림',sorted((OUT/'cards').glob('art_mage_*.png'))+sorted((OUT/'cards').glob('art_archer_*.png'))+sorted((OUT/'cards').glob('art_warrior_*.png'))),('03  시스템 · 주문 · 승격',sorted((OUT/'cards').glob('art_sys_*.png'))+sorted((OUT/'cards').glob('art_spell_*.png'))+[OUT/'cards/art_promote.png']),('04  UI · 유물',sorted((OUT/'ui').glob('*.png'))),('05  판 · 작업대 텍스처',[OUT/'textures'/(n+'.png') for n in ['grass','path_stone','board_wood','table_wood']])]
    h=210+sum(80+((len(fs)+5)//6)*300 for _,fs in groups)
    a=Art(2040,h);a.rect((0,0,a.w,h),'#F1E7D5',None,0);a.text((1020,65),'카드 디펜스  /  리소스 라이브러리',48,INK);a.text((1020,131),'신규 40종 + 기존 카드 그림 3종  ·  PNG / RGBA  ·  2026.11.05',22,INK)
    y=195;manifest=[]
    for label,files in groups:
        a.rect((30,y,2010,y+55),NAVY,None,0,10);a.text((1020,y+26),label,26);y+=80
        for i,p in enumerate(files):
            x=30+(i%6)*335;yy=y+(i//6)*300;a.rect((x,yy,x+315,yy+278),'#FFFAF0','#D7C9AF',2,16)
            iw,ih=Image.open(p).size;sc=min(255/iw,203/ih);w,h1=iw*sc,ih*sc
            image(a,str(p.relative_to(OUT)).replace('\\','/'),x+(315-w)/2,yy+12+(203-h1)/2,w,h1,True)
            a.text((x+157,yy+239),p.name,17,INK);a.text((x+157,yy+263),f'{iw} × {ih}',14,'#83684F');manifest.append({'path':str(p.relative_to(OUT)).replace('\\','/'),'size':[iw,ih]})
        y+=((len(files)+5)//6)*300
    save(a,'resource_library');(DEST/'manifest.json').write_text(json.dumps(manifest,ensure_ascii=False,indent=2),encoding='utf-8')
if __name__=='__main__':
    for k in ['battle','prepare','title']:screen(k)
    assetsheet()
