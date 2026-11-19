#include "Renderer.h"
#include "Util/Texture.h"
#include <cmath>

void RendererInit(Renderer& renderer)
{
	renderer.program = CreateProgram("shader.vert", "shader.frag");
	renderer.axes = CreateAxes();
	renderer.cube = CreateCube();
	renderer.quad = CreateQuad();

	// 모델 이름 = 수치 CSV의 id (assets/obj/<id>.obj). 못 읽으면 큐브로 그린다.
	for (const UnitData& ud : gameData.units) {
		Model model;
		LoadModel(("assets/obj/" + ud.id + ".obj").c_str(), model);
		renderer.unitModels.push_back(model);
	}
	for (const MobData& md : gameData.mobs) {
		Model model;
		LoadModel(("assets/obj/" + md.id + ".obj").c_str(), model);
		renderer.mobModels.push_back(model);
	}

	// 카드 그림은 tools/compose_cards.py로 합성한 완성 카드
	for (const UnitData& ud : gameData.units) {
		renderer.unitCards.push_back(LoadTexture(("assets/cards/card_" + ud.id + ".png").c_str()));
	}
	renderer.startCard = LoadTexture("assets/cards/card_sys_start.png");
	renderer.exitCard = LoadTexture("assets/cards/card_sys_exit.png");
	renderer.retryCard = LoadTexture("assets/cards/card_sys_retry.png");
	renderer.titleCard = LoadTexture("assets/cards/card_sys_title.png");
	renderer.digits = LoadTexture("assets/ui/digits.png");
	renderer.chipIcon = LoadTexture("assets/ui/icon_chip.png");
	renderer.booster = LoadTexture("assets/ui/pack_booster.png");
	renderer.endTurn = LoadTexture("assets/ui/button_end_turn.png");
	renderer.logo = LoadTexture("assets/ui/logo.png");
	renderer.victory = LoadTexture("assets/ui/text_victory.png");
	renderer.defeat = LoadTexture("assets/ui/text_defeat.png");
}

void RendererUpdate(Renderer& renderer, float dt)
{
	renderer.camera.yaw += renderer.camera.yawDir * dt * 30.f;
}

// 그리기와 마우스 피킹이 같은 행렬을 쓰도록 함수로 둔다.
glm::mat4 ViewMatrix(const Camera& camera)
{
	// 눈과 바라보는 점을 함께 0.7 내려서 판이 화면 위쪽에 오게 한다 (아래는 손패 자리).
	glm::vec3 shift(0.f, -0.7f, 0.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, camera.distance) + shift, shift, glm::vec3(0.f, 1.f, 0.f));
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
	glDisable(GL_BLEND);
	glUseProgram(renderer.program);
	glViewport(0, 0, width, height);

	float aspect = (float)width / height;
	SetMat4(renderer.program, "uProjection", ProjectionMatrix(renderer, aspect));
	glm::mat4 view = ViewMatrix(renderer.camera);
	SetMat4(renderer.program, "uView", view);
	SetVec3(renderer.program, "uViewPos", glm::vec3(glm::inverse(view)[3]));
	SetVec3(renderer.program, "uLightDir", glm::normalize(glm::vec3(0.4f, 1.f, 0.6f)));
	SetFloat(renderer.program, "uFlash", 0.f);
	SetFloat(renderer.program, "uAlpha", 1.f);
	SetVec4(renderer.program, "uUVRect", glm::vec4(0.f, 0.f, 1.f, 1.f));
	SetInt(renderer.program, "uUseTexture", 0);
	SetInt(renderer.program, "uLighting", 1);

	RenderBoard(renderer, game);
	RenderUnits(renderer, game);
	RenderMobs(renderer, game);
}

