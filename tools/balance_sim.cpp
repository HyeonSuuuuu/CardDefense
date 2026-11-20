// 밸런스 시뮬레이션: 실제 Game.cpp 로직으로 자동 플레이 봇(bot.h)이 N판을 돌려 승률과 패배 라운드를 센다. 렌더링 없음.
// 빌드: x64 Native Tools 명령 프롬프트, 프로젝트 폴더에서 README의 '검증 도구' 명령. 실행: Data/가 있는 폴더에서 balance_sim.exe [판 수]
#include "Game.h"
#include "bot.h"
#include <cstdio>
#include <cstdlib>
#include <map>

int main(int argc, char** argv)
{
	if (!LoadGameData()) {
		printf("Data를 못 읽음\n");
		return 1;
	}
	int n = argc > 1 ? atoi(argv[1]) : 1000;
	int wins = 0, relicSum = 0, bets = 0;
	std::map<int, int> lostAt;
	for (int k = 0; k < n; ++k) {
		Game g;
		StartGame(g);
		float lastClear = 99.f;
		while (g.phase != Phase::Result) {
			BotPrepare(g);
			int round = g.round;
			// 직전 라운드를 18초 안에 깼으면 베팅 (보스 라운드 제외)
			g.bet = lastClear < 18.f && round < MaxRound;
			bets += g.bet ? 1 : 0;
			EndTurn(g);
			float t = 0.f;
			while (g.phase == Phase::Battle) {
				BotBattle(g);
				t = g.timer;
				GameUpdate(g, 1.f / 60.f);
				g.events.clear();
			}
			lastClear = t;
			if (!g.won && g.phase == Phase::Result) {
				++lostAt[round];
			}
		}
		wins += g.won ? 1 : 0;
		relicSum += (int)g.relics.size();
	}
	printf("%d판 중 %d승 (%.1f%%), 판당 유물 %.2f개, 판당 베팅 %.2f번\n", n, wins, 100.0 * wins / n, (double)relicSum / n, (double)bets / n);
	for (auto& p : lostAt) {
		printf("  %d라운드 패배: %d\n", p.first, p.second);
	}
	return 0;
}
