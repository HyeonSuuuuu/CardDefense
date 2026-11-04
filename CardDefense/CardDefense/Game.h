#pragma once
#include "Common.h"

// 게임 상태와 로직. 렌더링 코드는 넣지 않는다 (Renderer가 이 값을 읽어서 그린다).

enum class Phase { Title, Prepare, Battle, Result };

// 보드: 안쪽 4x4 유닛 타일 + 바깥 한 칸 두께의 경로 = 6x6
constexpr int BoardSize = 6;
constexpr float TileSize = 1.f;

bool IsPathTile(int i, int j);         // 바깥 테두리면 경로 타일
glm::vec3 TileCenter(int i, int j);    // 타일 중심의 월드 좌표 (보드 중심 = 원점, y = 0)

struct Game
{
	Phase phase = Phase::Title;
	float timer = 0.f;  // 현재 페이즈 경과 시간(초)

	// 보드 타일, 유닛, 몹, 재화(칩) 등은 필요한 순서대로 여기에 추가한다.
};

void GameInit(Game& game);
void GameUpdate(Game& game, float dt);