void RenderBoard(const Renderer& renderer, const Game& game)
{
	const glm::vec3 pathColor(0.74f, 0.56f, 0.40f);  // 나무 #BC9067
	const glm::vec3 tileColor(0.57f, 0.72f, 0.43f);  // 풀 #91B86D
	const glm::vec3 hoverColor(1.f, 0.94f, 0.81f);   // 카드 바탕 #FFF0CF: 카드를 놓을 수 있는 칸
	const glm::vec3 blockedColor(0.85f, 0.45f, 0.4f); // 놓을 수 없는 칸
	const float height = 0.1f;
	const float gap = 0.05f;  // 타일 사이 틈 (경계가 보이도록)

	for (int i = 0; i < BoardSize; ++i) {
		for (int j = 0; j < BoardSize; ++j) {
			// 윗면이 y = 0에 오도록 두께의 절반만큼 내린다.
			glm::vec3 position = TileCenter(i, j) - glm::vec3(0.f, height * 0.5f, 0.f);
			glm::mat4 model = glm::translate(glm::mat4(1.f), position);
			model = glm::scale(model, glm::vec3(TileSize - gap, height, TileSize - gap));

			glm::vec3 color = IsPathTile(i, j) ? pathColor : tileColor;
			// 카드를 끄는 동안만 가리키는 칸을 강조한다
			if (renderer.dragCard >= 0 && i == renderer.hoverI && j == renderer.hoverJ) {
				color = CanPlayCard(game, renderer.dragCard, i, j) ? hoverColor : blockedColor;
			}
			SetVec3(renderer.program, "uColor", color);
			Draw(renderer.program, renderer.cube, model);
		}
	}
}

// +Z를 보던 모델이 방향 dir(바닥 평면)을 보도록 하는 Y축 회전 각도
static float FacingYaw(glm::vec3 dir)
{
	return std::atan2(dir.x, dir.z);
}

// 모델이 있으면 모델(받침 바닥 = 원점, 높이 1)을, 없으면 큐브를 그린다.
static void DrawFigure(const Renderer& renderer, const Model& model, glm::vec3 base, float yaw, float height, glm::vec3 fallbackColor)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), base);
	m = glm::rotate(m, yaw, glm::vec3(0.f, 1.f, 0.f));
	if (model.mesh.vao) {
		SetVec3(renderer.program, "uColor", glm::vec3(1.f));
		DrawModel(renderer.program, model, glm::scale(m, glm::vec3(height)));
	}
	else {
		m = glm::translate(m, glm::vec3(0.f, height * 0.5f, 0.f));
		SetVec3(renderer.program, "uColor", fallbackColor);
		Draw(renderer.program, renderer.cube, glm::scale(m, glm::vec3(height * 0.6f, height, height * 0.6f)));
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
	// 등급이 높을수록 크게
	const float heights[] = { 0.7f, 0.8f, 0.95f };
	for (const Unit& u : game.units) {
		const UnitData& ud = gameData.units[u.type];
		float height = heights[ud.tier];
		float bump = u.attackTimer < 0.1f ? 1.08f : 1.f;  // 공격 순간 살짝 커진다

		// 마지막으로 공격한 쪽을 본다. 아직 공격 전이면 +Z(기본 카메라 쪽)
		glm::vec3 center = TileCenter(u.i, u.j);
		glm::vec3 toTarget = u.attackTarget - center;
		float yaw = glm::length(toTarget) > 0.01f && u.attackTimer < 9.f ? FacingYaw(toTarget) : 0.f;
		DrawFigure(renderer, renderer.unitModels[u.type], center, yaw, height * bump, UnitColor(ud));

		// 공격 연출(임시): 작은 큐브가 말 위에서 맞은 위치로 날아간다
		const float flyTime = 0.15f;
		if (u.attackTimer < flyTime) {
			float t = u.attackTimer / flyTime;
			glm::vec3 from = center + glm::vec3(0.f, height * 0.8f, 0.f);
			glm::vec3 to = u.attackTarget + glm::vec3(0.f, 0.3f, 0.f);
			glm::mat4 shot = glm::translate(glm::mat4(1.f), glm::mix(from, to, t));
			shot = glm::scale(shot, glm::vec3(0.1f));
			SetInt(renderer.program, "uLighting", 0);
			SetVec3(renderer.program, "uColor", glm::vec3(1.f, 0.94f, 0.81f));
			Draw(renderer.program, renderer.cube, shot);
			SetInt(renderer.program, "uLighting", 1);
		}
	}
}

