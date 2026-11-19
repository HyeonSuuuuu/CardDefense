#pragma once
#include "Game.h"
#include "Util/Obj.h"

// 렌더링에 필요한 GPU 리소스와 카메라. 게임 로직은 바꾸지 않는다.

// 화면 UI 사각형 (창 좌표: 왼쪽 위가 원점, y는 아래로)
struct Rect
{
	float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
};

struct Renderer
{
	GLuint program = 0;
	Mesh axes, cube, quad;
	std::vector<Model> unitModels;  // units.csv 순서
	std::vector<Model> mobModels;   // mobs.csv 순서
	Camera camera;
	bool isOrtho = false;
	int hoverI = -1, hoverJ = -1;  // 마우스가 가리키는 칸 (-1이면 없음)

	// UI 텍스처
	std::vector<GLuint> unitCards;  // units.csv 순서
	GLuint startCard = 0, exitCard = 0, retryCard = 0, titleCard = 0;
	GLuint digits = 0, chipIcon = 0, booster = 0, endTurn = 0, logo = 0, victory = 0, defeat = 0;

	// UI 입력 상태 (main이 채운다)
	glm::vec2 mouse = glm::vec2(0.f);  // 창 좌표
	int dragCard = -1;                 // 끌고 있는 손패 카드 번호 (-1이면 없음)
};

void RendererInit(Renderer& renderer);
glm::mat4 ViewMatrix(const Camera& camera);
glm::mat4 ProjectionMatrix(const Renderer& renderer, float aspect);
void RendererUpdate(Renderer& renderer, float dt);  // 카메라 자동 회전 같은 연출 쪽 업데이트
void RenderGame(const Renderer& renderer, const Game& game, int width, int height);
void RenderBoard(const Renderer& renderer, const Game& game);
void RenderUnits(const Renderer& renderer, const Game& game);
void RenderMobs(const Renderer& renderer, const Game& game);

// 화면 UI. 그리기와 클릭 판정이 같은 위치를 쓰도록 배치 함수를 둔다 (width, height = 창 크기).
bool Contains(const Rect& r, glm::vec2 p);
Rect HandCardRect(int index, int count, int width, int height);
int HandCardAt(const Game& game, glm::vec2 p, int width, int height);  // 그 위치의 손패 카드 번호, 없으면 -1
Rect PackRect(int width, int height);
Rect EndTurnRect(int width, int height);
void RenderUi(const Renderer& renderer, const Game& game, int width, int height);
