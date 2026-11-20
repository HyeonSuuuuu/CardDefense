#include "Renderer.h"
#include "Util/Texture.h"
#include "Util/Csv.h"
#include "Util/Easing.h"
#include <algorithm>
#include <random>
#include <cmath>

static std::mt19937 effectRng{ 1234u };
static const glm::vec3 LightDir = glm::normalize(glm::vec3(0.7f, 1.f, 0.5f));  // 표면에서 빛 쪽으로
static const int ShadowSize = 2048;
static bool shadowPass = false;  // 그림자 맵을 그리는 중이면 외곽선 등 깊이에 필요 없는 것을 건너뛴다

static float RandomRange(float a, float b)
{
	return std::uniform_real_distribution<float>(a, std::max(a, b))(effectRng);
}

// "#RRGGBBAA" → 0~1 색
static glm::vec4 ParseColor(const std::string& text)
{
	unsigned value = (unsigned)std::stoul(text.substr(1), nullptr, 16);
	return glm::vec4((value >> 24) & 255, (value >> 16) & 255, (value >> 8) & 255, value & 255) / 255.f;
}

static void LoadEffects(Renderer& renderer)
{
	// EventType 순서에 맞춰 effects.csv에서 이름으로 찾는다
	const char* names[] = { "SummonDust", "Hit", "Death", "Meteor" };
	const int count = 4;
	renderer.effects.assign(count, EffectData{});
	CsvTable t;
	if (!LoadCsv("Data/effects.csv", t)) {
		return;
	}
	for (auto& row : t.rows) {
		for (int k = 0; k < count; ++k) {
			if (row[CsvColumn(t, "type")] != names[k]) {
				continue;
			}
			EffectData& e = renderer.effects[k];
			e.ring = row[CsvColumn(t, "shape")] == "ring";
			e.count = std::stoi(row[CsvColumn(t, "count")]);
			e.speedMin = std::stof(row[CsvColumn(t, "speed_min")]);
			e.speedMax = std::stof(row[CsvColumn(t, "speed_max")]);
			e.lifeMin = std::stof(row[CsvColumn(t, "life_min")]);
			e.lifeMax = std::stof(row[CsvColumn(t, "life_max")]);
			e.sizeStart = std::stof(row[CsvColumn(t, "size_start")]);
			e.sizeEnd = std::stof(row[CsvColumn(t, "size_end")]);
			e.colorStart = ParseColor(row[CsvColumn(t, "color_start")]);
			e.colorEnd = ParseColor(row[CsvColumn(t, "color_end")]);
			e.gravity = std::stof(row[CsvColumn(t, "gravity")]);
		}
	}
}