void RenderMobs(const Renderer& renderer, const Game& game)
{
	// 몹은 칸마다 한 번씩 뛰어 전진하고, 머리 위에 체력 막대
	const glm::vec3 colors[] = {
		glm::vec3(0.85f, 0.85f, 0.80f),  // 해골 병사
		glm::vec3(0.55f, 0.80f, 0.25f),  // 고블린
		glm::vec3(0.45f, 0.35f, 0.25f),  // 오크
		glm::vec3(0.25f, 0.15f, 0.35f),  // 그림자 왕
	};
	const float heights[] = { 0.55f, 0.45f, 0.7f, 1.0f };

	for (const Mob& m : game.mobs) {
		int type = m.type < 4 ? m.type : 0;
		float height = heights[type];

		// 칸 사이를 지날 때 포물선으로 뛴다 (소수 부분 0 → 1 동안 sin 반 주기)
		float frac = m.pathPos - std::floor(m.pathPos);
		float hop = 0.35f * std::sin(frac * 3.14159265f);

		glm::vec3 ground = PathPosition(m.pathPos);
		glm::vec3 forward = PathPosition(std::floor(m.pathPos) + 1.f) - PathPosition(std::floor(m.pathPos));
		glm::vec3 base = ground + glm::vec3(0.f, hop, 0.f);

		SetFloat(renderer.program, "uFlash", m.hitTimer > 0.f ? 0.7f : 0.f);
		DrawFigure(renderer, renderer.mobModels[m.type], base, FacingYaw(forward), height, colors[type]);
		SetFloat(renderer.program, "uFlash", 0.f);

		// 체력 막대: 남은 비율만큼 왼쪽부터 채운다
		float ratio = glm::clamp(m.hp / m.maxHp, 0.f, 1.f);
		const float barWidth = 0.5f;
		// 카메라가 돈 만큼 막대도 돌려서 항상 화면에 가로로 보이게 한다.
		glm::mat4 bar = glm::translate(glm::mat4(1.f), base + glm::vec3(0.f, height + 0.12f, 0.f));
		bar = glm::rotate(bar, glm::radians(renderer.camera.yaw), glm::vec3(0.f, 1.f, 0.f));
		bar = glm::translate(bar, glm::vec3(-barWidth * 0.5f + barWidth * ratio * 0.5f, 0.f, 0.f));
		bar = glm::scale(bar, glm::vec3(barWidth * ratio, 0.05f, 0.05f));
		SetInt(renderer.program, "uLighting", 0);
		SetVec3(renderer.program, "uColor", glm::vec3(0.9f, 0.2f, 0.2f));
		Draw(renderer.program, renderer.cube, bar);
		SetInt(renderer.program, "uLighting", 1);
	}
}

// ========================================================================
// 화면 UI
//   창 좌표(왼쪽 위 원점, 픽셀)로 배치한다. 720 높이를 기준으로 비율을 맞춘다
// ========================================================================

static float UiScale(int height)
{
	return height / 720.f;
}

bool Contains(const Rect& r, glm::vec2 p)
{
	return p.x >= r.x && p.x <= r.x + r.w && p.y >= r.y && p.y <= r.y + r.h;
}

Rect HandCardRect(int index, int count, int width, int height)
{
	// 손패를 화면 아래 가운데에 한 줄로 늘어놓는다
	float s = UiScale(height);
	float cardW = 120.f * s, cardH = 180.f * s, step = 130.f * s;
	float rowW = cardW + step * (count - 1);
	return Rect{ width * 0.5f - rowW * 0.5f + step * index, height - cardH - 16.f * s, cardW, cardH };
}

int HandCardAt(const Game& game, glm::vec2 p, int width, int height)
{
	int count = (int)game.hand.size();
	for (int k = count - 1; k >= 0; --k) {  // 오른쪽 카드가 위에 그려지므로 뒤에서부터
		if (Contains(HandCardRect(k, count, width, height), p)) {
			return k;
		}
	}
	return -1;
}

Rect PackRect(int width, int height)
{
	float s = UiScale(height);
	return Rect{ 24.f * s, height - 190.f * s, 120.f * s, 160.f * s };
}

Rect EndTurnRect(int width, int height)
{
	float s = UiScale(height);
	return Rect{ width - 254.f * s, height - 110.f * s, 230.f * s, 80.f * s };
}

