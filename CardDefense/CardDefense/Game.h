#pragma once
#include "Common.h"

// 게임 상태와 로직. 렌더링 코드는 넣지 않는다 (Renderer가 이 값을 읽어서 그린다).

enum class Phase { Title, Prepare, Battle, Result };

// ========================================================================
// 보드
//   6x6 칸: 안쪽 4x4는 말을 놓는 칸, 바깥 테두리는 몹이 도는 길
// ========================================================================

constexpr int BoardSize = 6;
constexpr float TileSize = 1.f;
constexpr int PathLength = (BoardSize - 1) * 4;  // 바깥 테두리 칸 수 = 20
constexpr int MaxRound = 10;
constexpr int MaxHand = 6;

// 손패 카드 번호: 0 ~ 99는 유닛 종류(units.csv 행 번호), 100부터는 주문(100 + spells.csv 행 번호), 음수는 화면 전환 카드
constexpr int CardSpellBase = 100;
constexpr int CardStart = -1;  // 타이틀: 판에 놓으면 게임 시작
constexpr int CardExit = -2;   // 타이틀: 판에 놓으면 프로그램 종료
constexpr int CardRetry = -3;  // 결과: 다시 하기
constexpr int CardTitle = -4;  // 결과: 타이틀로

constexpr int CardPromote = 200;  // 승격: 말 위에 놓으면 같은 클래스의 다음 등급으로 (골드 부스터에서만)

inline bool IsUnitCard(int card) { return card >= 0 && card < CardSpellBase; }
inline bool IsSpellCard(int card) { return card >= CardSpellBase && card < CardPromote; }

bool IsPathTile(int i, int j);         // 바깥 테두리면 경로 타일
glm::vec3 TileCenter(int i, int j);    // 타일 중심의 월드 좌표 (보드 중심 = 원점, y = 0)
bool WorldToTile(glm::vec3 p, int& i, int& j); // 월드 좌표가 놓인 타일. 판 밖이면 false
void PathTile(int index, int& i, int& j);      // 경로 index번째 칸 (0 ~ PathLength-1, 시계 방향)
glm::vec3 PathPosition(float pathPos); // 경로 위 위치(0 ~ PathLength, 칸 단위 실수)의 월드 좌표

// ========================================================================
// 수치 데이터
//   Data/*.csv에서 한 번 읽는다. 행 번호가 곧 종류 번호
// ========================================================================

enum class UnitClass { Mage, Archer, Warrior };

struct UnitData
{
	std::string id;
	UnitClass unitClass = UnitClass::Mage;
	int tier = 0;          // 0 일반 1 희귀 2 영웅
	float damage = 0.f;
	float interval = 1.f;  // 공격 간격(초)
	float range = 1.f;     // 사거리(칸)
	float splash = 0.f;    // 범위 반경(0이면 한 마리)
};

struct MobData
{
	std::string id;
	float hpScale = 1.f;
	float fixedHp = 0.f;   // 0보다 크면 배율 대신 이 체력(보스)
	float speed = 1.f;     // 초당 칸
	int chips = 0;
};

struct RoundData
{
	float baseHp = 0.f;
	std::vector<int> counts;  // mobs 순서대로 등장 수
};

enum class SpellTarget { Point, Slot, None };

struct SpellData
{
	std::string id;
	int rarity = 0;
	SpellTarget target = SpellTarget::None;
	float radius = 0.f;          // 칸 단위 범위
	float amount = 0.f;          // 효과량 = (amount + amountPerRound × 라운드) × (dice면 주사위 1~6)
	float amountPerRound = 0.f;
	bool dice = false;
	float multiplier = 1.f;      // 배율 효과 (전투의 북: 공격 속도)
	float duration = 0.f;        // 지속 시간(초)
};

enum class RelicEffect { RareChance, PackExtraCard, RoundTime, ClassDamage };

