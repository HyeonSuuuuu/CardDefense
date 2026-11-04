"""Editable game UX state boards; preserves v1 designs and source assets."""
from compose_design import Art,image,card,save,S,INK,CREAM,GOLD
from PIL import Image
import math
from pathlib import Path

CARDS=[('warrior_1','꼬마 창병','common'),('archer_2','석궁수','rare'),('mage_1','꼬마 마법사','common'),('spell_meteor','운석 주사위','common'),('mage_3','대마법사','hero')]

def rotated(a,item,cx,cy,angle,w=160):
    b=Art(w,int(w*1.5));card(b,*item,0,0,w)
    im=b.im.rotate(-angle,Image.Resampling.BICUBIC,expand=True)
    a.im.alpha_composite(im,(round(cx*S-im.width/2),round(cy*S-im.height/2)))
    a.svg.append(f'<g transform="translate({cx},{cy}) rotate({angle}) translate({-w/2},{-w*.75})">'+''.join(b.svg)+'</g>')

def paper(a,x,y,w,h,title,sub='',color=CREAM):
    a.poly([(x+7,y+8),(x+w+5,y+5),(x+w,y+h+8),(x,y+h+10)],'#422D20',None,0)
    a.poly([(x+6,y),(x+w,y+3),(x+w-5,y+h),(x,y+h-3)],color,INK,3)
    a.text((x+w/2,y+h*.38 if sub else y+h/2),title,25,INK)
    if sub:a.text((x+w/2,y+h*.75),sub,16,INK)

def base(state):
    battle=state in ['battle','danger','victory','defeat']
    a=Art(1600,900)
    image(a,'textures/table_wood.png',0,0,1600,900)
    a.rect((255,28,1345,795),'#395B4A','#C5A265',9,150)
    a.rect((272,45,1328,778),None,'#7D9168',2,135)
    image(a,'design_preview/board_'+('battle' if battle else 'prepare')+'.png',215,35,1170,679)
    paper(a,40,36,230,80,'ROUND 05 / 10','전투 중' if battle else '정비 중 · 시간 정지')
    a.ellipse((733,20,867,154),'#54392A',GOLD,5)
    a.ellipse((744,30,856,142),'#FAEAC6',INK,3)
    a.text((800,80),'05' if state=='danger' else '18' if battle else 'Ⅱ',44,'#BD3D2F' if state=='danger' else INK)
    a.text((800,119),'남은 시간' if battle else '정비 중',15,INK)
    if battle:paper(a,1340,38,220,78,'남은 적  3' if state=='danger' else '남은 적  11','모두 처치하면 승리')
    else:paper(a,1330,38,230,78,'다음 라운드','카드를 놓고 종을 울리세요')
    for i,n in enumerate(['lucky_dice','crest_archer','hourglass']):
        a.ellipse((49+i*69,151,109+i*69,211),'#775236',GOLD,2)
        image(a,'ui/relic_'+n+'.png',54+i*69,156,50,50,True)
    for i in range(3):image(a,'ui/icon_chip.png',65+i*28,438-i*12,66,66,True)
    a.text((142,533),'24 칩',32,CREAM,INK,2)
    image(a,'ui/pack_booster.png',39,595,108,144,True)
    image(a,'ui/pack_gold.png',157,589,108,144,True)
    a.text((94,768),'5 칩',23,CREAM,INK,2);a.text((210,762),'15 칩',23,CREAM,INK,2)
    a.text((150,821),'팩을 눌러 카드 뽑기',17,CREAM,INK,1)
    # A brass table bell, rather than a dashboard action bar.
    a.ellipse((1385,670,1535,711),'#39291F',None,0)
    a.poly([(1400,676),(1407,630),(1431,608),(1488,608),(1514,632),(1521,676)],'#C4933F' if not battle else '#7F7967',INK,4)
    a.ellipse((1447,592,1474,618),GOLD if not battle else '#7F7967',INK,3)
    a.rect((1380,675,1540,693),'#9D6632',INK,4,7)
    a.text((1460,737),'다음 턴' if not battle else '전투 중',27,CREAM,INK,2)
    a.text((1460,778),'종을 눌러 시작' if not battle else '카드 사용 가능',16,CREAM,INK,1)
    if not battle:paper(a,1340,455,220,100,'베팅 안 함','추가 적 ↔ 유물 보상')
    for i,it in enumerate(CARDS):
        if state in ['hover','place','replace'] and i==2:continue
        rotated(a,it,564+i*116,707+abs(i-2)*18,(i-2)*7,160)
    a.text((800,886),'카드를 잡아 판 위에 놓으세요  ·  손패 '+('6 / 6' if state=='full' else '5 / 6'),17,CREAM,INK,1)
    return a