// 텍스처(없으면 단색) 사각형 하나
static void DrawRect(const Renderer& renderer, GLuint texture, const Rect& r, glm::vec3 color, float alpha)
{
	if (texture) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		SetInt(renderer.program, "uTexture", 0);
	}
	SetInt(renderer.program, "uUseTexture", texture ? 1 : 0);
	SetVec3(renderer.program, "uColor", color);
	SetFloat(renderer.program, "uAlpha", alpha);
	glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(r.x, r.y, 0.f));
	model = glm::scale(model, glm::vec3(r.w, r.h, 1.f));
	Draw(renderer.program, renderer.quad, model);
}

// 숫자 시트(64×96 칸 14개: 0~9 / × + :)로 글자를 그린다. align: 0 왼쪽, 0.5 가운데, 1 오른쪽 기준
static void DrawNumber(const Renderer& renderer, const char* text, float x, float y, float height, float align, glm::vec3 color)
{
	const char* glyphs = "0123456789/x+:";
	float glyphW = height * 64.f / 96.f;
	float advance = glyphW * 0.78f;  // 칸에 여백이 있어 조금 겹쳐 놓는다
	int length = (int)strlen(text);
	float left = x - (advance * (length - 1) + glyphW) * align;

	for (int k = 0; k < length; ++k) {
		const char* found = strchr(glyphs, text[k]);
		if (!found || text[k] == '\0') {
			continue;
		}
		int cell = (int)(found - glyphs);
		SetVec4(renderer.program, "uUVRect", glm::vec4(cell / 14.f, 0.f, 1.f / 14.f, 1.f));
		DrawRect(renderer, renderer.digits, Rect{ left + advance * k, y, glyphW, height }, color, 1.f);
	}
	SetVec4(renderer.program, "uUVRect", glm::vec4(0.f, 0.f, 1.f, 1.f));
}

static GLuint CardTexture(const Renderer& renderer, int card)
{
	switch (card) {
	case CardStart: return renderer.startCard;
	case CardExit: return renderer.exitCard;
	case CardRetry: return renderer.retryCard;
	case CardTitle: return renderer.titleCard;
	default: return card >= 0 && card < (int)renderer.unitCards.size() ? renderer.unitCards[card] : 0;
	}
}

