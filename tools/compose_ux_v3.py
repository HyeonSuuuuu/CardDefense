from compose_design import Art,image,save,CREAM,INK
from compose_ux_v2 import rotated,CARDS

for state in ['battle','prepare']:
    a=Art(1600,900)
    image(a,'design_preview/stage_v3_'+state+'.png',0,0,1600,900)
    # Compact HUD sits outside the board, with no enclosing dashboard.
    a.text((130,40),'ROUND 05 / 10',25,CREAM,INK,1)
    a.text((800,35),'18초   ·   남은 적 11' if state=='battle' else '정비 중 · 시간 정지',26,CREAM,INK,1)
    image(a,'ui/icon_chip.png',1430,18,37,37,True)
    a.text((1500,38),'24',27,CREAM,INK,1)
    for i,n in enumerate(['lucky_dice','crest_archer','hourglass']):image(a,'ui/relic_'+n+'.png',25+i*45,82,35,35,True)
    for i,it in enumerate(CARDS):rotated(a,it,584+i*108,803+abs(i-2)*8,(i-2)*4,105)
    for i,n in enumerate(['booster','gold']):
        image(a,'ui/pack_'+n+'.png',40+i*86,744,62,83,True)
        a.text((71+i*86,854),'5 칩' if i==0 else '15 칩',18,CREAM,INK,1)
    a.text((800,890),'손패 5 / 6 · 카드에 마우스를 올려 확인',15,CREAM,INK,1)
    if state=='prepare':
        a.rect((1390,796,1565,853),'#3C5845','#CAB790',2,8)
        a.text((1478,824),'전투 시작',23)
        a.text((1478,760),'베팅 안 함',17,CREAM,INK,1)
    else:a.text((1480,835),'전투 중',21,CREAM,INK,1)
    save(a,'UX_v3_'+state)
