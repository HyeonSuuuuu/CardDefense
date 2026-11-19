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

static glm::vec3 UnitColor(const UnitData& ud)
{
	switch (ud.unitClass) {
	case UnitClass::Mage: return glm::vec3(0.35f, 0.45f, 0.95f);
	case UnitClass::Archer: return glm::vec3(0.30f, 0.75f, 0.35f);
	default: return glm::vec3(0.90f, 0.35f, 0.30f);
	}
}

void RenderUnits(const Renderer& renderer, const Game& game)
{
	// 임시: 말은 클래스 색 큐브, 등급이 높을수록 키가 크다 (모델이 준비되면 OBJ로 교체)
	for (const Unit& u : game.units) {
		const UnitData& ud = gameData.units[u.type];
		float height = 0.4f + 0.25f * ud.tier;
		float bump = u.attackTimer < 0.1f ? 1.15f : 1.f;  // 공격 순간 살짝 커진다

		glm::vec3 position = TileCenter(u.i, u.j) + glm::vec3(0.f, height * 0.5f, 0.f);
		glm::mat4 model = glm::translate(glm::mat4(1.f), position);
		model = glm::scale(model, glm::vec3(0.5f * bump, height, 0.5f * bump));
		SetVec3(renderer.program, "uColor", UnitColor(ud));
		Draw(renderer.program, renderer.cube, model);

		// 공격 연출(임시): 작은 큐브가 말 위에서 맞은 위치로 날아간다
		const float flyTime = 0.15f;
		if (u.attackTimer < flyTime) {
			float t = u.attackTimer / flyTime;
			glm::vec3 from = TileCenter(u.i, u.j) + glm::vec3(0.f, height, 0.f);
			glm::vec3 to = u.attackTarget + glm::vec3(0.f, 0.2f, 0.f);
			glm::mat4 shot = glm::translate(glm::mat4(1.f), glm::mix(from, to, t));
			shot = glm::scale(shot, glm::vec3(0.12f));
			SetVec3(renderer.program, "uColor", glm::vec3(1.f, 0.94f, 0.81f));
			Draw(renderer.program, renderer.cube, shot);
		}
	}
}

void RenderMobs(const Renderer& renderer, const Game& game)
{
	// 임시: 몹은 종류별 색 큐브. 칸마다 한 번씩 뛰어 전진하고, 머리 위에 체력 막대
	const glm::vec3 colors[] = {
		glm::vec3(0.85f, 0.85f, 0.80f),  // 해골 병사
		glm::vec3(0.55f, 0.80f, 0.25f),  // 고블린
		glm::vec3(0.45f, 0.35f, 0.25f),  // 오크
		glm::vec3(0.25f, 0.15f, 0.35f),  // 그림자 왕
	};
	const float sizes[] = { 0.35f, 0.28f, 0.5f, 0.75f };

	for (const Mob& m : game.mobs) {
		int type = m.type < 4 ? m.type : 0;
		float size = sizes[type];

		// 칸 사이를 지날 때 포물선으로 뛴다 (소수 부분 0 → 1 동안 sin 반 주기)
		float frac = m.pathPos - std::floor(m.pathPos);
		float hop = 0.35f * std::sin(frac * 3.14159265f);

		glm::vec3 position = PathPosition(m.pathPos) + glm::vec3(0.f, size * 0.5f + hop, 0.f);
		glm::mat4 model = glm::translate(glm::mat4(1.f), position);
		model = glm::scale(model, glm::vec3(size));
		glm::vec3 color = m.hitTimer > 0.f ? glm::vec3(1.f) : colors[type];
		SetVec3(renderer.program, "uColor", color);
		Draw(renderer.program, renderer.cube, model);

		// 체력 막대: 남은 비율만큼 왼쪽부터 채운다
		float ratio = glm::clamp(m.hp / m.maxHp, 0.f, 1.f);
		const float barWidth = 0.5f;
		glm::vec3 barLeft = position + glm::vec3(-barWidth * 0.5f, size * 0.5f + 0.15f, 0.f);
		glm::mat4 bar = glm::translate(glm::mat4(1.f), barLeft + glm::vec3(barWidth * ratio * 0.5f, 0.f, 0.f));
		bar = glm::scale(bar, glm::vec3(barWidth * ratio, 0.05f, 0.05f));
		SetVec3(renderer.program, "uColor", glm::vec3(0.9f, 0.2f, 0.2f));
		Draw(renderer.program, renderer.cube, bar);
	}
}
