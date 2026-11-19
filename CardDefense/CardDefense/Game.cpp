#include "Game.h"
#include "Util/Csv.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

GameData gameData;
static std::mt19937 rng{ std::random_device{}() };

// ========================================================================
// 보드
//   칸 번호와 월드 좌표 변환
// ========================================================================

bool IsPathTile(int i, int j)
{
	return i == 0 || i == BoardSize - 1 || j == 0 || j == BoardSize - 1;
}

glm::vec3 TileCenter(int i, int j)
{
	// 인덱스 0~5의 가운데(2.5)를 빼서 보드 중심을 원점에 맞춘다.
	float offset = (BoardSize - 1) * 0.5f;
	return glm::vec3((i - offset) * TileSize, 0.f, (j - offset) * TileSize);
}

bool WorldToTile(glm::vec3 p, int& i, int& j)
{
	// TileCenter의 역: 판 왼쪽 끝(-3)이 0이 되도록 밀고 칸 크기로 나눠 내림한다.
	float half = BoardSize * 0.5f;
	int ti = (int)std::floor(p.x / TileSize + half);
	int tj = (int)std::floor(p.z / TileSize + half);
	if (ti < 0 || ti >= BoardSize || tj < 0 || tj >= BoardSize) {
		return false;
	}
	i = ti;
	j = tj;
	return true;
}

void PathTile(int index, int& i, int& j)
{
	// 테두리를 네 변으로 나눠 (0,0)에서 출발해 한 바퀴 돈다. 변 하나 = 5칸.
	const int side = BoardSize - 1;
	index = ((index % PathLength) + PathLength) % PathLength;
	int k = index % side;
	switch (index / side) {
	case 0: i = k;        j = 0;        break;  // (0,0) → (4,0)
	case 1: i = side;     j = k;        break;  // (5,0) → (5,4)
	case 2: i = side - k; j = side;     break;  // (5,5) → (1,5)
	default: i = 0;       j = side - k; break;  // (0,5) → (0,1)
	}
}

glm::vec3 PathPosition(float pathPos)
{
	// 지금 칸과 다음 칸의 중심을 소수 부분으로 보간한다.
	int index = (int)std::floor(pathPos);
	float t = pathPos - index;
	int i0, j0, i1, j1;
	PathTile(index, i0, j0);
	PathTile(index + 1, i1, j1);
	return glm::mix(TileCenter(i0, j0), TileCenter(i1, j1), t);
}

// ========================================================================
// 수치 데이터
//   Data/*.csv 읽기
// ========================================================================

static int FindUnit(const std::string& id)
{
	for (int k = 0; k < (int)gameData.units.size(); ++k) {
		if (gameData.units[k].id == id) {
			return k;
		}
	}
	return -1;
}

