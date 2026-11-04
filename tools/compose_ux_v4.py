"""Editable overlay; all world effects are in the perspective stage render."""
from compose_design import Art,image,save,CREAM,INK,DEST
from compose_ux_v2 import rotated,CARDS
import json
anchors=json.loads((DEST/'ux_v4_anchors.json').read_text())
for state in ['battle','place']:
 a=Art(1600,900);image(a,'design_preview/stage_v4_'+state+'.png',0,0,1600,900)
 a.text((108,38),'05 / 10',31,CREAM,INK,1)
 a.text((800,32),'18',38,CREAM,INK,1)
 a.rect((650,62,950,69),'#3B3534',None,0,3);a.rect((650,62,830,69),'#70AADC',None,0,3)
 a.text((800,91),'전투 중 · 남은 적 5',17,CREAM,INK,1)
 image(a,'ui/icon_chip.png',1440,19,35,35,True);a.text((1510,38),'24',30,CREAM,INK,1)
 for i,n in enumerate(['lucky_dice','crest_archer','hourglass']):image(a,'ui/relic_'+n+'.png',28+i*48,83,35,35,True)
 a.text((51,133),'×2',15,CREAM,INK,1)
 for i,it in enumerate(CARDS):
  if state=='place' and i==2:continue
  rotated(a,it,584+i*108,808+abs(i-2)*8,(i-2)*4,100)
 for i,n in enumerate(['booster','gold']):
  image(a,'ui/pack_'+n+'.png',40+i*82,760,58,77,True)
  a.text((69+i*82,862),'5 칩' if i==0 else '15 칩',18,CREAM,INK,1)
 a.text((800,890),'5 / 6',15,CREAM,INK,1)
 a.text((1470,825),'턴 종료',23,'#C1B8AA',INK,1)
 a.text((1470,860),'전투 종료 후 활성화',15,'#C1B8AA',INK,1)
 if state=='battle':
  x,y=anchors['hit'];a.rect((x-31,y-14,x+31,y-7),'#3A2925',None,0,2);a.rect((x-30,y-13,x+8,y-8),'#EC735C',None,0,1);a.text((x+36,y+20),'18',24,'#FFF0B8',INK,1)
 else:
  x,y=anchors['spawn'];a.text((x,y+65),'놓아서 소환',20,'#BCFFE6',INK,2)
  rotated(a,CARDS[2],1277,635,6,120)
  a.text((1280,745),'Esc 취소',17,CREAM,INK,1)
 save(a,'UX_v4_'+state)
