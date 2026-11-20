// 자동 플레이 캡처: 숨긴 창에서 봇이 한 판을 하며 보고서용 화면을 BMP로 저장한다 (타이틀, 배치 미리보기, 전투, 툰/퐁, 디졸브, 보스, 결과).
// 빌드 (x64 Native Tools 명령 프롬프트, 프로젝트 폴더에서):
//   cl /EHsc /O2 /MD /utf-8 /ICardDefense\CardDefense /Iexternal\include tools\capture_playthrough.cpp CardDefense\CardDefense\Game.cpp CardDefense\CardDefense\Renderer.cpp CardDefense\CardDefense\Common.cpp CardDefense\CardDefense\Util\Csv.cpp CardDefense\CardDefense\Util\Obj.cpp CardDefense\CardDefense\Util\Texture.cpp CardDefense\CardDefense\Util\Capture.cpp CardDefense\CardDefense\Util\Picking.cpp /Fe:capture_playthrough.exe /link /LIBPATH:external\lib user32.lib gdi32.lib shell32.lib
// 실행 (Release 빌드 폴더에서, 저장할 폴더를 인자로): cd CardDefense\x64\Release && ..\..\..\capture_playthrough.exe C:\shots
// 봇이 질 수도 있다(승률 약 67%). 승리 화면이 필요하면 다시 실행한다.
#include "Renderer.h"
#include "Util/Capture.h"
#include <cstdio>
#include <cstdlib>
Game game; Renderer renderer; GLFWwindow* win; int W=1280,H=720; int glErrors=0;
static float Power(int t){ auto&u=gameData.units[t]; return u.damage/u.interval*(1+u.splash); }
static bool Center(int i,int j){ return (i==2||i==3)&&(j==2||j==3); }
static void Prepare(Game& g){
  while (BuyPack(g)) {}
  for (int pass=0; pass<12; ++pass){
    int best=-1; for(int h=0;h<(int)g.hand.size();++h) if(g.hand[h]>=0&&(best<0||Power(g.hand[h])>Power(g.hand[best]))) best=h;
    if(best<0) break;
    int type=g.hand[best]; bool hero=gameData.units[type].tier>=1; int bi=-1,bj=-1;
    for(int i=1;i<=4;++i)for(int j=1;j<=4;++j){ bool occ=false; for(auto&u:g.units) if(u.i==i&&u.j==j) occ=true;
      if(occ) continue; if(Center(i,j)&&!hero) continue; if(bi<0||(hero&&Center(i,j))){bi=i;bj=j;} }
    if(bi<0){ int w=-1; for(int k=0;k<(int)g.units.size();++k){ auto&u=g.units[k]; if(Center(u.i,u.j)&&!hero) continue; if(w<0||Power(u.type)<Power(g.units[w].type)) w=k; }
      if(w>=0&&Power(g.units[w].type)<Power(type)){bi=g.units[w].i;bj=g.units[w].j;} }
    if(bi<0){ SellCard(g,best); continue; }
    PlaceUnit(g,best,bi,bj);
    if((int)g.hand.size()<=2) break; // keep a couple of cards visible in hand
  }
}
static void Frame(const char* shot=nullptr){
  float dt=1.f/60; GameUpdate(game,dt); RendererUpdate(renderer,game,dt); game.events.clear();
  RenderGame(renderer,game,W,H); RenderUi(renderer,game,W,H);
  for(GLenum e; (e=glGetError())!=GL_NO_ERROR;) { if(glErrors++<5) printf("GL error 0x%x\n",e); }
  if(shot){ SaveScreenshotBmp(shot,W,H); printf("shot %s phase=%d round=%d chips=%d units=%d mobs=%d dying=%d\n",shot,(int)game.phase,game.round,game.chips,(int)game.units.size(),(int)game.mobs.size(),(int)game.dying.size()); }
  glfwSwapBuffers(win); glfwPollEvents();
}
static void RunUntil(float t){ while(game.phase==Phase::Battle && game.timer<t) Frame(); }
int main(int argc,char**argv){
  const char* out=argv[1];
  glfwInit(); glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
  glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE); glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);
  win=glfwCreateWindow(W,H,"shot",nullptr,nullptr); glfwMakeContextCurrent(win); glewExperimental=GL_TRUE; glewInit();
  if(!LoadGameData()) return 1; GameInit(game); RendererInit(renderer);
  glfwGetFramebufferSize(win,&W,&H); printf("fb %dx%d\n",W,H);
  char p[512]; auto P=[&](const char*n){ snprintf(p,512,"%s/%s",out,n); return p; };
  for(int k=0;k<30;++k) Frame(k==29?P("01_title.bmp"):nullptr);
  StartGame(game);
  // drag preview of first starter card over tile (1,1)
  for(int k=0;k<20;++k){ renderer.dragCard=0; renderer.hoverI=1; renderer.hoverJ=2; renderer.mouse=glm::vec2(640,360); Frame(k==19?P("02_drag_preview.bmp"):nullptr); }
  renderer.dragCard=-1; renderer.hoverI=-1;
  game.chips=10; Prepare(game); for(int k=0;k<8;++k) Frame(k==7?P("03_prepare_summon.bmp"):nullptr);
  bool shotR5=false, shotDie=false;
  while(game.phase!=Phase::Result){
    Prepare(game); int r=game.round; EndTurn(game);
    while(game.phase==Phase::Battle){
      const char* s=nullptr;
      if(r==1 && game.timer>=6.f && game.timer<6.f+1.f/60) s=P("04_battle_r1.bmp");
      if(r==5 && !shotR5 && game.timer>=9.f){ shotR5=true; renderer.toon=true; Frame(P("05_battle_r5_toon.bmp")); renderer.toon=false; Frame(P("06_battle_r5_phong.bmp")); renderer.toon=true; continue; }
      if(r>=6 && !shotDie && game.dying.size()>=2 && game.dying[0].dieTimer>0.3f){ shotDie=true; renderer.camera.distance=7.f; Frame(P("07_dissolve_zoom.bmp")); renderer.camera=Camera{}; continue; }
      if(r==10 && game.timer>=12.f && game.timer<12.f+1.f/60) s=P("08_boss_r10.bmp");
      Frame(s);
    }
    printf("round %d done -> phase %d won %d\n", r, (int)game.phase, (int)game.won);
  }
  for(int k=0;k<30;++k) Frame(k==29?P("09_result.bmp"):nullptr);
  printf("glErrors %d\n",glErrors); glfwTerminate(); return 0;
}