struct RelicData
{
	std::string id;
	RelicEffect effect = RelicEffect::RareChance;
	UnitClass targetClass = UnitClass::Mage;  // ClassDamage일 때만
	float amount = 0.f;
};

// 부스터 결과 한 줄 (pack_outcomes.csv)
enum class OutcomeKind { Relic, Unit, Spell, Promote };
struct PackOutcome
{
	OutcomeKind kind = OutcomeKind::Unit;
	int value = 0;      // Unit: 등급, Spell: spells 행 번호
	float weight = 0.f;
};

struct GameData
{
	std::vector<UnitData> units;
	std::vector<MobData> mobs;
	std::vector<RoundData> rounds;
	std::vector<int> starterUnits;     // 시작 손패 (유닛 종류 번호)
	std::vector<SpellData> spells;
	std::vector<RelicData> relics;
	std::vector<PackOutcome> boosterOutcomes;
	std::vector<PackOutcome> goldOutcomes;  // 골드 부스터: 희귀 이상 말, 승격, 주문, 유물
	int goldPrice = 15;
	float roundTimeLimit = 30.f;
	float spawnDuration = 15.f;
	int clearBonusBase = 3;
	int clearBonusPerRound = 1;
	int boosterPrice = 5;
	int sellChips = 1;  // 손패 카드를 팩에 끌어다 팔 때 받는 칩
	float betMobMultiplier = 1.5f;  // 베팅한 라운드의 몹 수 배율 (보스 제외)
	float jackpotChance = 0.05f;    // 부스터가 대박(카드 추가)일 확률
	int jackpotExtra = 2;           // 대박이면 더 나오는 카드 수
	int mergeCount = 3;             // 손패에 같은 말 카드가 이만큼 모이면 다음 등급 한 장으로 합성
};

extern GameData gameData;
bool LoadGameData();  // 실패하면 콘솔에 이유를 출력하고 false

// ========================================================================
// 게임 상태
//   말, 몹, 한 판의 전체 상태
// ========================================================================

// 필드에 놓인 말
struct Unit
{
	int type = 0;          // 유닛 종류 (units.csv 행 번호)
	int i = 0, j = 0;      // 놓인 타일
	float cooldown = 0.f;  // 다음 공격까지 남은 시간(초)
	float drumTimer = 0.f; // 전투의 북 남은 시간(초). 0보다 크면 공격 속도가 빨라진다
	float attackTimer = 0.f; // 공격한 뒤 지난 시간 (연출용)
	glm::vec3 attackTarget = glm::vec3(0.f); // 마지막으로 공격한 위치 (연출용)
};

// 경로를 도는 몹
struct Mob
{
	int type = 0;          // 몹 종류 (mobs.csv 행 번호)
	float hp = 0.f;
	float maxHp = 0.f;
	float pathPos = 0.f;   // 출발한 뒤 간 거리(칸). 한 바퀴를 넘어도 계속 커진다 → 클수록 앞선 몹
	float hitTimer = 0.f;  // 맞은 뒤 남은 번쩍임 시간 (연출용)
	float dieTimer = 0.f;  // 죽은 뒤 지난 시간 (연출용, dying 목록에서만 씀)
};

constexpr float DieTime = 0.7f;  // 죽은 몹이 사라지는 연출 시간(초)

// 게임에서 일어난 일. Renderer가 읽어 파티클 같은 연출을 만들고, main이 매 프레임 비운다.
// effects.csv 순서와 맞춘 앞의 넷은 파티클, 뒤는 UI 연출(value = 손패 번호 / 유물 번호, position.x = 1이면 골드 부스터)
enum class EventType { Summon, Hit, Death, Meteor, PackCard, PackRelic, MergeCard, Jackpot };
struct GameEvent
{
	EventType type;
	glm::vec3 position;
	int value = 0;
};

struct Game
{
	Phase phase = Phase::Title;
	float timer = 0.f;  // 현재 페이즈 경과 시간(초)

