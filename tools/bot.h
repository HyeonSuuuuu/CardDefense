#pragma once
// 자동 플레이 봇 (balance_sim, capture_playthrough가 같이 씀). 사람처럼 잘하지는 않고, 쉬운 규칙만 따른다.
#include "Game.h"

inline float BotPower(int card)
{
	const UnitData& u = gameData.units[card];
	return u.damage / u.interval * (1.f + u.splash);
}

inline bool BotCenter(int i, int j)
{
	return (i == 2 || i == 3) && (j == 2 || j == 3);
}

inline int BotFindCard(const Game& g, const char* spellId)
{
	for (int h = 0; h < (int)g.hand.size(); ++h) {
		if (IsSpellCard(g.hand[h]) && gameData.spells[g.hand[h] - CardSpellBase].id == spellId) {
			return h;
		}
	}
	return -1;
}

// 정비: 팩을 사고, 칩 주머니·전투의 북을 쓰고, 강한 말부터 놓는다. keepInHand장은 손패에 남긴다(캡처용)
inline void BotPrepare(Game& g, int keepInHand = 0)
{
	while (BuyPack(g)) {}
	for (int h; (h = BotFindCard(g, "chip_pouch")) >= 0;) {
		PlayCard(g, h, 2, 2);
		while (BuyPack(g)) {}
	}
	for (int h; (h = BotFindCard(g, "battle_drum")) >= 0 && !g.units.empty();) {
		PlayCard(g, h, 2, 2);
	}

	for (int pass = 0; pass < 12 && (int)g.hand.size() > keepInHand; ++pass) {
		int best = -1;
		for (int h = 0; h < (int)g.hand.size(); ++h) {
			if (IsUnitCard(g.hand[h]) && (best < 0 || BotPower(g.hand[h]) > BotPower(g.hand[best]))) {
				best = h;
			}
		}
		if (best < 0) {
			break;
		}
		int type = g.hand[best];
		bool strong = gameData.units[type].tier >= 1;
		int bi = -1, bj = -1;
		for (int i = 1; i <= 4; ++i) {
			for (int j = 1; j <= 4; ++j) {
				bool occupied = false;
				for (const Unit& u : g.units) {
					occupied |= u.i == i && u.j == j;
				}
				if (occupied || (BotCenter(i, j) && !strong)) {
					continue;
				}
				if (bi < 0 || (strong && BotCenter(i, j))) {
					bi = i;
					bj = j;
				}
			}
		}
		if (bi < 0) {
			int weakest = -1;
			for (int k = 0; k < (int)g.units.size(); ++k) {
				const Unit& u = g.units[k];
				if (BotCenter(u.i, u.j) && !strong) {
					continue;
				}
				if (weakest < 0 || BotPower(u.type) < BotPower(g.units[weakest].type)) {
					weakest = k;
				}
			}
			if (weakest >= 0 && BotPower(g.units[weakest].type) < BotPower(type)) {
				bi = g.units[weakest].i;
				bj = g.units[weakest].j;
			}
		}
		if (bi < 0) {
			SellCard(g, best);
			continue;
		}
		PlaceUnit(g, best, bi, bj);
	}
	// 손패가 꽉 차서 못 사면, 남은 일반 말 카드를 팔고 다시 산다
	while ((int)g.hand.size() >= MaxHand) {
		int sell = -1;
		for (int h = 0; h < (int)g.hand.size(); ++h) {
			if (IsUnitCard(g.hand[h]) && gameData.units[g.hand[h]].tier == 0) {
				sell = h;
			}
		}
		if (sell < 0) {
			break;
		}
		SellCard(g, sell);
	}
}

// 전투: 4초가 지나면 운석을 체력이 가장 많은 몹에 떨어뜨린다
inline void BotBattle(Game& g)
{
	if (g.timer < 4.f || g.mobs.empty()) {
		return;
	}
	int h = BotFindCard(g, "meteor_dice");
	if (h < 0) {
		return;
	}
	int target = 0;
	for (int k = 1; k < (int)g.mobs.size(); ++k) {
		if (g.mobs[k].hp > g.mobs[target].hp) {
			target = k;
		}
	}
	int i, j;
	if (WorldToTile(PathPosition(g.mobs[target].pathPos), i, j)) {
		PlayCard(g, h, i, j);
	}
}