void RenderUi(const Renderer& renderer, const Game& game, int width, int height)
{
	// 깊이 없이 그린 순서대로 덮고, 알파로 섞는다
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(renderer.program);
	SetMat4(renderer.program, "uProjection", glm::ortho(0.f, (float)width, (float)height, 0.f, -1.f, 1.f));
	SetMat4(renderer.program, "uView", glm::mat4(1.f));
	SetInt(renderer.program, "uLighting", 0);
	SetFloat(renderer.program, "uFlash", 0.f);

	float s = UiScale(height);
	const glm::vec3 white(1.f);
	const glm::vec3 dim(0.45f);
	bool playing = game.phase == Phase::Prepare || game.phase == Phase::Battle;
	char text[32];

	// 위 막대: 라운드 / 남은 시간 / 칩
	if (game.phase != Phase::Title) {
		DrawRect(renderer, 0, Rect{ 0.f, 0.f, (float)width, 64.f * s }, glm::vec3(0.17f, 0.2f, 0.25f), 0.85f);
		snprintf(text, sizeof(text), "%d/%d", game.round, MaxRound);
		DrawNumber(renderer, text, 24.f * s, 10.f * s, 44.f * s, 0.f, white);

		if (playing) {
			float limit = gameData.roundTimeLimit;
			float left = game.phase == Phase::Battle ? std::max(0.f, limit - game.timer) : limit;
			snprintf(text, sizeof(text), "%d", (int)std::ceil(left));
			bool hurry = game.phase == Phase::Battle && left <= 5.f;
			DrawNumber(renderer, text, width * 0.5f, 4.f * s, 42.f * s, 0.5f, hurry ? glm::vec3(1.f, 0.45f, 0.4f) : white);
			float barW = 300.f * s;
			DrawRect(renderer, 0, Rect{ width * 0.5f - barW * 0.5f, 50.f * s, barW, 8.f * s }, glm::vec3(0.1f), 0.8f);
			DrawRect(renderer, 0, Rect{ width * 0.5f - barW * 0.5f, 50.f * s, barW * left / limit, 8.f * s }, glm::vec3(0.4f, 0.75f, 0.95f), 1.f);
		}

		DrawRect(renderer, renderer.chipIcon, Rect{ width - 150.f * s, 10.f * s, 44.f * s, 44.f * s }, white, 1.f);
		snprintf(text, sizeof(text), "%d", game.chips);
		DrawNumber(renderer, text, width - 98.f * s, 10.f * s, 44.f * s, 0.f, white);
	}

	// 타이틀 로고, 결과 글자
	if (game.phase == Phase::Title) {
		DrawRect(renderer, renderer.logo, Rect{ width * 0.5f - 330.f * s, 40.f * s, 660.f * s, 248.f * s }, white, 1.f);
	}
	if (game.phase == Phase::Result) {
		GLuint texture = game.won ? renderer.victory : renderer.defeat;
		DrawRect(renderer, texture, Rect{ width * 0.5f - 320.f * s, 90.f * s, 640.f * s, 160.f * s }, white, 1.f);
	}

	// 팩 구매(정비 중에만), 턴 종료 버튼
	if (playing) {
		Rect pack = PackRect(width, height);
		bool canBuy = game.phase == Phase::Prepare && game.chips >= gameData.boosterPrice && (int)game.hand.size() < MaxHand;
		bool sellTarget = renderer.dragCard >= 0 && game.hand[renderer.dragCard] >= 0;  // 카드를 끌어 오면 판다
		glm::vec3 packColor = canBuy || sellTarget ? white : dim;
		if (sellTarget && Contains(pack, renderer.mouse)) {
			packColor = glm::vec3(1.f, 0.85f, 0.5f);
		}
		DrawRect(renderer, renderer.booster, pack, packColor, 1.f);
		DrawRect(renderer, renderer.chipIcon, Rect{ pack.x + pack.w * 0.5f - 34.f * s, pack.y + pack.h - 6.f * s, 28.f * s, 28.f * s }, white, 1.f);
		snprintf(text, sizeof(text), "%d", sellTarget ? gameData.sellChips : gameData.boosterPrice);
		DrawNumber(renderer, text, pack.x + pack.w * 0.5f - 2.f * s, pack.y + pack.h - 8.f * s, 32.f * s, 0.f, sellTarget ? glm::vec3(1.f, 0.85f, 0.4f) : white);

		Rect button = EndTurnRect(width, height);
		bool canEnd = game.phase == Phase::Prepare;
		bool pressed = canEnd && Contains(button, renderer.mouse);
		DrawRect(renderer, renderer.endTurn, button, canEnd ? (pressed ? glm::vec3(1.1f) : white) : dim, 1.f);
	}

	// 손패: 마우스가 올라간 카드는 조금 올리고, 끄는 카드는 마우스를 따라간다
	int count = (int)game.hand.size();
	int hovered = renderer.dragCard < 0 ? HandCardAt(game, renderer.mouse, width, height) : -1;
	for (int k = 0; k < count; ++k) {
		if (k == renderer.dragCard) {
			continue;
		}
		Rect r = HandCardRect(k, count, width, height);
		if (k == hovered) {
			r.y -= 24.f * s;
		}
		DrawRect(renderer, CardTexture(renderer, game.hand[k]), r, white, 1.f);
	}
	if (renderer.dragCard >= 0 && renderer.dragCard < count) {
		Rect r = HandCardRect(0, 1, width, height);
		r.w *= 0.8f;
		r.h *= 0.8f;
		r.x = renderer.mouse.x - r.w * 0.5f;
		r.y = renderer.mouse.y - r.h * 0.5f;
		// 판 위를 가리키면 반투명하게 해서 놓을 칸이 보이게 한다
		float alpha = renderer.hoverI >= 0 ? 0.55f : 1.f;
		DrawRect(renderer, CardTexture(renderer, game.hand[renderer.dragCard]), r, white, alpha);
	}

	SetFloat(renderer.program, "uAlpha", 1.f);
	SetInt(renderer.program, "uUseTexture", 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}
