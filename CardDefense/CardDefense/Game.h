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

// 손패 카드 번호: 0 이상은 유닛 종류(units.csv 행 번호), 음수는 화면 전환 카드
constexpr int CardStart = -1;  // 타이틀: 판에 놓으면 게임 시작
constexpr int CardExit = -2;   // 타이틀: 판에 놓으면 프로그램 종료
constexpr int CardRetry = -3;  // 결과: 다시 하기
constexpr int CardTitle = -4;  // 결과: 타이틀로

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

struct GameData
{
	std::vector<UnitData> units;
	std::vector<MobData> mobs;
	std::vector<RoundData> rounds;
	std::vector<int> starterUnits;     // 시작 손패 (유닛 종류 번호)
	std::vector<float> boosterTierWeights; // 부스터에서 유닛 등급 0~2가 나올 가중치
	float roundTimeLimit = 30.f;
	float spawnDuration = 15.f;
	int clearBonusBase = 3;
	int clearBonusPerRound = 1;
	int boosterPrice = 5;
	int sellChips = 1;  // 손패 카드를 팩에 끌어다 팔 때 받는 칩
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
enum class EventType { Summon, Hit, Death };
struct GameEvent
{
	EventType type;
	glm::vec3 position;
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

	std::vector<int> hand;   // 손패 카드 (위 카드 번호)
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
bool BuyPack(Game& game);                                // 칩으로 팩을 사서 손패에 카드 추가, 못 사면 false
bool PlaceUnit(Game& game, int handIndex, int i, int j); // 손패 유닛 카드를 타일에 놓기, 못 놓으면 false
bool CanPlayCard(const Game& game, int handIndex, int i, int j); // 이 칸에 이 카드를 놓을 수 있나
bool PlayCard(Game& game, int handIndex, int i, int j);  // 카드를 판에 놓기 (유닛 배치 또는 화면 전환)
bool SellCard(Game& game, int handIndex);                // 유닛 카드를 팔아 칩 받기
void EndTurn(Game& game);                                // 정비 → 전투

// 전투 페이즈 (GameUpdate에서 호출)
void SpawnMobs(Game& game, float dt);
void UpdateMobs(Game& game, float dt);
void UpdateUnits(Game& game, float dt);
void CheckRoundEnd(Game& game);                          // 라운드 클리어·승패 판정
