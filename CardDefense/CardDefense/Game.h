#pragma once
#include "Common.h"

// 게임 상태와 로직. 렌더링 코드는 넣지 않는다 (Renderer가 이 값을 읽어서 그린다).

enum class Phase { Title, Prepare, Battle, Result };

// 보드: 안쪽 4x4 유닛 타일 + 바깥 한 칸 두께의 경로 = 6x6
constexpr int BoardSize = 6;
constexpr float TileSize = 1.f;
constexpr int PathLength = (BoardSize - 1) * 4;  // 바깥 테두리 칸 수 = 20
constexpr int MaxRound = 10;
constexpr int MaxHand = 6;

bool IsPathTile(int i, int j);         // 바깥 테두리면 경로 타일
glm::vec3 TileCenter(int i, int j);    // 타일 중심의 월드 좌표 (보드 중심 = 원점, y = 0)
glm::vec3 PathPosition(float pathPos); // 경로 위 위치(0 ~ PathLength, 칸 단위 실수)의 월드 좌표

// 필드에 놓인 말
struct Unit
{
	int type = 0;          // 유닛 종류 (units.csv 행 번호)
	int i = 0, j = 0;      // 놓인 타일
	float cooldown = 0.f;  // 다음 공격까지 남은 시간(초)
};

// 경로를 도는 몹
struct Mob
{
	int type = 0;          // 몹 종류 (mobs.csv 행 번호)
	float hp = 0.f;
	float pathPos = 0.f;   // 경로 위 위치 (0 ~ PathLength, 넘으면 한 바퀴)
};

struct Game
{
	Phase phase = Phase::Title;
	float timer = 0.f;  // 현재 페이즈 경과 시간(초)

	int round = 1;
	int chips = 0;
	bool won = false;   // Result에서 승패 표시용

	std::vector<int> hand;   // 손패 카드 (유닛 종류 번호)
	std::vector<Unit> units;
	std::vector<Mob> mobs;
	int spawnedCount = 0;    // 이번 라운드에 나온 몹 수
};

void GameInit(Game& game);
void GameUpdate(Game& game, float dt);

// 정비 페이즈 조작
void BuyPack(Game& game);                                // 칩으로 팩을 사서 손패에 카드 추가
bool PlaceUnit(Game& game, int handIndex, int i, int j); // 손패 카드를 타일에 놓기, 못 놓으면 false
void EndTurn(Game& game);                                // 정비 → 전투

// 전투 페이즈 (GameUpdate에서 호출)
void SpawnMobs(Game& game, float dt);
void UpdateMobs(Game& game, float dt);
void UpdateUnits(Game& game, float dt);
void CheckRoundEnd(Game& game);                          // 라운드 클리어·승패 판정
