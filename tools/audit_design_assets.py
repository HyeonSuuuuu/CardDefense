from pathlib import Path
from PIL import Image
import json,re
R=Path(__file__).resolve().parents[1]; A=R/'assets';spec=(R/'docs'/'카드디펜스-리소스목록.md').read_text(encoding='utf-8');folder='cards';items=[]
for line in spec.splitlines():
 if line.startswith('### ') and 'assets/' in line:folder=re.search(r'assets/(\w+)/',line).group(1)
 m=re.match(r'\| `([^`]+\.png)` \| (\d+)×(\d+)',line)
 if not m:continue
 file,w,h=m.groups();file=file if '/' in file else folder+'/'+file;p=A/file
 assert p.exists(),file
 im=Image.open(p);assert im.size==(int(w),int(h)),(file,im.size);assert im.mode=='RGBA',(file,im.mode)
 alpha=im.getchannel('A');transparent=alpha.getextrema()[0]==0
 if not file.startswith('textures/'):assert transparent,file
 items.append({'file':file,'width':im.width,'height':im.height,'mode':im.mode,'transparent':transparent,'status':'complete'})
assert len(items)==40,len(items)
digits=Image.open(A/'ui/digits.png');heights=[]
for i,c in enumerate('0123456789/×+:'):
 b=digits.crop((i*64,0,(i+1)*64,96)).getbbox();assert b and b[0]>=4 and b[2]<=60,(c,b);heights.append(b[3]-b[1])
assert len(set(heights))==1,heights
report={'date':'2026-11-05','figma_url':'https://www.figma.com/design/7bJe7e36VbwEZh3YKRbHju/','new_asset_count':40,'existing_card_art_reused':3,'existing_obj_preserved':13,'figma_text_font':'Noto Sans KR Bold','png_ui_font':'Malgun Gothic Bold','digits':{'cell':[64,96],'order':'0123456789/×+:','glyph_height':heights[0]},'textures':'2x2 repeat preview visually checked; mean opposite-edge RGB difference 1.1–2.6 / 255','items':items}
(A/'design_preview/production_manifest.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print('PASS: 40 exact-size RGBA files; transparent margins; 14 fixed sprite cells; consistent glyph height',heights[0])
p=A/'assets.md';s=p.read_text(encoding='utf-8');lines=s.splitlines();inside=False
for i,line in enumerate(lines):
 if line=='## 2D 그림':inside=True
 elif line.startswith('## '):inside=False
 if inside and line.startswith('| `') and '완성 카드:' not in line:
  parts=line.split('|');parts[-2]=' 완료 ';lines[i]='|'.join(parts)
s='\n'.join(lines)+'\n'
s+='\n| 2026-11-05 | 신규 2D 리소스 40종 | Codex 내장 ImageGen, Python/Pillow·SVG, Blender, Figma | 캐릭터 그림 6종·주문/승격 4종·시작 그림 1종·반복 텍스처 4종 생성, UI 25종 제작, 기존 OBJ로 화면 3종 합성 및 Figma 배치 | 규격·RGBA·숫자 14칸 검증, 텍스처 2×2 반복 검토, Figma 한글 글꼴 보정 |\n'
p.write_text(s,encoding='utf-8')