def build(state):
    a=base(state)
    if state=='hover':
        rotated(a,CARDS[2],800,615,0,222)
        paper(a,945,515,290,150,'꼬마 마법사','공격 6 · 간격 1.2초 · 범위 1.8')
        a.text((1090,632),'범위 공격 · 빈 칸에 배치',17,INK)
    if state in ['place','replace']:
        replace=state=='replace';c='#F3BA50' if replace else '#B5F2B1'
        a.ellipse((645,300,1005,510),None,c,5)
        a.poly([(800,365),(868,394),(825,435),(756,406)],'#5A9A67' if not replace else '#9A6C37',c,5)
        a.text((815,400),'!' if replace else '+',35,c)
        rotated(a,CARDS[2],1040,440,8,148)
        paper(a,590,210,460,80,'기존 유닛이 사라집니다' if replace else '여기에 배치 가능','환불 없음 · 놓으면 교체 / Esc 취소' if replace else '공격 범위 미리보기 · 놓아서 소환',c)
    if state=='pack':
        a.star(184,560,155,'#F8D66D')
        rotated(a,CARDS[1],260,495,-8,190)
        paper(a,46,250,310,104,'새 카드!  석궁수','클릭하면 손패로 이동')
    if state=='full':
        rotated(a,('archer_3','숲의 엘프','hero'),1175,748,20,150)
        paper(a,35,290,340,115,'손패가 가득 찼어요','먼저 카드를 사용하세요 · 6 / 6')
    if state=='danger':
        a.rect((7,7,1593,893),None,'#C84835',12,12)
        paper(a,600,163,400,65,'5초 남음 · 적 3마리','', '#F7BC90')
    if state in ['pause','victory','defeat']:
        a.rect((450,240,1150,650),'#4A3428',GOLD,6,24)
        a.text((800,305),{'pause':'잠시 쉬어가기','victory':'라운드 클리어!','defeat':'방어 실패'}[state],48,CREAM)
        a.text((800,377),{'pause':'시간과 전투가 멈췄습니다','victory':'획득한 칩은 다음 정비에 사용할 수 있습니다','defeat':'시간 안에 모든 적을 처치하지 못했습니다'}[state],22,CREAM)
        if state=='pause':
            paper(a,560,440,480,70,'계속하기');a.text((800,567),'Esc로 전투 복귀',21,CREAM)
        elif state=='victory':
            image(a,'ui/icon_chip.png',745,414,100,100,True)
            paper(a,560,545,480,65,'정비로 계속')
        else:
            paper(a,510,465,265,94,'다시 도전');paper(a,825,465,265,94,'타이틀로')
    save(a,'ux_v2_'+state)

if __name__=='__main__':
    for state in ['prepare','hover','place','replace','pack','full','battle','danger','pause','victory','defeat']:build(state)
    dest=Path(__file__).resolve().parents[1]/'assets/design_preview'
    states=[('prepare','01 정비 · 종을 눌러 전투 시작'),('hover','02 카드 확인 · 마우스를 올리면 확대'),('place','03 배치 · 유효 칸과 사거리 미리보기'),('replace','04 교체 · 기존 유닛 소멸 / 환불 없음'),('pack','05 팩 개봉 · 새 카드가 손패로 이동'),('full','06 손패 가득 참 · 구매 차단과 이유'),('battle','07 전투 · 카드 사용 가능'),('danger','08 마지막 5초 · 시간과 잔여 적 강조'),('pause','09 일시정지 · Esc로 복귀'),('victory','10 라운드 클리어 · 정비로 연결'),('defeat','11 패배 · 재도전 / 타이틀')]
    body=['<rect width="5120" height="4450" fill="#EDE4D3"/>','<text x="60" y="95" font-family="Noto Sans KR" font-size="55" font-weight="700" fill="#3B2A20">카드 디펜스 · 테이블 위에서 플레이하는 UX</text>','<text x="60" y="153" font-family="Noto Sans KR" font-size="25" fill="#3B2A20">정비 → 카드 확인 / 배치 / 팩 개봉 → 전투 → 클리어 또는 패배</text>']
    for i,(state,title) in enumerate(states):
        x=60+(i%3)*1680;y=240+(i//3)*1030
        raw=(dest/('ux_v2_'+state+'.svg')).read_text(encoding='utf-8');inner=raw[raw.index('>')+1:raw.rindex('</svg>')]
        body.append(f'<text x="{x}" y="{y-25}" font-family="Noto Sans KR" font-size="29" font-weight="700" fill="#3B2A20">{title}</text><g id="UX_{state}" transform="translate({x},{y})">{inner}</g>')
    body.append('<text x="3420" y="3500" font-family="Noto Sans KR" font-size="32" fill="#3B2A20">상호작용 구현 기준</text>')
    notes=['카드: 호버 확대 → 드래그 → 유효 칸에 드롭','잘못된 위치: 원래 손패로 복귀 / Esc 취소','교체: 주황색 + 경고문으로 소멸과 환불 없음 표시','팩: 부족한 칩 수 표시 / 손패 6장일 때 구매 차단','베팅: 전투 시작 전에 추가 적과 유물 보상 안내','전투: 적중 / 선택 시 체력, 선택 시 사거리 노출','시간: 정비 / 일시정지에서는 정지, 마지막 5초 강조','연출 제안: 카드 상승 120ms / 배치 착지 180ms','사거리 표시는 UX 예시이며 실제 월드 좌표에 맞춰 구현','이 보드는 상태별 디자인 시안이며 실행형 프로토타입 아님']
    for i,t in enumerate(notes):body.append(f'<text x="3420" y="{3570+i*60}" font-family="Noto Sans KR" font-size="24" fill="#3B2A20">{t}</text>')
    (dest/'UX_v2_gameplay.svg').write_text('<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="5120" height="4450">'+''.join(body)+'</svg>',encoding='utf-8')