bool LoadGameData()
{
	GameData& d = gameData;
	d = GameData{};
	CsvTable t;

	if (!LoadCsv("Data/units.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		UnitData u;
		u.id = row[CsvColumn(t, "id")];
		std::string c = row[CsvColumn(t, "class")];
		u.unitClass = c == "mage" ? UnitClass::Mage : c == "archer" ? UnitClass::Archer : UnitClass::Warrior;
		u.tier = std::stoi(row[CsvColumn(t, "tier")]);
		u.damage = std::stof(row[CsvColumn(t, "damage")]);
		u.interval = std::stof(row[CsvColumn(t, "interval")]);
		u.range = std::stof(row[CsvColumn(t, "range")]);
		u.splash = std::stof(row[CsvColumn(t, "splash")]);
		d.units.push_back(u);
	}

	if (!LoadCsv("Data/mobs.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		MobData m;
		m.id = row[CsvColumn(t, "id")];
		m.hpScale = std::stof(row[CsvColumn(t, "hp_scale")]);
		m.fixedHp = std::stof(row[CsvColumn(t, "fixed_hp")]);
		m.speed = std::stof(row[CsvColumn(t, "speed")]);
		m.chips = std::stoi(row[CsvColumn(t, "chips")]);
		d.mobs.push_back(m);
	}

	if (!LoadCsv("Data/rounds.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		RoundData r;
		r.baseHp = std::stof(row[CsvColumn(t, "base_hp")]);
		for (auto& m : d.mobs) {
			int column = CsvColumn(t, m.id.c_str());
			r.counts.push_back(column >= 0 ? std::stoi(row[column]) : 0);
		}
		d.rounds.push_back(r);
	}

	if (!LoadCsv("Data/economy.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		const std::string& key = row[0];
		const std::string& value = row[1];
		if (key == "round_time_limit") d.roundTimeLimit = std::stof(value);
		else if (key == "spawn_duration") d.spawnDuration = std::stof(value);
		else if (key == "clear_bonus_base") d.clearBonusBase = std::stoi(value);
		else if (key == "clear_bonus_per_round") d.clearBonusPerRound = std::stoi(value);
		else if (key == "starter_units") {
			std::stringstream ss(value);
			std::string id;
			while (ss >> id) {
				int type = FindUnit(id);
				if (type >= 0) {
					d.starterUnits.push_back(type);
				}
			}
		}
	}

	if (!LoadCsv("Data/packs.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		if (row[CsvColumn(t, "id")] == "booster") {
			d.boosterPrice = std::stoi(row[CsvColumn(t, "price")]);
		}
	}

	// 지금은 부스터의 유닛 결과만 쓴다 (유물·주문·골드 부스터는 나중에).
	if (!LoadCsv("Data/pack_outcomes.csv", t)) {
		return false;
	}
	d.boosterTierWeights = { 0.f, 0.f, 0.f };
	for (auto& row : t.rows) {
		const std::string& outcome = row[CsvColumn(t, "outcome")];
		if (row[CsvColumn(t, "pack")] == "booster" && outcome.compare(0, 5, "unit:") == 0) {
			int tier = std::stoi(outcome.substr(5));
			d.boosterTierWeights[tier] += std::stof(row[CsvColumn(t, "weight")]);
		}
	}

	if ((int)d.rounds.size() < MaxRound) {
		printf("rounds.csv: 라운드가 %d개 필요\n", MaxRound);
		return false;
	}
	return true;
}

// ========================================================================
// 게임 흐름
//   초기화와 매 프레임 업데이트
// ========================================================================

void GameInit(Game& game)
{
	game = Game{};
}

void StartGame(Game& game)
{
	game = Game{};
	game.phase = Phase::Prepare;
	game.hand = gameData.starterUnits;
}

void GameUpdate(Game& game, float dt)
{
	game.timer += dt;

	// 연출용 타이머는 페이즈와 상관없이 흐른다.
	for (Unit& u : game.units) {
		u.attackTimer += dt;
	}
	for (Mob& m : game.mobs) {
		m.hitTimer = std::max(0.f, m.hitTimer - dt);
	}

	switch (game.phase) {
	case Phase::Title:
		break;
	case Phase::Prepare:
		break;
	case Phase::Battle:
		SpawnMobs(game, dt);
		UpdateMobs(game, dt);
		UpdateUnits(game, dt);
		CheckRoundEnd(game);
		break;
	case Phase::Result:
		break;
	}
}

// ========================================================================
// 정비 페이즈
//   플레이어 조작(팩 사기, 말 놓기, 턴 종료)
// ========================================================================

bool BuyPack(Game& game)
{
	if (game.phase != Phase::Prepare || game.chips < gameData.boosterPrice || (int)game.hand.size() >= MaxHand) {
		return false;
	}
	game.chips -= gameData.boosterPrice;

	// 등급을 가중치로 뽑고, 클래스는 셋 중 무작위
	const std::vector<float>& w = gameData.boosterTierWeights;
	std::discrete_distribution<int> tierDist(w.begin(), w.end());
	int tier = tierDist(rng);
	UnitClass unitClass = (UnitClass)std::uniform_int_distribution<int>(0, 2)(rng);
	for (int k = 0; k < (int)gameData.units.size(); ++k) {
		if (gameData.units[k].tier == tier && gameData.units[k].unitClass == unitClass) {
			game.hand.push_back(k);
			break;
		}
	}
	return true;
}

bool PlaceUnit(Game& game, int handIndex, int i, int j)
{
	if (game.phase != Phase::Prepare && game.phase != Phase::Battle) {
		return false;
	}
	if (handIndex < 0 || handIndex >= (int)game.hand.size()) {
		return false;
	}
	if (i < 0 || i >= BoardSize || j < 0 || j >= BoardSize || IsPathTile(i, j)) {
		return false;
	}

	// 이미 말이 있으면 그 말을 없앤다 (환불 없음)
	for (int k = 0; k < (int)game.units.size(); ++k) {
		if (game.units[k].i == i && game.units[k].j == j) {
			game.units.erase(game.units.begin() + k);
			break;
		}
	}

	Unit u;
	u.type = game.hand[handIndex];
	u.i = i;
	u.j = j;
	u.attackTimer = 10.f;  // 처음엔 공격 연출이 보이지 않게
	game.units.push_back(u);
	game.hand.erase(game.hand.begin() + handIndex);
	return true;
}

void EndTurn(Game& game)
{
	if (game.phase != Phase::Prepare) {
		return;
	}
	game.phase = Phase::Battle;
	game.timer = 0.f;
	game.mobs.clear();
	game.spawnedCount = 0;

	// 등장 순서: 종류마다 (k + 0.5) / 수 위치에 놓고 정렬해 종류가 고르게 섞이게 한다.
	const RoundData& r = gameData.rounds[game.round - 1];
	std::vector<std::pair<float, int>> order;
	for (int type = 0; type < (int)r.counts.size(); ++type) {
		for (int k = 0; k < r.counts[type]; ++k) {
			order.push_back({ (k + 0.5f) / r.counts[type], type });
		}
	}
	std::stable_sort(order.begin(), order.end(),
		[](const std::pair<float, int>& a, const std::pair<float, int>& b) { return a.first < b.first; });
	game.spawnQueue.clear();
	for (auto& o : order) {
		game.spawnQueue.push_back(o.second);
	}

	for (Unit& u : game.units) {
		u.cooldown = 0.f;
	}
}

// ========================================================================
// 전투 페이즈
//   GameUpdate가 매 프레임 부른다
// ========================================================================

void SpawnMobs(Game& game, float dt)
{
	// spawnDuration 동안 고르게: n번째 몹은 n × 간격 초에 나온다.
	int total = (int)game.spawnQueue.size();
	if (total == 0) {
		return;
	}
	float interval = gameData.spawnDuration / total;
	const RoundData& r = gameData.rounds[game.round - 1];

	while (game.spawnedCount < total && game.timer >= game.spawnedCount * interval) {
		Mob m;
		m.type = game.spawnQueue[game.spawnedCount];
		const MobData& md = gameData.mobs[m.type];
		m.hp = md.fixedHp > 0.f ? md.fixedHp : std::round(r.baseHp * md.hpScale);
		m.maxHp = m.hp;
		game.mobs.push_back(m);
		++game.spawnedCount;
	}
}

void UpdateMobs(Game& game, float dt)
{
	for (Mob& m : game.mobs) {
		m.pathPos += gameData.mobs[m.type].speed * dt;
	}
}

void UpdateUnits(Game& game, float dt)
{
	for (Unit& u : game.units) {
		const UnitData& ud = gameData.units[u.type];
		u.cooldown -= dt;
		if (u.cooldown > 0.f) {
			continue;
		}

		// 사거리 안에서 가장 앞선 몹 (높이는 무시하고 바닥 거리로 잰다)
		glm::vec3 center = TileCenter(u.i, u.j);
		int target = -1;
		for (int k = 0; k < (int)game.mobs.size(); ++k) {
			glm::vec3 p = PathPosition(game.mobs[k].pathPos);
			if (glm::distance(center, p) <= ud.range &&
				(target < 0 || game.mobs[k].pathPos > game.mobs[target].pathPos)) {
				target = k;
			}
		}
		if (target < 0) {
			u.cooldown = 0.f;  // 준비된 채로 기다린다
			continue;
		}

		// 범위 공격이면 맞은 위치 주변 몹도 같은 피해
		glm::vec3 hitPos = PathPosition(game.mobs[target].pathPos);
		for (int k = 0; k < (int)game.mobs.size(); ++k) {
			bool inSplash = ud.splash > 0.f && glm::distance(hitPos, PathPosition(game.mobs[k].pathPos)) <= ud.splash;
			if (k == target || inSplash) {
				game.mobs[k].hp -= ud.damage;
				game.mobs[k].hitTimer = 0.15f;
			}
		}
		u.cooldown = ud.interval;
		u.attackTimer = 0.f;
		u.attackTarget = hitPos;
	}

	// 죽은 몹 제거, 칩 지급
	for (int k = (int)game.mobs.size() - 1; k >= 0; --k) {
		if (game.mobs[k].hp <= 0.f) {
			game.chips += gameData.mobs[game.mobs[k].type].chips;
			++game.kills;
			game.mobs.erase(game.mobs.begin() + k);
		}
	}
}

void CheckRoundEnd(Game& game)
{
	bool allSpawned = game.spawnedCount >= (int)game.spawnQueue.size();
	if (allSpawned && game.mobs.empty()) {
		game.chips += gameData.clearBonusBase + gameData.clearBonusPerRound * game.round;
		game.timer = 0.f;
		if (game.round >= MaxRound) {
			game.phase = Phase::Result;
			game.won = true;
		}
		else {
			++game.round;
			game.phase = Phase::Prepare;
		}
	}
	else if (game.timer >= gameData.roundTimeLimit) {
		game.phase = Phase::Result;
		game.won = false;
		game.timer = 0.f;
	}
}
