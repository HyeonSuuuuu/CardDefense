// 자동 플레이 캡처: 숨긴 창에서 봇(bot.h)이 한 판을 하며 보고서용 화면을 BMP로 저장한다. 봇이 지면 승리 화면 대신 패배 화면이 저장된다.
// 빌드: README의 '검증 도구' 명령. 실행: Release 빌드 폴더에서 capture_playthrough.exe <저장할 폴더>
#include "Renderer.h"
#include "Util/Capture.h"
#include "bot.h"
#include <cstdio>
#include <string>

Game game;
Renderer renderer;
GLFWwindow* window;
int width = 1280, height = 720;
int glErrors = 0;
std::string outDir;

static void Frame(const char* shot = nullptr)
{
	const float dt = 1.f / 60.f;
	GameUpdate(game, dt);
	RendererUpdate(renderer, game, dt);
	game.events.clear();
	BeginScene(renderer, width, height);
	RenderGame(renderer, game, width, height);
	EndScene(renderer, width, height);
	RenderUi(renderer, game, width, height);
	for (GLenum e; (e = glGetError()) != GL_NO_ERROR;) {
		++glErrors;
	}
	if (shot) {
		SaveScreenshotBmp((outDir + "/" + shot).c_str(), width, height);
		printf("%s: round %d, units %d, mobs %d, relics %d\n", shot, game.round, (int)game.units.size(), (int)game.mobs.size(), (int)game.relics.size());
	}
	glfwSwapBuffers(window);
	glfwPollEvents();
}

static void Frames(int count, const char* shot = nullptr)
{
	for (int k = 0; k < count; ++k) {
		Frame(k == count - 1 ? shot : nullptr);
	}
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("사용: capture_playthrough.exe <저장할 폴더>\n");
		return 1;
	}
	outDir = argv[1];
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	window = glfwCreateWindow(width, height, "capture", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	if (!LoadGameData()) {
		return 1;
	}
	GameInit(game);
	RendererInit(renderer);
	glfwGetFramebufferSize(window, &width, &height);

	Frames(30, "01_title.bmp");
	StartGame(game);
	renderer.dragCard = 0;
	renderer.hoverI = 1;
	renderer.hoverJ = 2;
	renderer.mouse = glm::vec2(640.f, 360.f);
	Frames(20, "02_drag_preview.bmp");
	renderer.dragCard = -1;
	renderer.hoverI = -1;

	// 첫 정비: 칩을 조금 주고 팩을 사서 개봉 연출을 찍는다
	game.chips = 10;
	BuyPack(game);
	Frames(12, "03_pack_open.bmp");
	BotPrepare(game, 1);
	Frames(8, "04_prepare_summon.bmp");

	bool shotToon = false, shotDissolve = false, shotBoss = false;
	while (game.phase != Phase::Result) {
		BotPrepare(game);
		int round = game.round;
		EndTurn(game);
		while (game.phase == Phase::Battle) {
			BotBattle(game);
			if (round == 1 && game.timer >= 6.f && game.timer < 6.f + 1.f / 60.f) {
				Frame("05_battle_r1.bmp");
				continue;
			}
			if (round == 5 && !shotToon && game.timer >= 9.f) {
				shotToon = true;
				renderer.toon = true;
				Frame("06_battle_r5_toon.bmp");
				renderer.toon = false;
				Frame("07_battle_r5_phong.bmp");
				renderer.toon = true;
				continue;
			}
			if (round >= 6 && !shotDissolve && game.dying.size() >= 2 && game.dying[0].dieTimer > 0.3f) {
				shotDissolve = true;
				renderer.camera.distance = 7.f;
				Frame("08_dissolve_zoom.bmp");
				renderer.camera = Camera{};
				continue;
			}
			if (round == 10 && !shotBoss && game.timer >= 12.f) {
				shotBoss = true;
				Frame("09_boss_r10.bmp");
				continue;
			}
			Frame();
		}
		printf("round %d -> %s\n", round, game.phase == Phase::Result ? (game.won ? "win" : "lose") : "next");
	}
	Frames(30, game.won ? "10_result_victory.bmp" : "10_result_defeat.bmp");
	printf("GL errors: %d\n", glErrors);
	glfwTerminate();
	return 0;
}
