// 퍼즈 테스트: 숨긴 창에서 무작위 조작(카드·팩·턴 종료·F키·끌기 상태·카메라)을 수십만 프레임 넣고 크래시와 상태 규칙 위반을 찾는다.
// Debug 런타임(/MDd)으로 빌드해야 벡터 범위 검사가 켜진다. 실행: Debug 빌드 폴더에서 fuzz_test.exe [프레임 수], 시드는 환경 변수 FUZZ_SEED
#include "Renderer.h"
#include <cstdio>
#include <cmath>
#include <random>
#include <set>
#include <crtdbg.h>
char lastAction[128];
Game game; Renderer renderer; GLFWwindow* win; int W = 1280, H = 720;
std::mt19937 r(getenv("FUZZ_SEED") ? atoi(getenv("FUZZ_SEED")) : 12345);
int R(int a, int b) { return std::uniform_int_distribution<int>(a, b)(r); }
int violations = 0;
void Bad(const char* what, long f) { if (violations++ < 20) printf("VIOLATION frame %ld: %s\n", f, what); }
int main(int argc, char** argv) {
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE); _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE); _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
	long frames = argc > 1 ? atol(argv[1]) : 200000;
	glfwInit(); glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	win = glfwCreateWindow(W, H, "f", nullptr, nullptr); glfwMakeContextCurrent(win); glewExperimental = GL_TRUE; glewInit();
	if (!LoadGameData()) return 1; GameInit(game); RendererInit(renderer);
	long phaseCount[4] = {}; int wins = 0, results = 0; Phase last = game.phase;
	for (long f = 0; f < frames; ++f) {
		int a = R(0, 99);
		int hs = (int)game.hand.size();
		snprintf(lastAction, sizeof(lastAction), "f%ld a%d phase%d hand%d drag%d hover%d,%d anims%zu", f, a, (int)game.phase, hs, renderer.dragCard, renderer.hoverI, renderer.hoverJ, renderer.packAnims.size());
		if (getenv("FUZZ_LOG")) fprintf(stderr, "%s\n", lastAction);
		if (a < 8) BuyPack(game, R(0, 2) == 0);
		else if (a < 18) PlayCard(game, R(-1, hs), R(-1, BoardSize), R(-1, BoardSize));
		else if (a < 21) SellCard(game, R(-1, hs));
		else if (a < 23) EndTurn(game);
		else if (a == 23 && R(0, 9) == 0) DemoNextRound(game);
		else if (a == 24 && R(0, 9) == 0) DemoAddChips(game);
		else if (a == 25 && R(0, 30) == 0) DemoBossRound(game);
		else if (a == 26) DemoReplayEffects(game);
		else if (a == 27 && R(0, 50) == 0) renderer.toon = !renderer.toon;
		else if (a == 28 && R(0, 50) == 0) renderer.shadows = !renderer.shadows;
		else if (a == 29 && R(0, 2000) == 0) GameInit(game);
		else if (a == 30 && game.phase == Phase::Title && R(0, 20) == 0) StartGame(game);
		// 드래그·마우스 상태는 실제 입력처럼 아무 값이나
		if (R(0, 9) == 0) renderer.dragCard = R(-1, hs + 1);
		if (R(0, 9) == 0) { renderer.hoverI = R(-1, BoardSize - 1); renderer.hoverJ = renderer.hoverI < 0 ? -1 : R(0, BoardSize - 1); }
		if (R(0, 9) == 0) renderer.mouse = glm::vec2((float)R(0, W), (float)R(0, H));
		if (R(0, 200) == 0) { renderer.camera.yaw = (float)R(-45, 45); renderer.camera.distance = (float)R(6, 14); }
		if (game.quit) { game.quit = false; GameInit(game); }

		// 전투는 빨리 진행 (dt를 크게)
		float dt = game.phase == Phase::Battle ? 1.f / 20.f : 1.f / 60.f;
		GameUpdate(game, dt); RendererUpdate(renderer, game, dt); game.events.clear();
		if (f % 7 == 0) { RenderGame(renderer, game, W, H); RenderUi(renderer, game, W, H); glfwSwapBuffers(win); glfwPollEvents(); }

		// 상태 규칙
		if ((int)game.hand.size() > MaxHand) Bad("hand > MaxHand", f);
		if (game.chips < 0) Bad("chips < 0", f);
		std::set<int> tiles;
		for (auto& u : game.units) { if (IsPathTile(u.i, u.j)) Bad("unit on path", f); if (!tiles.insert(u.i * 10 + u.j).second) Bad("two units one tile", f);
			if (u.type < 0 || u.type >= (int)gameData.units.size()) Bad("bad unit type", f); }
		for (auto& m : game.mobs) if (std::isnan(m.hp) || m.hp <= 0.f) Bad("dead/NaN mob alive after update", f);
		for (int c : game.hand) if (!(IsUnitCard(c) && c < (int)gameData.units.size()) && !(IsSpellCard(c) && c - CardSpellBase < (int)gameData.spells.size()) && !(c >= CardTitle && c <= CardStart) && c != CardPromote) Bad("bad card", f);
		if (game.round < 1 || game.round > MaxRound) Bad("round out of range", f);
		if (game.phase == Phase::Battle && game.timer > RoundTimeLimit(game) + 0.2f) Bad("battle past time limit", f);
		if (renderer.particles.size() > 20000) Bad("particle leak", f);
		GLenum e = glGetError(); if (e != GL_NO_ERROR) Bad("GL error", f);
		phaseCount[(int)game.phase]++;
		if (game.phase == Phase::Result && last != Phase::Result) { results++; wins += game.won; }
		last = game.phase;
	}
	printf("frames %ld, title %ld prepare %ld battle %ld result %ld, results %d (wins %d), violations %d, particles %zu\n",
		frames, phaseCount[0], phaseCount[1], phaseCount[2], phaseCount[3], results, wins, violations, renderer.particles.size());
	glfwTerminate();
}


