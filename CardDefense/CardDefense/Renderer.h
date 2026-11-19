#pragma once
#include "Game.h"

// 렌더링에 필요한 GPU 리소스와 카메라. 게임 로직은 바꾸지 않는다.

struct Renderer
{
	GLuint program = 0;
	Mesh axes, cube;
	Camera camera;
	bool isOrtho = false;
	int hoverI = -1, hoverJ = -1;  // 마우스가 가리키는 칸 (-1이면 없음)
	int selectedHand = 0;          // 놓을 손패 카드 번호 (임시: 숫자 키로 고름)
};

void RendererInit(Renderer& renderer);
glm::mat4 ViewMatrix(const Camera& camera);
glm::mat4 ProjectionMatrix(const Renderer& renderer, float aspect);
void RendererUpdate(Renderer& renderer, float dt);  // 카메라 자동 회전 같은 연출 쪽 업데이트
void RenderGame(const Renderer& renderer, const Game& game, int width, int height);
void RenderBoard(const Renderer& renderer);
void RenderUnits(const Renderer& renderer, const Game& game);
void RenderMobs(const Renderer& renderer, const Game& game);