	int round = 1;
	int chips = 0;
	bool won = false;   // Result에서 승패 표시용
	int kills = 0;      // 결과 통계
	bool quit = false;  // Exit 카드를 놓으면 true (main이 창을 닫는다)
	bool bet = false;        // 정비 중 베팅 스위치. 턴 종료 때 betRound로 넘어간다
	bool betRound = false;   // 이번 전투가 베팅한 라운드인지 (깨면 유물)

	std::vector<int> hand;   // 손패 카드 (위 카드 번호)
	std::vector<int> relics; // 가진 유물 (relics.csv 행 번호, 같은 유물이 여러 번 들어갈 수 있다)
	std::vector<Unit> units;
	std::vector<Mob> mobs;
	std::vector<Mob> dying;      // 죽어서 사라지는 중인 몹 (연출용, 판정에는 안 씀)
	std::vector<int> spawnQueue; // 이번 라운드에 나올 몹 종류 순서
	std::vector<GameEvent> events; // 이번 프레임에 일어난 일 (연출용)
	int spawnedCount = 0;    // 이번 라운드에 나온 몹 수
};

// ========================================================================
// 게임 흐름
//   타이틀 → 정비 → 전투 → (정비 …) → 결과. 정비는 입력이, 전투는 GameUpdate가 진행한다
// ========================================================================

void GameInit(Game& game);
void StartGame(Game& game);   // 새 판: 스타터 손패를 받고 1라운드 정비로
void GameUpdate(Game& game, float dt);

// 정비 페이즈 조작 (말 놓기는 전투 중에도 된다)
bool BuyPack(Game& game, bool gold = false);             // 칩으로 팩(gold면 골드 부스터)을 사서 손패에 카드 추가, 못 사면 false
bool CanBuyPack(const Game& game, bool gold);
int PromotedType(int type);                              // 같은 클래스의 다음 등급 유닛 종류, 영웅이면 -1
bool PlaceUnit(Game& game, int handIndex, int i, int j); // 손패 유닛 카드를 타일에 놓기, 못 놓으면 false
bool CanPlayCard(const Game& game, int handIndex, int i, int j); // 이 칸에 이 카드를 놓을 수 있나
bool PlayCard(Game& game, int handIndex, int i, int j);  // 카드를 판에 놓기 (유닛 배치 또는 화면 전환)
bool SellCard(Game& game, int handIndex);                // 유닛·주문 카드를 팔아 칩 받기
void CastSpell(Game& game, int spell, int i, int j);     // 주문 효과 (PlayCard가 부른다)

// 유물을 반영한 값
int RelicCount(const Game& game, RelicEffect effect, UnitClass unitClass = UnitClass::Mage);
float RoundTimeLimit(const Game& game);
float UnitDamage(const Game& game, const Unit& unit);
int PackCardCount(const Game& game);                     // 팩 하나에서 나오는 결과 수
void EndTurn(Game& game);                                // 정비 → 전투
void ToggleBet(Game& game);                              // 정비 중 베팅 켜고 끄기

// 전투 페이즈 (GameUpdate에서 호출)
void SpawnMobs(Game& game, float dt);
void UpdateMobs(Game& game, float dt);
void UpdateUnits(Game& game, float dt);
void CheckRoundEnd(Game& game);                          // 라운드 클리어·승패 판정

// 발표용 단축키 (제안서 1장 조작표의 F1~F4). 3~5분 데모에서 10라운드를 다 할 수 없어서 둔다.
void DemoNextRound(Game& game);    // F1: 지금 라운드를 깬 것으로 하고 다음 라운드 정비로
void DemoAddChips(Game& game);     // F2: 칩 50개
void DemoBossRound(Game& game);    // F3: 10라운드(보스) 정비로
void DemoReplayEffects(Game& game); // F4: 소환·피격·처치 이펙트를 판 위에 한 번씩 보여 준다
