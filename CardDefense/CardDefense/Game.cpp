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

static int FindSpell(const std::string& id)
{
	for (int k = 0; k < (int)gameData.spells.size(); ++k) {
		if (gameData.spells[k].id == id) {
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
		else if (key == "sell_chips") d.sellChips = std::stoi(value);
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
		else if (row[CsvColumn(t, "id")] == "gold_booster") {
			d.goldPrice = std::stoi(row[CsvColumn(t, "price")]);
		}
	}

	if (!LoadCsv("Data/spells.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		SpellData sp;
		sp.id = row[CsvColumn(t, "id")];
		sp.rarity = std::stoi(row[CsvColumn(t, "rarity")]);
		std::string target = row[CsvColumn(t, "target")];
		sp.target = target == "point" ? SpellTarget::Point : target == "slot" ? SpellTarget::Slot : SpellTarget::None;
		sp.radius = std::stof(row[CsvColumn(t, "radius")]);
		sp.amount = std::stof(row[CsvColumn(t, "amount")]);
		sp.amountPerRound = std::stof(row[CsvColumn(t, "amount_per_round")]);
		sp.dice = row[CsvColumn(t, "dice")] == "1";
		sp.multiplier = std::stof(row[CsvColumn(t, "multiplier")]);
		sp.duration = std::stof(row[CsvColumn(t, "duration")]);
		d.spells.push_back(sp);
	}

	if (!LoadCsv("Data/relics.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		RelicData r;
		r.id = row[CsvColumn(t, "id")];
		std::string effect = row[CsvColumn(t, "effect")];
		r.effect = effect == "rare_chance" ? RelicEffect::RareChance
			: effect == "pack_extra_card" ? RelicEffect::PackExtraCard
			: effect == "round_time" ? RelicEffect::RoundTime : RelicEffect::ClassDamage;
		std::string c = row[CsvColumn(t, "target_class")];
		r.targetClass = c == "archer" ? UnitClass::Archer : c == "warrior" ? UnitClass::Warrior : UnitClass::Mage;
		r.amount = std::stof(row[CsvColumn(t, "amount")]);
		d.relics.push_back(r);
	}

	if (!LoadCsv("Data/pack_outcomes.csv", t)) {
		return false;
	}
	for (auto& row : t.rows) {
		const std::string& pack = row[CsvColumn(t, "pack")];
		if (pack != "booster" && pack != "gold_booster") {
			continue;
		}
		const std::string& text = row[CsvColumn(t, "outcome")];
		PackOutcome o;
		o.weight = std::stof(row[CsvColumn(t, "weight")]);
		if (text == "relic") {
			o.kind = OutcomeKind::Relic;
		}
		else if (text == "promote") {
			o.kind = OutcomeKind::Promote;
		}
		else if (text.compare(0, 5, "unit:") == 0) {
			o.kind = OutcomeKind::Unit;
			o.value = std::stoi(text.substr(5));
		}
		else if (text.compare(0, 6, "spell:") == 0) {
			o.kind = OutcomeKind::Spell;
			o.value = FindSpell(text.substr(6));
			if (o.value < 0) {
				continue;
			}
		}
		else {
			continue;
		}
		(pack == "booster" ? d.boosterOutcomes : d.goldOutcomes).push_back(o);
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
	game.hand = { CardStart, CardExit };
}

// 결과 화면으로: 손패를 Retry·Title 카드로 바꾼다
static void EnterResult(Game& game, bool won)
{
	game.phase = Phase::Result;
	game.won = won;
	game.timer = 0.f;
	game.hand = { CardRetry, CardTitle };
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
	for (int k = (int)game.dying.size() - 1; k >= 0; --k) {
		game.dying[k].dieTimer += dt;
		if (game.dying[k].dieTimer >= DieTime) {
			game.dying.erase(game.dying.begin() + k);
		}
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

// 팩 결과 가중치. 행운의 주사위 1개당 유닛 일반 등급 확률 5%p를 희귀·영웅에 나눠 준다 (일반이 없는 골드 부스터는 그대로)
static std::vector<float> PackWeights(const Game& game, const std::vector<PackOutcome>& outcomes)
{
	std::vector<float> w;
	float unitTotal = 0.f, common = 0.f, higher = 0.f;
	for (const PackOutcome& o : outcomes) {
		w.push_back(o.weight);
		if (o.kind == OutcomeKind::Unit) {
			unitTotal += o.weight;
			if (o.value == 0) {
				common += o.weight;
			}
			else {
				higher += o.weight;
			}
		}
	}
	float shift = std::min(common, unitTotal * 0.05f * RelicCount(game, RelicEffect::RareChance));
	for (int k = 0; k < (int)outcomes.size() && shift > 0.f && higher > 0.f; ++k) {
		if (outcomes[k].kind == OutcomeKind::Unit) {
			w[k] += outcomes[k].value == 0 ? -shift * w[k] / common : shift * w[k] / higher;
		}
	}
	return w;
}

static int RandomUnit(int tier)
{
	// 클래스는 셋 중 무작위
	UnitClass unitClass = (UnitClass)std::uniform_int_distribution<int>(0, 2)(rng);
	for (int k = 0; k < (int)gameData.units.size(); ++k) {
		if (gameData.units[k].tier == tier && gameData.units[k].unitClass == unitClass) {
			return k;
		}
	}
	return 0;
}

int PackCardCount(const Game& game)
{
	return 1 + RelicCount(game, RelicEffect::PackExtraCard);
}

bool CanBuyPack(const Game& game, bool gold)
{
	int price = gold ? gameData.goldPrice : gameData.boosterPrice;
	return game.phase == Phase::Prepare && game.chips >= price && (int)game.hand.size() + PackCardCount(game) <= MaxHand;
}

bool BuyPack(Game& game, bool gold)
{
	if (!CanBuyPack(game, gold)) {
		return false;
	}
	game.chips -= gold ? gameData.goldPrice : gameData.boosterPrice;

	const std::vector<PackOutcome>& outcomes = gold ? gameData.goldOutcomes : gameData.boosterOutcomes;
	std::vector<float> w = PackWeights(game, outcomes);
	std::discrete_distribution<int> pick(w.begin(), w.end());
	// 연출이 어느 팩에서 나올지: 이벤트 위치 x = 1이면 골드 부스터
	glm::vec3 from(gold ? 1.f : 0.f, 0.f, 0.f);
	for (int n = 0; n < PackCardCount(game); ++n) {
		const PackOutcome& o = outcomes[pick(rng)];
		if (o.kind == OutcomeKind::Relic) {
			int relic = std::uniform_int_distribution<int>(0, (int)gameData.relics.size() - 1)(rng);
			game.relics.push_back(relic);
			game.events.push_back({ EventType::PackRelic, from, relic });
			continue;
		}
		int card = o.kind == OutcomeKind::Unit ? RandomUnit(o.value)
			: o.kind == OutcomeKind::Spell ? CardSpellBase + o.value : CardPromote;
		game.hand.push_back(card);
		game.events.push_back({ EventType::PackCard, from, (int)game.hand.size() - 1 });
	}
	return true;
}

static Unit* UnitAt(Game& game, int i, int j)
{
	for (Unit& u : game.units) {
		if (u.i == i && u.j == j) {
			return &u;
		}
	}
	return nullptr;
}

static const Unit* UnitAt(const Game& game, int i, int j)
{
	return UnitAt(const_cast<Game&>(game), i, j);
}

int PromotedType(int type)
{
	const UnitData& ud = gameData.units[type];
	for (int k = 0; k < (int)gameData.units.size(); ++k) {
		if (gameData.units[k].unitClass == ud.unitClass && gameData.units[k].tier == ud.tier + 1) {
			return k;
		}
	}
	return -1;
}

bool CanPlayCard(const Game& game, int handIndex, int i, int j)
{
	if (handIndex < 0 || handIndex >= (int)game.hand.size()) {
		return false;
	}
	if (i < 0 || i >= BoardSize || j < 0 || j >= BoardSize) {
		return false;
	}
	int card = game.hand[handIndex];
	if (card < 0) {
		return true;  // 화면 전환 카드는 판 어디에 놓아도 된다
	}
	bool playing = game.phase == Phase::Prepare || game.phase == Phase::Battle;
	if (IsSpellCard(card)) {
		// 운석은 몹이 있는 전투 중에만, 나머지는 판 어디든
		const SpellData& sp = gameData.spells[card - CardSpellBase];
		return sp.target == SpellTarget::Point ? game.phase == Phase::Battle : playing;
	}
	const Unit* there = UnitAt(game, i, j);
	if (card == CardPromote) {
		// 영웅이 아닌 말 위에만
		return playing && there && PromotedType(there->type) >= 0;
	}
	// 같은 말 위에 같은 카드를 놓으면 바뀌는 것 없이 카드만 사라지므로 막는다
	return playing && !IsPathTile(i, j) && !(there && there->type == card);
}

bool PlaceUnit(Game& game, int handIndex, int i, int j)
{
	if (!CanPlayCard(game, handIndex, i, j) || !IsUnitCard(game.hand[handIndex])) {
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
	game.events.push_back({ EventType::Summon, TileCenter(i, j) });
	return true;
}

bool PlayCard(Game& game, int handIndex, int i, int j)
{
	if (!CanPlayCard(game, handIndex, i, j)) {
		return false;
	}
	switch (game.hand[handIndex]) {
	case CardStart:
	case CardRetry:
		StartGame(game);
		return true;
	case CardTitle:
		GameInit(game);
		return true;
	case CardExit:
		game.quit = true;
		return true;
	default:
		break;
	}
	int card = game.hand[handIndex];
	if (IsSpellCard(card)) {
		game.hand.erase(game.hand.begin() + handIndex);
		CastSpell(game, card - CardSpellBase, i, j);
		return true;
	}
	if (card == CardPromote) {
		Unit* u = UnitAt(game, i, j);
		u->type = PromotedType(u->type);
		u->cooldown = 0.f;
		game.hand.erase(game.hand.begin() + handIndex);
		game.events.push_back({ EventType::Summon, TileCenter(i, j) });
		return true;
	}
	return PlaceUnit(game, handIndex, i, j);
}

void CastSpell(Game& game, int spell, int i, int j)
{
	const SpellData& sp = gameData.spells[spell];
	float amount = sp.amount + sp.amountPerRound * game.round;
	if (sp.dice) {
		amount *= std::uniform_int_distribution<int>(1, 6)(rng);
	}
	glm::vec3 center = TileCenter(i, j);

	switch (sp.target) {
	case SpellTarget::Point:
		// 운석: 범위 안 몹에 피해. 죽은 몹은 다음 UpdateUnits에서 정리된다
		for (Mob& m : game.mobs) {
			if (glm::distance(center, PathPosition(m.pathPos)) <= sp.radius) {
				m.hp -= amount;
				m.hitTimer = 0.15f;
			}
		}
		game.events.push_back({ EventType::Meteor, center + glm::vec3(0.f, 0.2f, 0.f), (int)amount });
		break;
	case SpellTarget::Slot:
		// 전투의 북: 그 칸과 주변 8칸의 말
		for (Unit& u : game.units) {
			if (std::abs(u.i - i) <= (int)sp.radius && std::abs(u.j - j) <= (int)sp.radius) {
				u.drumTimer = sp.duration;
				game.events.push_back({ EventType::Summon, TileCenter(u.i, u.j) });
			}
		}
		break;
	case SpellTarget::None:
		game.chips += (int)amount;
		game.events.push_back({ EventType::Summon, center });
		break;
	}
}

int RelicCount(const Game& game, RelicEffect effect, UnitClass unitClass)
{
	int count = 0;
	for (int r : game.relics) {
		const RelicData& rd = gameData.relics[r];
		if (rd.effect == effect && (effect != RelicEffect::ClassDamage || rd.targetClass == unitClass)) {
			++count;
		}
	}
	return count;
}

float RoundTimeLimit(const Game& game)
{
	float bonus = 0.f;
	for (int r : game.relics) {
		if (gameData.relics[r].effect == RelicEffect::RoundTime) {
			bonus += gameData.relics[r].amount;
		}
	}
	return gameData.roundTimeLimit + bonus;
}

float UnitDamage(const Game& game, const Unit& unit)
{
	const UnitData& ud = gameData.units[unit.type];
	float bonus = 0.f;
	for (int r : game.relics) {
		const RelicData& rd = gameData.relics[r];
		if (rd.effect == RelicEffect::ClassDamage && rd.targetClass == ud.unitClass) {
			bonus += rd.amount * 0.01f;
		}
	}
	return ud.damage * (1.f + bonus);
}

bool SellCard(Game& game, int handIndex)
{
	if (game.phase != Phase::Prepare && game.phase != Phase::Battle) {
		return false;
	}
	if (handIndex < 0 || handIndex >= (int)game.hand.size() || game.hand[handIndex] < 0) {
		return false;
	}
	game.chips += gameData.sellChips;
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
		float speed = 1.f;
		if (u.drumTimer > 0.f) {
			u.drumTimer -= dt;
			speed = gameData.spells[FindSpell("battle_drum")].multiplier;
		}
		u.cooldown -= dt * speed;
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
				game.mobs[k].hp -= UnitDamage(game, u);
				game.mobs[k].hitTimer = 0.15f;
				game.events.push_back({ EventType::Hit, PathPosition(game.mobs[k].pathPos) + glm::vec3(0.f, 0.3f, 0.f) });
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
			game.dying.push_back(game.mobs[k]);
			game.events.push_back({ EventType::Death, PathPosition(game.mobs[k].pathPos) + glm::vec3(0.f, 0.3f, 0.f) });
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
			EnterResult(game, true);
		}
		else {
			++game.round;
			game.phase = Phase::Prepare;
		}
	}
	else if (game.timer >= RoundTimeLimit(game)) {
		EnterResult(game, false);
	}
}

// ========================================================================
// 발표용 단축키
//   게임 중(정비·전투)에만 동작한다
// ========================================================================

static bool InGame(const Game& game)
{
	return game.phase == Phase::Prepare || game.phase == Phase::Battle;
}

// 남은 몹을 없애고 다음 라운드 정비로 간다 (클리어 보너스는 주지 않는다)
static void GoToRound(Game& game, int round)
{
	game.mobs.clear();
	game.spawnQueue.clear();
	game.spawnedCount = 0;
	game.timer = 0.f;
	game.round = std::min(round, MaxRound);
	game.phase = Phase::Prepare;
}

void DemoNextRound(Game& game)
{
	if (InGame(game)) {
		GoToRound(game, game.round + 1);
	}
}

void DemoAddChips(Game& game)
{
	if (InGame(game)) {
		game.chips += 50;
	}
}

void DemoBossRound(Game& game)
{
	if (InGame(game)) {
		GoToRound(game, MaxRound);
	}
}

void DemoReplayEffects(Game& game)
{
	if (!InGame(game)) {
		return;
	}
	// 소환 먼지: 가운데 칸 / 피격: 앞쪽 길 / 처치: 해골 병사 하나가 디졸브되며 사라짐
	game.events.push_back({ EventType::Summon, TileCenter(2, 2) });
	glm::vec3 hitPos = PathPosition(12.f) + glm::vec3(0.f, 0.3f, 0.f);
	game.events.push_back({ EventType::Hit, hitPos });

	Mob dead;
	dead.type = 0;
	dead.pathPos = 13.f;
	dead.hp = 0.f;
	dead.maxHp = 1.f;
	game.dying.push_back(dead);
	game.events.push_back({ EventType::Death, PathPosition(dead.pathPos) + glm::vec3(0.f, 0.3f, 0.f) });
}
