// 밸런스 시뮬레이션: 실제 Game.cpp 로직으로 자동 플레이 봇이 N판을 돌려 승률과 패배 라운드를 센다. 렌더링 없음.
// 봇: 정비마다 팩을 살 수 있는 만큼 사고, 강한 말(피해/간격 × (1+범위))부터 바깥쪽 칸(일반) 또는 가운데 칸(희귀 이상)에 놓는다.
//     칸이 다 차면 가장 약한 말과 바꾸고, 못 쓰는 카드는 판다.
// 빌드 (x64 Native Tools 명령 프롬프트, 프로젝트 폴더에서):
//   cl /EHsc /O2 /utf-8 /ICardDefense\CardDefense /Iexternal\include tools\balance_sim.cpp CardDefense\CardDefense\Game.cpp CardDefense\CardDefense\Util\Csv.cpp /Fe:balance_sim.exe
// 실행 (Data/가 있는 폴더에서): balance_sim.exe [판 수] [보스 체력 덮어쓰기] [라운드 체력 배율]
//   예) cd CardDefense\CardDefense && ..\..\balance_sim.exe 1000
#include "Game.h"
#include <cstdio>
#include <map>
#include <cmath>
#include <cstdlib>
static float Power(int t){ auto&u=gameData.units[t]; return u.damage/u.interval*(1+u.splash); }
static bool Center(int i,int j){ return (i==2||i==3)&&(j==2||j==3); }
static void Prepare(Game& g){
  while (BuyPack(g)) {}
  for (int pass=0; pass<12; ++pass){
    // best card in hand
    int best=-1; for(int h=0;h<(int)g.hand.size();++h) if(g.hand[h]>=0&&(best<0||Power(g.hand[h])>Power(g.hand[best]))) best=h;
    if(best<0) break;
    int type=g.hand[best]; bool hero=gameData.units[type].tier>=1;
    // free tile: prefer edge tiles for low tier, center for high tier
    int bi=-1,bj=-1;
    for(int i=1;i<=4;++i)for(int j=1;j<=4;++j){
      bool occ=false; for(auto&u:g.units) if(u.i==i&&u.j==j) occ=true;
      if(occ) continue;
      if(Center(i,j)&&!hero) continue;
      if(bi<0 || (hero&&Center(i,j))) {bi=i;bj=j;}
    }
    if(bi<0){ // replace weakest placed unit if stronger
      int w=-1; for(int k=0;k<(int)g.units.size();++k){ auto&u=g.units[k]; if(Center(u.i,u.j)&&!hero) continue; if(w<0||Power(u.type)<Power(g.units[w].type)) w=k; }
      if(w>=0 && Power(g.units[w].type)<Power(type)){ bi=g.units[w].i; bj=g.units[w].j; }
    }
    if(bi<0){ SellCard(g,best); continue; }
    PlaceUnit(g,best,bi,bj);
  }
  while (BuyPack(g)) {}  // spend chips from selling
}
int main(int argc,char**argv){
  if(!LoadGameData()){printf("load fail\n");return 1;}
  int N=argc>1?atoi(argv[1]):500;
  float boss=argc>2?atof(argv[2]):0, hpMul=argc>3?atof(argv[3]):1;
  if(boss>0) gameData.mobs[3].fixedHp=boss; for(auto&r:gameData.rounds) r.baseHp=std::round(r.baseHp*hpMul);
  double bossLeft=0; int bossLosses=0, boardSum=0; std::map<int,int> lostAt; int wins=0; double maxTimeR10=0;
  for(int n=0;n<N;++n){
    Game g; StartGame(g);
    while(g.phase!=Phase::Result){
      Prepare(g); int r=g.round; EndTurn(g);
      while(g.phase==Phase::Battle){ GameUpdate(g,1.f/60); g.events.clear(); }
      if(r==10 && g.phase==Phase::Result && g.won) {}
      if(g.phase==Phase::Result && !g.won){ lostAt[r]++; for(auto&m:g.mobs) if(m.type==3){bossLeft+=m.hp;bossLosses++;} }
    }
    if(g.won) wins++; boardSum+=(int)g.units.size();
  }
  printf("games %d wins %d (%.1f%%)\n",N,wins,100.0*wins/N);
  printf(" avg boss hp left when lost w/ boss: %.0f (%d)\n", bossLosses?bossLeft/bossLosses:0, bossLosses);
  for(auto&p:lostAt) printf(" lost at round %d: %d\n",p.first,p.second);
}
