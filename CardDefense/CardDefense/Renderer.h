#pragma once
#include "Game.h"

// 렌더링에 필요한 GPU 리소스와 카메라. 게임 로직은 바꾸지 않는다.

struct Renderer
{
	GLuint program = 0;
	Mesh axes, cube;
	Camera camera;
	bool isOrtho = false;
};

void RendererInit(Renderer& renderer);
void RendererUpdate(Renderer& renderer, float dt);  // 카메라 자동 회전 같은 연출 쪽 업데이트
void RenderGame(const Renderer& renderer, const Game& game, int width, int height);
void RenderBoard(const Renderer& renderer);
void RenderUnits(const Renderer& renderer, const Game& game);
void RenderMobs(const Renderer& renderer, const Game& game);