// 파티클용 둥근 점 텍스처를 코드로 만든다 (가운데 1 → 가장자리 0)
static GLuint CreateDotTexture()
{
	const int size = 64;
	std::vector<unsigned char> pixels(size * size * 4);
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			float dx = (x + 0.5f) / size * 2.f - 1.f;
			float dy = (y + 0.5f) / size * 2.f - 1.f;
			float a = glm::clamp(1.f - std::sqrt(dx * dx + dy * dy), 0.f, 1.f);
			unsigned char* p = &pixels[(y * size + x) * 4];
			p[0] = p[1] = p[2] = 255;
			p[3] = (unsigned char)(255.f * std::min(1.f, a * 1.8f));
		}
	}
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void RendererInit(Renderer& renderer)
{
	renderer.program = CreateProgram("shader.vert", "shader.frag");
	renderer.axes = CreateAxes();
	renderer.cube = CreateCube();
	renderer.quad = CreateQuad();
	renderer.disc = CreateDisc(48);
	renderer.circle = CreateCircle(48);
	renderer.roundTile = CreateRoundedTile(0.18f, 8);
	renderer.dotTexture = CreateDotTexture();

	// 그림자 맵: 색 없이 깊이만 받는 프레임버퍼. 맵 밖은 깊이 1(가장 멂)로 읽혀 그림자가 생기지 않는다
	glGenTextures(1, &renderer.shadowMap);
	glBindTexture(GL_TEXTURE_2D, renderer.shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, ShadowSize, ShadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	const float border[] = { 1.f, 1.f, 1.f, 1.f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glGenFramebuffers(1, &renderer.shadowFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer.shadowFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, renderer.shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("shadow framebuffer incomplete\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	LoadEffects(renderer);

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

	renderer.grassTexture = LoadTexture("assets/textures/grass.png");
	renderer.stoneTexture = LoadTexture("assets/textures/path_stone.png");
	renderer.woodTexture = LoadTexture("assets/textures/board_wood.png");
	renderer.tableTexture = LoadTexture("assets/textures/table_wood.png");

	// 카드 그림은 tools/compose_cards.py로 합성한 완성 카드
	for (const UnitData& ud : gameData.units) {
		renderer.unitCards.push_back(LoadTexture(("assets/cards/card_" + ud.id + ".png").c_str()));
	}
	for (const SpellData& sp : gameData.spells) {
		renderer.spellCards.push_back(LoadTexture(("assets/cards/card_" + sp.id + ".png").c_str()));
	}
	// 유물 아이콘 파일 이름은 id에서 앞뒤를 바꾼 것 (mage_crest → relic_crest_mage)
	for (const RelicData& rd : gameData.relics) {
		std::string file = rd.id;
		size_t bar = file.find('_');
		if (file.compare(bar + 1, std::string::npos, "crest") == 0) {
			file = "crest_" + file.substr(0, bar);
		}
		renderer.relicIcons.push_back(LoadTexture(("assets/ui/relic_" + file + ".png").c_str()));
		renderer.relicTips.push_back(LoadTexture(("assets/ui/tip_" + rd.id + ".png").c_str()));
	}
	renderer.cardBack = LoadTexture("assets/cards/card_back.png");
	renderer.promoteCard = LoadTexture("assets/cards/card_promote.png");
	renderer.goldPack = LoadTexture("assets/ui/pack_gold.png");
	renderer.betOn = LoadTexture("assets/ui/toggle_bet_on.png");
	renderer.betOff = LoadTexture("assets/ui/toggle_bet_off.png");
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

void RendererUpdate(Renderer& renderer, const Game& game, float dt)
{
	renderer.camera.yaw += renderer.camera.yawDir * dt * 30.f;

	// 이번 프레임의 게임 이벤트마다 파티클을 뿌린다
	int packCount = 0;
	for (const GameEvent& event : game.events) {
		if (event.type == EventType::Jackpot) {
			renderer.jackpotTimer = 0.f;
			renderer.jackpotSource = (int)event.position.x;
			continue;
		}
		if (event.type == EventType::PackCard || event.type == EventType::PackRelic || event.type == EventType::MergeCard) {
			Renderer::PackAnim anim;
			anim.handIndex = event.type == EventType::PackRelic ? -1 : event.value;
			anim.relic = event.type == EventType::PackRelic ? event.value : -1;
			anim.delay = 0.25f * packCount++;
			anim.source = (int)(event.position.x + 0.5f);
			anim.merged = event.type == EventType::MergeCard;
			renderer.packAnims.push_back(anim);
			renderer.packHandSize = (int)game.hand.size();
			continue;
		}
		int k = (int)event.type;
		if (k < 0 || k >= (int)renderer.effects.size()) {
			continue;
		}
		const EffectData& e = renderer.effects[k];
		for (int n = 0; n < e.count; ++n) {
			Particle p;
			p.effect = k;
			p.life = RandomRange(e.lifeMin, e.lifeMax);
			float speed = RandomRange(e.speedMin, e.speedMax);
			float angle = RandomRange(0.f, 6.2831853f);
			glm::vec3 dir;
			if (e.ring) {
				dir = glm::vec3(std::cos(angle), 0.15f, std::sin(angle));  // 바닥을 따라 둥글게
			}
			else {
				float up = RandomRange(-0.3f, 1.f);  // 사방으로, 위쪽이 조금 더 많게
				float side = std::sqrt(1.f - up * up);
				dir = glm::vec3(side * std::cos(angle), up, side * std::sin(angle));
			}
			p.position = event.position + glm::vec3(0.f, 0.05f, 0.f);
			p.velocity = dir * speed;
			renderer.particles.push_back(p);
		}
	}

	if (renderer.jackpotTimer >= 0.f) {
		renderer.jackpotTimer += dt;
	}
	if (renderer.transition.active) {
		renderer.transition.time += dt;
		if (renderer.transition.time >= TransitionIn + TransitionHold + TransitionOut) {
			renderer.transition.active = false;
		}
	}

	for (int k = (int)renderer.packAnims.size() - 1; k >= 0; --k) {
		Renderer::PackAnim& a = renderer.packAnims[k];
		a.age += dt;
		// 연출 중에 손패를 쓰거나 팔면 번호가 밀리므로 그때는 연출을 끝낸다
		bool handChanged = a.handIndex >= (int)game.hand.size() || (int)game.hand.size() != renderer.packHandSize;
		if (a.age - a.delay >= PackAnimTime || game.phase != Phase::Prepare || (a.handIndex >= 0 && handChanged)) {
			renderer.packAnims.erase(renderer.packAnims.begin() + k);
		}
	}

	// 움직이고, 수명이 다하면 지운다 (뒤에서부터 지워야 번호가 안 밀린다)
	for (int k = (int)renderer.particles.size() - 1; k >= 0; --k) {
		Particle& p = renderer.particles[k];
		p.age += dt;
		if (p.age >= p.life) {
			renderer.particles.erase(renderer.particles.begin() + k);
			continue;
		}
		p.velocity.y += renderer.effects[p.effect].gravity * dt;
		p.velocity *= 1.f - 2.f * dt;  // 공기 저항
		p.position += p.velocity * dt;
	}
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

glm::mat4 LightSpaceMatrix()
{
	// 판(±3.5)을 다 덮는 직교 투영. 해처럼 먼 빛이라 평행 광선
	glm::mat4 view = glm::lookAt(LightDir * 10.f, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	glm::mat4 projection = glm::ortho(-5.f, 5.f, -5.f, 5.f, 1.f, 20.f);
	return projection * view;
}

// 1패스: 빛에서 본 말·몹의 깊이만 그림자 맵에 그린다
static void RenderShadowMap(const Renderer& renderer, const Game& game)
{
	// 그림자 맵에 그리는 동안 같은 텍스처를 읽으려고 묶어 두면 안 되므로 1번 유닛에서 뺀다
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindFramebuffer(GL_FRAMEBUFFER, renderer.shadowFbo);
	glViewport(0, 0, ShadowSize, ShadowSize);
	glClear(GL_DEPTH_BUFFER_BIT);
	SetMat4(renderer.program, "uProjection", glm::mat4(1.f));
	SetMat4(renderer.program, "uView", LightSpaceMatrix());
	SetInt(renderer.program, "uShadowOn", 0);
	shadowPass = true;
	RenderUnits(renderer, game);
	RenderMobs(renderer, game);
	shadowPass = false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderGame(const Renderer& renderer, const Game& game, int width, int height)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glUseProgram(renderer.program);
	SetVec3(renderer.program, "uLightDir", LightDir);
	SetFloat(renderer.program, "uFlash", 0.f);
	SetFloat(renderer.program, "uAlpha", 1.f);
	SetVec4(renderer.program, "uUVRect", glm::vec4(0.f, 0.f, 1.f, 1.f));
	SetInt(renderer.program, "uUseTexture", 0);
	SetInt(renderer.program, "uLighting", 1);
	SetInt(renderer.program, "uToon", renderer.toon ? 1 : 0);
	SetFloat(renderer.program, "uDissolve", 0.f);
	SetFloat(renderer.program, "uOutline", 0.f);
	if (renderer.shadows) {
		RenderShadowMap(renderer, game);
	}

	// 2패스: 카메라에서 보며, 각 조각을 빛의 좌표로 옮겨 그림자 맵 깊이와 비교한다
	glClearColor(0.16f, 0.12f, 0.1f, 1.f);  // 작업대 밖은 어두운 나무색
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, width, height);
	float aspect = (float)width / height;
	SetMat4(renderer.program, "uProjection", ProjectionMatrix(renderer, aspect));
	glm::mat4 view = ViewMatrix(renderer.camera);
	SetMat4(renderer.program, "uView", view);
	SetVec3(renderer.program, "uViewPos", glm::vec3(glm::inverse(view)[3]));
	SetMat4(renderer.program, "uLightSpace", LightSpaceMatrix());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderer.shadowMap);
	glActiveTexture(GL_TEXTURE0);
	SetInt(renderer.program, "uShadowMap", 1);
	SetInt(renderer.program, "uShadowOn", renderer.shadows ? 1 : 0);

	RenderBoard(renderer, game);
	RenderUnits(renderer, game);
	RenderMobs(renderer, game);
	RenderPreview(renderer, game);  // 반투명이라 불투명한 것을 다 그린 뒤에
	RenderParticles(renderer);
}

// 텍스처를 입힌 상자 하나. repeat는 텍스처를 면마다 몇 번 반복할지 (GL_REPEAT로 감싼다)
static void DrawTexturedBox(const Renderer& renderer, GLuint texture, glm::vec3 center, glm::vec3 size, glm::vec3 color, glm::vec2 repeat, const Mesh* mesh = nullptr)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	SetInt(renderer.program, "uTexture", 0);
	SetInt(renderer.program, "uUseTexture", texture ? 1 : 0);
	SetVec4(renderer.program, "uUVRect", glm::vec4(0.f, 0.f, repeat.x, repeat.y));
	SetVec3(renderer.program, "uColor", color);
	glm::mat4 model = glm::translate(glm::mat4(1.f), center);
	Draw(renderer.program, mesh ? *mesh : renderer.cube, glm::scale(model, size));
	SetVec4(renderer.program, "uUVRect", glm::vec4(0.f, 0.f, 1.f, 1.f));
	SetInt(renderer.program, "uUseTexture", 0);
}

void RenderBoard(const Renderer& renderer, const Game& game)
{
	const glm::vec3 white(1.f);
	const glm::vec3 hoverColor(1.25f, 1.2f, 0.9f);    // 카드를 놓을 수 있는 칸: 밝게
	const glm::vec3 blockedColor(1.f, 0.45f, 0.4f);   // 놓을 수 없는 칸: 붉게
	const float height = 0.1f;
	const float gap = 0.06f;  // 타일 사이 틈 (경계가 보이도록)
	const float boardHalf = BoardSize * TileSize * 0.5f;

	// 작업대: 넓은 나무판, 텍스처를 여러 번 반복
	DrawTexturedBox(renderer, renderer.tableTexture, glm::vec3(0.f, -0.45f, 0.f), glm::vec3(30.f, 0.1f, 30.f), white, glm::vec2(5.f));

	// 판 바닥(초록 펠트)과 나무 테두리 네 개
	const float rim = 0.22f;
	// 바닥 윗면(y = -0.05)이 칸 윗면(y = 0)보다 낮아야 칸이 바닥에 묻히지 않는다
	DrawTexturedBox(renderer, 0, glm::vec3(0.f, -0.25f, 0.f), glm::vec3(boardHalf * 2.f + 0.1f, 0.4f, boardHalf * 2.f + 0.1f), glm::vec3(0.36f, 0.47f, 0.31f), glm::vec2(1.f));
	float railLength = boardHalf * 2.f + rim * 2.f;
	float railOffset = boardHalf + rim * 0.5f;
	glm::vec3 railX(railLength, 0.5f, rim), railZ(rim, 0.5f, railLength);
	DrawTexturedBox(renderer, renderer.woodTexture, glm::vec3(0.f, -0.15f, railOffset), railX, white, glm::vec2(4.f, 1.f));
	DrawTexturedBox(renderer, renderer.woodTexture, glm::vec3(0.f, -0.15f, -railOffset), railX, white, glm::vec2(4.f, 1.f));
	DrawTexturedBox(renderer, renderer.woodTexture, glm::vec3(railOffset, -0.15f, 0.f), railZ, white, glm::vec2(4.f, 1.f));
	DrawTexturedBox(renderer, renderer.woodTexture, glm::vec3(-railOffset, -0.15f, 0.f), railZ, white, glm::vec2(4.f, 1.f));

	// 칸: 길은 돌, 안쪽은 풀
	for (int i = 0; i < BoardSize; ++i) {
		for (int j = 0; j < BoardSize; ++j) {
			// 윗면이 y = 0에 오도록 두께의 절반만큼 내린다.
			glm::vec3 position = TileCenter(i, j) - glm::vec3(0.f, height * 0.5f, 0.f);
			glm::vec3 size(TileSize - gap, height, TileSize - gap);

			glm::vec3 color = white;
			// 카드를 끄는 동안만 가리키는 칸을 강조한다
			if (renderer.dragCard >= 0 && i == renderer.hoverI && j == renderer.hoverJ) {
				color = CanPlayCard(game, renderer.dragCard, i, j) ? hoverColor : blockedColor;
			}
			GLuint texture = IsPathTile(i, j) ? renderer.stoneTexture : renderer.grassTexture;
			DrawTexturedBox(renderer, texture, position, size, color, glm::vec2(1.f), &renderer.roundTile);
		}
	}
}

// +Z를 보던 모델이 방향 dir(바닥 평면)을 보도록 하는 Y축 회전 각도
static float FacingYaw(glm::vec3 dir)
{
	return std::atan2(dir.x, dir.z);
}

// 모델이 있으면 모델(받침 바닥 = 원점, 높이 1)을, 없으면 큐브를 그린다.
static void DrawFigure(const Renderer& renderer, const Model& model, glm::vec3 base, float yaw, float height, glm::vec3 fallbackColor, bool outline = true)
{
	glm::mat4 m = glm::translate(glm::mat4(1.f), base);
	m = glm::rotate(m, yaw, glm::vec3(0.f, 1.f, 0.f));
	if (model.mesh.vao) {
		glm::mat4 transform = glm::scale(m, glm::vec3(height));

		// 툰 외곽선: 법선 방향으로 부풀린 모델의 뒷면만 어둡게 그린다.
		// 앞면은 잘라 내므로 원래 모델 둘레로 삐져나온 뒷면이 테두리처럼 보인다.
		if (renderer.toon && outline && !shadowPass) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			SetFloat(renderer.program, "uOutline", 0.018f);
			SetInt(renderer.program, "uLighting", 0);
			SetInt(renderer.program, "uUseTexture", 0);
			SetVec3(renderer.program, "uColor", glm::vec3(0.16f, 0.12f, 0.12f));
			Draw(renderer.program, model.mesh, transform);
			SetFloat(renderer.program, "uOutline", 0.f);
			SetInt(renderer.program, "uLighting", 1);
			glDisable(GL_CULL_FACE);
		}

		SetVec3(renderer.program, "uColor", glm::vec3(1.f));
		DrawModel(renderer.program, model, transform);
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
		// 전투의 북을 받은 말은 밝게 깜빡인다
		if (u.drumTimer > 0.f) {
			float pulse = 0.5f + 0.5f * std::sin((float)glfwGetTime() * 10.f);
			SetFloat(renderer.program, "uFlash", 0.15f + 0.15f * pulse);
		}
		DrawFigure(renderer, renderer.unitModels[u.type], center, yaw, height * bump, UnitColor(ud));
		SetFloat(renderer.program, "uFlash", 0.f);

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

	// 죽은 몹: 제자리에서 fBm 노이즈 모양으로 타들어가며 사라진다
	for (const Mob& m : game.dying) {
		int type = m.type < 4 ? m.type : 0;
		glm::vec3 ground = PathPosition(m.pathPos);
		glm::vec3 forward = PathPosition(std::floor(m.pathPos) + 1.f) - PathPosition(std::floor(m.pathPos));
		SetFloat(renderer.program, "uDissolve", std::max(0.01f, m.dieTimer / DieTime));
		DrawFigure(renderer, renderer.mobModels[m.type], ground, FacingYaw(forward), heights[type], colors[type]);
	}
	SetFloat(renderer.program, "uDissolve", 0.f);
}

// 주문 범위: 운석은 붉은 원, 전투의 북은 주변 3×3 칸
static void RenderSpellPreview(const Renderer& renderer, const SpellData& sp, glm::vec3 center)
{
	if (sp.target == SpellTarget::Point) {
		glm::mat4 ring = glm::translate(glm::mat4(1.f), center + glm::vec3(0.f, 0.02f, 0.f));
		ring = glm::scale(ring, glm::vec3(sp.radius, 1.f, sp.radius));
		SetVec3(renderer.program, "uColor", glm::vec3(1.f, 0.45f, 0.2f));
		SetFloat(renderer.program, "uAlpha", 0.3f);
		Draw(renderer.program, renderer.disc, ring);
		SetFloat(renderer.program, "uAlpha", 0.9f);
		Draw(renderer.program, renderer.circle, ring);
	}
	else if (sp.target == SpellTarget::Slot) {
		float size = (sp.radius * 2.f + 1.f) * TileSize;
		glm::mat4 area = glm::translate(glm::mat4(1.f), center + glm::vec3(0.f, 0.03f, 0.f));
		area = glm::scale(area, glm::vec3(size, 0.01f, size));
		SetVec3(renderer.program, "uColor", glm::vec3(1.f, 0.85f, 0.3f));
		SetFloat(renderer.program, "uAlpha", 0.3f);
		Draw(renderer.program, renderer.cube, area);
	}
}

void RenderPreview(const Renderer& renderer, const Game& game)
{
	int card = renderer.dragCard >= 0 && renderer.dragCard < (int)game.hand.size() ? game.hand[renderer.dragCard] : -1;
	if (card < 0 || !CanPlayCard(game, renderer.dragCard, renderer.hoverI, renderer.hoverJ)) {
		return;
	}
	glm::vec3 center = TileCenter(renderer.hoverI, renderer.hoverJ);

	// 반투명: 뒤에 있는 것과 섞어 그리고, 깊이는 쓰지 않아 뒤쪽 물체를 가리지 않게 한다.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	SetInt(renderer.program, "uLighting", 0);

	if (IsSpellCard(card)) {
		RenderSpellPreview(renderer, gameData.spells[card - CardSpellBase], center);
		SetFloat(renderer.program, "uAlpha", 1.f);
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);
		return;
	}
	// 승격: 그 칸 말이 바뀔 다음 등급 말과 사거리를 보여 준다 (CanPlayCard가 말이 있는지 확인했다)
	if (card == CardPromote) {
		for (const Unit& u : game.units) {
			if (u.i == renderer.hoverI && u.j == renderer.hoverJ) {
				card = PromotedType(u.type);
			}
		}
	}
	const UnitData& ud = gameData.units[card];

	// 사거리 원: 옅은 원판 + 진한 테두리. 칸 윗면(y = 0)과 겹쳐 깜빡이지 않게 조금 띄운다
	glm::mat4 ring = glm::translate(glm::mat4(1.f), center + glm::vec3(0.f, 0.02f, 0.f));
	ring = glm::scale(ring, glm::vec3(ud.range, 1.f, ud.range));
	SetVec3(renderer.program, "uColor", glm::vec3(1.f, 0.94f, 0.81f));
	SetFloat(renderer.program, "uAlpha", 0.22f);
	Draw(renderer.program, renderer.disc, ring);
	SetFloat(renderer.program, "uAlpha", 0.9f);
	Draw(renderer.program, renderer.circle, ring);

	// 반투명 말 (같은 칸에 말이 있으면 그 말과 바뀐다는 것을 보여준다)
	const float heights[] = { 0.7f, 0.8f, 0.95f };
	SetInt(renderer.program, "uLighting", 1);
	SetFloat(renderer.program, "uAlpha", 0.5f);
	DrawFigure(renderer, renderer.unitModels[card], center, 0.f, heights[ud.tier], UnitColor(ud), false);

	SetFloat(renderer.program, "uAlpha", 1.f);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void RenderParticles(const Renderer& renderer)
{
	if (renderer.particles.empty()) {
		return;
	}
	// 빌보드: 카메라의 오른쪽·위쪽 축으로 사각형을 세워 항상 화면을 보게 한다
	glm::mat4 view = ViewMatrix(renderer.camera);
	glm::vec3 right(view[0][0], view[1][0], view[2][0]);
	glm::vec3 up(view[0][1], view[1][1], view[2][1]);
	glm::vec3 forward = glm::cross(right, up);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	SetInt(renderer.program, "uLighting", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer.dotTexture);
	SetInt(renderer.program, "uTexture", 0);
	SetInt(renderer.program, "uUseTexture", 1);

	for (const Particle& p : renderer.particles) {
		const EffectData& e = renderer.effects[p.effect];
		float t = p.age / p.life;  // 0 → 1 동안 크기와 색이 시작 값에서 끝 값으로
		float size = glm::mix(e.sizeStart, e.sizeEnd, t);
		glm::vec4 color = glm::mix(e.colorStart, e.colorEnd, t);

		glm::mat4 model(1.f);
		model[0] = glm::vec4(right * size, 0.f);
		model[1] = glm::vec4(-up * size, 0.f);  // 사각형은 y가 아래로 커지므로 뒤집는다
		model[2] = glm::vec4(forward, 0.f);
		model[3] = glm::vec4(p.position, 1.f);
		model = model * glm::translate(glm::mat4(1.f), glm::vec3(-0.5f, -0.5f, 0.f));

		SetVec3(renderer.program, "uColor", glm::vec3(color));
		SetFloat(renderer.program, "uAlpha", color.a);
		Draw(renderer.program, renderer.quad, model);
	}

	SetFloat(renderer.program, "uAlpha", 1.f);
	SetInt(renderer.program, "uUseTexture", 0);
	SetInt(renderer.program, "uLighting", 1);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
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

Rect GoldPackRect(int width, int height)
{
	float s = UiScale(height);
	return Rect{ 24.f * s, height - 380.f * s, 120.f * s, 160.f * s };
}

Rect BetRect(int width, int height)
{
	float s = UiScale(height);
	return Rect{ width - 254.f * s, height - 260.f * s, 200.f * s, 75.f * s };
}

Rect SpeedRect(int width, int height)
{
	float s = UiScale(height);
	return Rect{ width - 254.f * s, height - 180.f * s, 110.f * s, 58.f * s };
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
	case CardPromote: return renderer.promoteCard;
	default:
		if (IsSpellCard(card)) {
			return card - CardSpellBase < (int)renderer.spellCards.size() ? renderer.spellCards[card - CardSpellBase] : 0;
		}
		return card >= 0 && card < (int)renderer.unitCards.size() ? renderer.unitCards[card] : 0;
	}
}

static bool IsPackAnimating(const Renderer& renderer, int handIndex)
{
	for (const Renderer::PackAnim& a : renderer.packAnims) {
		if (a.handIndex == handIndex) {
			return true;
		}
	}
	return false;
}

static Rect RelicSlotRect(const Game& game, int relic, float s)
{
	// 유물 줄에서 그 유물 아이콘의 자리 (RenderUi의 유물 줄과 같은 배치)
	float x = 24.f * s;
	for (int r = 0; r < relic; ++r) {
		if (std::count(game.relics.begin(), game.relics.end(), r) > 0) {
			x += 56.f * s;
		}
	}
	return Rect{ x, 72.f * s, 48.f * s, 48.f * s };
}

static void DrawScreenDisc(const Renderer& renderer, glm::vec2 center, float radius, glm::vec3 color, float alpha);

// 카드팩 개봉: 팩 위에서 커지며 y축으로 반 바퀴 돌아 앞면이 보이고, 손패(유물은 유물 줄) 자리로 날아간다
static void RenderPackAnims(const Renderer& renderer, const Game& game, int width, int height)
{
	float s = UiScale(height);
	int count = (int)game.hand.size();
	for (const Renderer::PackAnim& a : renderer.packAnims) {
		Rect pack = a.source == 1 ? GoldPackRect(width, height) : a.source == 2 ? BetRect(width, height) : PackRect(width, height);
		float t = (a.age - a.delay) / PackAnimTime;
		if (t < 0.f) {
			continue;
		}
		bool relic = a.handIndex < 0;
		if (!relic && a.handIndex >= count) {
			continue;
		}
		Rect to = relic ? RelicSlotRect(game, a.relic, s) : HandCardRect(a.handIndex, count, width, height);
		Rect from = Rect{ pack.x + pack.w * 0.5f - to.w * 0.5f, pack.y - to.h * 0.6f, to.w, to.h };
		if (relic) {
			from = Rect{ pack.x + pack.w * 0.5f - 60.f * s, pack.y - 150.f * s, 120.f * s, 120.f * s };
		}

		// 앞 절반: 제자리에서 뒤집기, 뒤 절반: 손패로 이동
		float flip = Clamp01(t * 2.f);
		float move = EaseOutCubic(t * 2.f - 1.f);
		Rect r;
		r.x = glm::mix(from.x, to.x, move);
		r.y = glm::mix(from.y, to.y, move);
		r.w = glm::mix(from.w, to.w, move);
		r.h = glm::mix(from.h, to.h, move);

		// y축 회전을 화면에서는 가로 폭 |cos|로 보인다. 90도를 넘으면 앞면
		float squash = std::abs(std::cos(flip * 3.14159265f));
		bool front = flip >= 0.5f;
		float cx = r.x + r.w * 0.5f;
		r.w *= squash;
		r.x = cx - r.w * 0.5f;
		int card = relic ? -1 : game.hand[a.handIndex];
		GLuint texture = !front ? renderer.cardBack : relic ? renderer.relicIcons[a.relic] : CardTexture(renderer, card);

		// 합성 카드·영웅 말·승격 카드는 앞면이 보이면 뒤에서 금빛이 퍼진다
		bool special = a.merged || card == CardPromote || (IsUnitCard(card) && gameData.units[card].tier == 2);
		if (special && front) {
			float glow = Clamp01((flip - 0.5f) * 2.f) * (1.f - Clamp01(t * 2.f - 1.f) * 0.6f);
			glm::vec2 c(r.x + r.w * 0.5f, r.y + r.h * 0.5f);
			DrawScreenDisc(renderer, c, r.h * (0.6f + 0.25f * glow), glm::vec3(1.f, 0.78f, 0.3f), 0.55f * glow);
		}
		DrawRect(renderer, texture, r, glm::vec3(1.f), 1.f);
	}
}

// 대박: 팩에서 금빛 원 두 개가 퍼지며 옅어지고 화면이 잠깐 금빛으로 번쩍인다
static void RenderJackpot(const Renderer& renderer, int width, int height)
{
	float t = renderer.jackpotTimer;
	if (t < 0.f || t > 1.2f) {
		return;
	}
	Rect pack = renderer.jackpotSource == 1 ? GoldPackRect(width, height) : PackRect(width, height);
	glm::vec2 c(pack.x + pack.w * 0.5f, pack.y + pack.h * 0.5f);
	float flash = std::max(0.f, 1.f - t / 0.35f);
	DrawRect(renderer, 0, Rect{ 0.f, 0.f, (float)width, (float)height }, glm::vec3(1.f, 0.85f, 0.4f), 0.35f * flash);
	for (int k = 0; k < 2; ++k) {
		float local = Clamp01((t - 0.15f * k) / 0.9f);
		float radius = EaseOutCubic(local) * height * 0.9f;
		DrawScreenDisc(renderer, c, radius, glm::vec3(1.f, 0.75f, 0.25f), 0.45f * (1.f - local));
	}
}

void StartTransition(Renderer& renderer, glm::vec2 origin, int handIndex, int i, int j)
{
	Renderer::Transition& t = renderer.transition;
	t = Renderer::Transition{};
	t.active = true;
	t.origin = origin;
	t.handIndex = handIndex;
	t.i = i;
	t.j = j;
}

bool TransitionCovered(const Renderer& renderer)
{
	const Renderer::Transition& t = renderer.transition;
	return t.active && !t.applied && t.time >= TransitionIn;
}

// 화면 좌표의 원판: XZ 평면 원판을 x축으로 90도 세워 화면(XY)에 눕힌다
static void DrawScreenDisc(const Renderer& renderer, glm::vec2 center, float radius, glm::vec3 color, float alpha)
{
	glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(center, 0.f));
	model = glm::rotate(model, glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
	model = glm::scale(model, glm::vec3(radius, 1.f, radius));
	SetInt(renderer.program, "uUseTexture", 0);
	SetVec3(renderer.program, "uColor", color);
	SetFloat(renderer.program, "uAlpha", alpha);
	Draw(renderer.program, renderer.disc, model);
}

// 화면 전환: 금색 테두리를 두른 크림색 원이 놓은 곳에서 퍼져 덮고, 다 덮인 뒤 커지며 투명해진다
static void RenderTransition(const Renderer& renderer, int width, int height)
{
	const Renderer::Transition& t = renderer.transition;
	if (!t.active) {
		return;
	}
	// 어느 점에서 퍼져도 화면 네 모서리를 다 덮는 반지름
	glm::vec2 o = t.origin;
	float cover = 0.f;
	const glm::vec2 corners[4] = { { 0.f, 0.f }, { (float)width, 0.f }, { 0.f, (float)height }, { (float)width, (float)height } };
	for (const glm::vec2& c : corners) {
		cover = std::max(cover, glm::distance(o, c));
	}

	float radius, alpha;
	if (t.time < TransitionIn) {
		float k = EaseInOutQuad(t.time / TransitionIn);
		radius = cover * 1.08f * k;
		alpha = 1.f;
	}
	else {
		float k = Clamp01((t.time - TransitionIn - TransitionHold) / TransitionOut);
		radius = cover * (1.08f + 0.3f * k);
		alpha = 1.f - EaseOutCubic(k);
	}
	const glm::vec3 cream(0.98f, 0.96f, 0.92f), amber(0.91f, 0.64f, 0.24f);
	DrawScreenDisc(renderer, o, radius, amber, alpha);
	DrawScreenDisc(renderer, o, std::max(0.f, radius - 18.f * UiScale(height)), cream, alpha);
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
			float limit = RoundTimeLimit(game);
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

	// 유물: 위 막대 아래 왼쪽에 종류마다 아이콘 하나, 2개 이상이면 개수
	if (playing) {
		float x = 24.f * s;
		for (int r = 0; r < (int)gameData.relics.size(); ++r) {
			int owned = (int)std::count(game.relics.begin(), game.relics.end(), r);
			if (owned == 0) {
				continue;
			}
			DrawRect(renderer, renderer.relicIcons[r], Rect{ x, 72.f * s, 48.f * s, 48.f * s }, white, 1.f);
			if (owned > 1) {
				snprintf(text, sizeof(text), "x%d", owned);
				DrawNumber(renderer, text, x + 48.f * s, 96.f * s, 26.f * s, 1.f, white);
			}
			x += 56.f * s;
		}
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
		// 골드 부스터: 승격 카드와 희귀 이상 말이 나오는 비싼 팩
		Rect gold = GoldPackRect(width, height);
		bool canBuyGold = CanBuyPack(game, true);
		bool overGold = canBuyGold && renderer.dragCard < 0 && Contains(gold, renderer.mouse);
		DrawRect(renderer, renderer.goldPack, gold, canBuyGold ? (overGold ? glm::vec3(1.1f) : white) : dim, 1.f);
		DrawRect(renderer, renderer.chipIcon, Rect{ gold.x + gold.w * 0.5f - 40.f * s, gold.y + gold.h - 6.f * s, 28.f * s, 28.f * s }, white, 1.f);
		snprintf(text, sizeof(text), "%d", gameData.goldPrice);
		DrawNumber(renderer, text, gold.x + gold.w * 0.5f - 8.f * s, gold.y + gold.h - 8.f * s, 32.f * s, 0.f, white);

		Rect pack = PackRect(width, height);
		bool canBuy = CanBuyPack(game, false);
		bool sellTarget = renderer.dragCard >= 0 && renderer.dragCard < (int)game.hand.size() && game.hand[renderer.dragCard] >= 0;  // 카드를 끌어 오면 판다
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

		// 배속 버튼: 1배는 흰 글자, 빠를 때는 금색
		Rect speed = SpeedRect(width, height);
		bool overSpeed = Contains(speed, renderer.mouse);
		DrawRect(renderer, 0, speed, overSpeed ? glm::vec3(0.25f, 0.29f, 0.36f) : glm::vec3(0.17f, 0.2f, 0.25f), 0.9f);
		snprintf(text, sizeof(text), "x%d", renderer.speed);
		glm::vec3 speedColor = renderer.speed > 1 ? glm::vec3(1.f, 0.78f, 0.35f) : white;
		DrawNumber(renderer, text, speed.x + speed.w * 0.5f, speed.y + 7.f * s, 44.f * s, 0.5f, speedColor);

		// 최대 베팅: 정비 중에 켜고 끈다. 전투 중에는 베팅한 라운드일 때만 켜진 채로 보인다
		bool betShown = game.phase == Phase::Prepare ? game.bet : game.betRound;
		bool betEnabled = game.phase == Phase::Prepare;
		DrawRect(renderer, betShown ? renderer.betOn : renderer.betOff, BetRect(width, height), betEnabled || betShown ? white : dim, 1.f);
	}

	// 손패: 마우스가 올라간 카드는 조금 올리고, 끄는 카드는 마우스를 따라간다
	int count = (int)game.hand.size();
	int hovered = renderer.dragCard < 0 ? HandCardAt(game, renderer.mouse, width, height) : -1;
	for (int k = 0; k < count; ++k) {
		bool usedCard = renderer.transition.active && !renderer.transition.applied && k == renderer.transition.handIndex;
		if (k == renderer.dragCard || IsPackAnimating(renderer, k) || usedCard) {
			continue;
		}
		Rect r = HandCardRect(k, count, width, height);
		if (k == hovered) {
			r.y -= 24.f * s;
		}
		DrawRect(renderer, CardTexture(renderer, game.hand[k]), r, white, 1.f);
	}
	if (renderer.dragCard >= 0 && renderer.dragCard < count) {
		// 판 위를 가리키면 카드를 작게 줄여 커서 오른쪽 위로 비켜서, 놓을 칸과 미리보기 말이 보이게 한다
		bool overBoard = renderer.hoverI >= 0;
		Rect r = HandCardRect(0, 1, width, height);
		r.w *= overBoard ? 0.45f : 0.8f;
		r.h *= overBoard ? 0.45f : 0.8f;
		r.x = renderer.mouse.x + (overBoard ? 18.f * s : -r.w * 0.5f);
		r.y = renderer.mouse.y - (overBoard ? r.h + 12.f * s : r.h * 0.5f);
		DrawRect(renderer, CardTexture(renderer, game.hand[renderer.dragCard]), r, white, overBoard ? 0.85f : 1.f);
	}

	RenderPackAnims(renderer, game, width, height);
	// 유물 아이콘에 마우스를 올리면 아래에 설명 리본 (다른 UI보다 위에 그린다)
	if (playing && renderer.dragCard < 0) {
		for (int r = 0; r < (int)gameData.relics.size(); ++r) {
			if (std::count(game.relics.begin(), game.relics.end(), r) == 0) {
				continue;
			}
			Rect slot = RelicSlotRect(game, r, s);
			if (Contains(slot, renderer.mouse) && r < (int)renderer.relicTips.size()) {
				float tw = 340.f * s, th = tw * 130.f / 560.f;
				float tx = std::min(slot.x, width - tw - 8.f * s);
				DrawRect(renderer, renderer.relicTips[r], Rect{ tx, slot.y + slot.h + 6.f * s, tw, th }, white, 1.f);
			}
		}
	}

	RenderJackpot(renderer, width, height);
	RenderTransition(renderer, width, height);

	SetFloat(renderer.program, "uAlpha", 1.f);
	SetInt(renderer.program, "uUseTexture", 0);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}
