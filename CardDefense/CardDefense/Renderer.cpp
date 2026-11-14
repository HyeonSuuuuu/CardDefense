#include "Renderer.h"

void RendererInit(Renderer& renderer)
{
	renderer.program = CreateProgram("shader.vert", "shader.frag");
	renderer.axes = CreateAxes();
	renderer.cube = CreateCube();
}

void RendererUpdate(Renderer& renderer, float dt)
{
	renderer.camera.yaw += renderer.camera.yawDir * dt * 30.f;
}

// 그리기와 마우스 피킹이 같은 행렬을 쓰도록 함수로 둔다.
glm::mat4 ViewMatrix(const Camera& camera)
{
	glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, camera.distance), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	view = glm::rotate(view, glm::radians(camera.pitch), glm::vec3(1.f, 0.f, 0.f));
	view = glm::rotate(view, glm::radians(-camera.yaw), glm::vec3(0.f, 1.f, 0.f));
	return view;
}

glm::mat4 ProjectionMatrix(const Renderer& renderer, float aspect)
{
	if (renderer.isOrtho) {
		return glm::ortho(-3.f * aspect, 3.f * aspect, -3.f, 3.f, 0.1f, 100.f);
	}
	return glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.f);
}

void RenderGame(const Renderer& renderer, const Game& game, int width, int height)
{
	glClearColor(0.1f, 0.1f, 0.15f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glUseProgram(renderer.program);
	glViewport(0, 0, width, height);

	float aspect = (float)width / height;
	SetMat4(renderer.program, "uProjection", ProjectionMatrix(renderer, aspect));
	SetMat4(renderer.program, "uView", ViewMatrix(renderer.camera));
	Draw(renderer.program, renderer.axes, glm::mat4(1.f));

	RenderBoard(renderer);
	RenderUnits(renderer, game);
	RenderMobs(renderer, game);
}

void RenderBoard(const Renderer& renderer)
{
	const glm::vec3 pathColor(0.74f, 0.56f, 0.40f);  // 나무 #BC9067
	const glm::vec3 tileColor(0.57f, 0.72f, 0.43f);  // 풀 #91B86D
	const glm::vec3 hoverColor(1.f, 0.94f, 0.81f);   // 카드 바탕 #FFF0CF
	const float height = 0.1f;
	const float gap = 0.05f;  // 타일 사이 틈 (경계가 보이도록)

	for (int i = 0; i < BoardSize; ++i) {
		for (int j = 0; j < BoardSize; ++j) {
			// 윗면이 y = 0에 오도록 두께의 절반만큼 내린다.
			glm::vec3 position = TileCenter(i, j) - glm::vec3(0.f, height * 0.5f, 0.f);
			glm::mat4 model = glm::translate(glm::mat4(1.f), position);
			model = glm::scale(model, glm::vec3(TileSize - gap, height, TileSize - gap));

			glm::vec3 color = IsPathTile(i, j) ? pathColor : tileColor;
			if (i == renderer.hoverI && j == renderer.hoverJ) {
				color = hoverColor;
			}
			SetVec3(renderer.program, "uColor", color);
			Draw(renderer.program, renderer.cube, model);
		}
	}
}

void RenderUnits(const Renderer& renderer, const Game& game)
{
	// TODO(이현수): 말마다 타일 위에 큐브 (모델이 준비되면 OBJ로 교체)
}

void RenderMobs(const Renderer& renderer, const Game& game)
{
	// TODO(이현수): 몹마다 PathPosition(mob.pathPos) 위치에 큐브
}
