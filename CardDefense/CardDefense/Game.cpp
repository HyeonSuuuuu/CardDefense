#include "Game.h"

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

void GameInit(Game& game)
{
	game = Game{};
}

void GameUpdate(Game& game, float dt)
{
	game.timer += dt;
}
