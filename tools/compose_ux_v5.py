"""Legible card-game HUD, large hand, explicit turn states."""
from compose_design import Art,image,save,CREAM,INK,DEST
from compose_ux_v2 import rotated,CARDS

def plaque(a,box,fill):
 x,y,x2,y2=box
 a.rect((x,y+5,x2,y2+5),'#30241D',None,0,12)
 a.rect(box,fill,'#382A23',3,12)

for state in ['battle','prepare','hover']:
 a=Art(1600,900);a.rect((0,0,1600,900),'#83786C',None,0)
 image(a,'design_preview/'+('stage_v5_prepare.png' if state=='prepare' else 'stage_v4_battle.png'),0,0,1600,900)
 plaque(a,(28,24,245,93),'#332D2A');a.text((136,58),'05 / 10',36)
 plaque(a,(627,16,973,112),'#332D2A')
 a.text((800,48),'정비 중' if state=='prepare' else '18초',38)
 if state=='prepare':a.text((800,88),'시간 정지',21,'#BCE4C5')
 else:
  a.rect((650,77,950,85),'#776C61',None,0,4);a.rect((650,77,830,85),'#83C9F3',None,0,4)
  a.text((800,135),'남은 적 5',22,CREAM,INK,1)
 plaque(a,(1370,24,1570,93),'#332D2A');image(a,'ui/icon_chip.png',1384,37,43,43,True);a.text((1491,58),'24',38)
 for i,n in enumerate(['lucky_dice','crest_archer','hourglass']):image(a,'ui/relic_'+n+'.png',30+i*66,116,52,52,True)
 a.text((57,187),'×2',21,CREAM,INK,1)
 # All names remain visible. Only the lower ornamental edge leaves the viewport.
 for i,it in enumerate(CARDS):
  if state=='hover' and i==2:continue
  rotated(a,it,548+i*126,790+abs(i-2)*14,(i-2)*5,145)
 plaque(a,(345,756,429,790),'#332D2A');a.text((387,772),'5 / 6',20)
 for i,n in enumerate(['booster','gold']):
  x=30+i*142
  plaque(a,(x,716,x+125,875),'#4B3B2D')
  image(a,'ui/pack_'+n+'.png',x+29,725,67,89,True)
  a.text((x+62,835),'부스터' if i==0 else '골드 팩',21)
  image(a,'ui/icon_chip.png',x+24,851,22,22,True);a.text((x+73,861),'5' if i==0 else '15',23)
 # State never relies on low opacity or color alone.
 active=state=='prepare';plaque(a,(1280,753,1570,842),'#E8BA55' if active else '#48515B')
 a.text((1425,796),'턴 종료  ▶' if active else '턴 종료',36,INK if active else CREAM)
 a.text((1425,869),'다음 라운드 시작' if active else '전투 중 · 종료 후 사용',21,CREAM,INK,1)
 if active:
  plaque(a,(1280,684,1570,735),'#332D2A');a.text((1425,709),'최대 베팅   꺼짐',23)
 else:a.text((1425,710),'팩 구매 · 카드 사용 가능',20,CREAM,INK,1)
 if state=='hover':
  rotated(a,CARDS[2],800,685,0,174)
  plaque(a,(935,518,1250,640),CREAM)
  a.text((1092,550),'꼬마 마법사',27,INK)
  a.text((1092,591),'공격 6 · 간격 1.2초',22,INK)
  a.text((1092,622),'사거리 1.8 · 범위 공격',20,INK)
 save(a,'UX_v5_'+state)

