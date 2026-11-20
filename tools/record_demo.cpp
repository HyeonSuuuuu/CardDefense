// 데모 영상 녹화: 숨긴 창에서 발표 데모 순서대로 조작하며 매 프레임 RGB를 stdout으로 내보낸다 (ffmpeg로 mp4를 만든다).
// 실행 예 (Release 빌드 폴더): record_demo.exe | ffmpeg -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 60 -i - -vf vflip -c:v libx264 -pix_fmt yuv420p demo.mp4
#include "Renderer.h"
#include "Util/Picking.h"
#include "bot.h"
#include <cstdio>
#include <fcntl.h>
#include <io.h>
#include <vector>

Game game;
Renderer renderer;
GLFWwindow* window;
const int Width = 1280, Height = 720;
std::vector<unsigned char> pixels(Width * Height * 3);
long frameCount = 0;

static void Frame()
{
	const float dt = 1.f / 60.f;
	GameUpdate(game, dt);
	RendererUpdate(renderer, game, dt);
	game.events.clear();

	// main.cpp의 UpdateHover와 같은 피킹: 마우스가 가리키는 칸
	renderer.hoverI = renderer.hoverJ = -1;
	Ray ray = ScreenToRay(renderer.mouse, Width, Height, ViewMatrix(renderer.camera), ProjectionMatrix(renderer, (float)Width / Height));
	glm::vec3 hit;
	int i, j;
	if (IntersectHorizontalPlane(ray, 0.f, hit) && WorldToTile(hit, i, j)) {
		renderer.hoverI = i;
		renderer.hoverJ = j;
	}

	RenderGame(renderer, game, Width, Height);
	RenderUi(renderer, game, Width, Height);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
	fwrite(pixels.data(), 1, pixels.size(), stdout);
	++frameCount;
	glfwSwapBuffers(window);
	glfwPollEvents();
}

static void Wait(float seconds)
{
	for (int k = 0; k < (int)(seconds * 60.f); ++k) {
		Frame();
	}
}

static glm::vec2 TileOnScreen(int i, int j)
{
	glm::vec4 clip = ProjectionMatrix(renderer, (float)Width / Height) * ViewMatrix(renderer.camera) * glm::vec4(TileCenter(i, j), 1.f);
	glm::vec3 ndc = glm::vec3(clip) / clip.w;
	return glm::vec2((ndc.x + 1.f) * 0.5f * Width, (1.f - ndc.y) * 0.5f * Height);
}

static glm::vec2 Center(const Rect& r)
{
	return glm::vec2(r.x + r.w * 0.5f, r.y + r.h * 0.5f);
}

// 마우스를 부드럽게 옮긴다 (EaseInOut)
static void MoveMouse(glm::vec2 to, float seconds)
{
	glm::vec2 from = renderer.mouse;
	int n = (int)(seconds * 60.f);
	for (int k = 1; k <= n; ++k) {
		float t = (float)k / n;
		t = t < 0.5f ? 2.f * t * t : 1.f - 2.f * (1.f - t) * (1.f - t);
		renderer.mouse = glm::mix(from, to, t);
		Frame();
	}
}

// 손패 카드를 끌어 칸에 놓는다
static bool DragCard(int handIndex, int i, int j)
{
	if (handIndex < 0 || handIndex >= (int)game.hand.size()) {
		return false;
	}
	MoveMouse(Center(HandCardRect(handIndex, (int)game.hand.size(), Width, Height)), 0.35f);
	renderer.dragCard = handIndex;
	MoveMouse(TileOnScreen(i, j), 0.8f);
	Wait(0.25f);
	bool ok = PlayCard(game, handIndex, i, j);
	renderer.dragCard = -1;
	return ok;
}

static void ClickPack()
{
	MoveMouse(Center(PackRect(Width, Height)), 0.4f);
	BuyPack(game);
	Wait(0.8f);
}

static void OrbitCamera(float yawTo, float distanceTo, float seconds)
{
	float yaw0 = renderer.camera.yaw, d0 = renderer.camera.distance;
	int n = (int)(seconds * 60.f);
	for (int k = 1; k <= n; ++k) {
		float t = (float)k / n;
		t = t < 0.5f ? 2.f * t * t : 1.f - 2.f * (1.f - t) * (1.f - t);
		renderer.camera.yaw = glm::mix(yaw0, yawTo, t);
		renderer.camera.distance = glm::mix(d0, distanceTo, t);
		Frame();
	}
}

int main()
{
	_setmode(_fileno(stdout), _O_BINARY);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	window = glfwCreateWindow(Width, Height, "record", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	if (!LoadGameData()) {
		return 1;
	}
	GameInit(game);
	RendererInit(renderer);
	renderer.mouse = glm::vec2(Width * 0.5f, Height * 0.6f);

	// 1. 타이틀 → Start 카드
	Wait(1.5f);
	DragCard(0, 2, 2);
	Wait(0.6f);

	// 2. 스타터 말 세 장 배치
	const int spots[3][2] = { { 1, 1 }, { 4, 1 }, { 1, 4 } };
	for (auto& s : spots) {
		DragCard(0, s[0], s[1]);
		Wait(0.3f);
	}

	// 3. F2로 칩을 받고 카드팩 세 번
	DemoAddChips(game);
	Wait(0.4f);
	for (int k = 0; k < 3; ++k) {
		ClickPack();
	}
	// 나온 말 카드는 빈 칸에, 전투의 북은 말 옆에
	const int more[4][2] = { { 4, 4 }, { 2, 1 }, { 1, 2 }, { 3, 4 } };
	int m = 0;
	for (int h = (int)game.hand.size() - 1; h >= 0 && m < 4; --h) {
		if (IsUnitCard(game.hand[h])) {
			DragCard(h, more[m][0], more[m][1]);
			++m;
		}
	}
	int drum = BotFindCard(game, "battle_drum");
	if (drum >= 0) {
		DragCard(drum, 2, 2);
	}

	// 4. 턴 종료 → 전투, 운석이 있으면 앞선 몹에
	MoveMouse(Center(EndTurnRect(Width, Height)), 0.4f);
	EndTurn(game);
	Wait(5.f);
	int meteor = BotFindCard(game, "meteor_dice");
	if (meteor >= 0 && !game.mobs.empty()) {
		int i, j;
		WorldToTile(PathPosition(game.mobs[0].pathPos), i, j);
		DragCard(meteor, i, j);
	}
	Wait(3.f);

	// 5. 툰 ↔ 퐁, 그림자, 카메라를 돌려 가까이
	renderer.toon = false;
	Wait(1.5f);
	renderer.toon = true;
	Wait(0.8f);
	OrbitCamera(35.f, 7.f, 2.f);
	renderer.shadows = false;
	Wait(1.2f);
	renderer.shadows = true;
	Wait(1.2f);
	OrbitCamera(-30.f, 8.f, 2.f);
	OrbitCamera(0.f, 11.f, 1.5f);
	while (game.phase == Phase::Battle) {
		Frame();
	}

	// 6. 보스 라운드: 칩으로 판을 채우고(봇) 전투, 결과 화면
	DemoBossRound(game);
	for (int k = 0; k < 8; ++k) {
		DemoAddChips(game);
		BotPrepare(game);
	}
	Wait(1.5f);
	MoveMouse(Center(EndTurnRect(Width, Height)), 0.4f);
	EndTurn(game);
	while (game.phase == Phase::Battle) {
		BotBattle(game);
		Frame();
	}
	Wait(3.f);
	fprintf(stderr, "frames %ld, result %s\n", frameCount, game.won ? "win" : "lose");
	glfwTerminate();
	return game.won ? 0 : 2;
}
