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

glm::vec3 PathPosition(float pathPos)
{
	// TODO(최명준): 테두리를 한 바퀴 도는 칸 순서를 정하고, 두 칸 사이를 보간한다.
	return TileCenter(0, 0);
}

void GameInit(Game& game)
{
	game = Game{};
}

void GameUpdate(Game& game, float dt)
{
	game.timer += dt;

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

void BuyPack(Game& game)
{
	// TODO(최명준): 칩 차감, 손패가 MaxHand면 구매 불가
}

bool PlaceUnit(Game& game, int handIndex, int i, int j)
{
	// TODO(최명준): 경로 타일·이미 말이 있는 타일이면 false
	return false;
}

void EndTurn(Game& game)
{
	// TODO(최명준): 정비 → 전투 전환, 라운드 준비
}

void SpawnMobs(Game& game, float dt)
{
	// TODO(최명준): rounds.csv의 등장 수만큼 일정 간격으로 몹 추가
}

void UpdateMobs(Game& game, float dt)
{
	// TODO(최명준): pathPos를 속도 × dt만큼 증가
}

void UpdateUnits(Game& game, float dt)
{
	// TODO(최명준): 사거리 안에서 가장 앞선 몹 공격, 체력 0이면 제거·칩 지급
}

void CheckRoundEnd(Game& game)
{
	// TODO(최명준): 다 잡으면 다음 라운드(정비), 제한 시간 초과면 패배, 마지막 라운드 클리어면 승리
}
