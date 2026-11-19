#include "Game.h"
#include "Renderer.h"
#include "Util/Picking.h"
#include <algorithm>
#include <algorithm>

Game game;
Renderer renderer;

void CharCallback(GLFWwindow* window, unsigned int c)
{
	Camera& camera = renderer.camera;
	switch (c) {
	case '+': camera.distance = glm::clamp(camera.distance - 0.5f, 1.f, 20.f); break;
	case '-': camera.distance = glm::clamp(camera.distance + 0.5f, 1.f, 20.f); break;
	case 'o': camera.yawDir = camera.yawDir != 1.f ? 1.f : 0.f; break;
	case 'p': renderer.isOrtho = !renderer.isOrtho; break;
	case 's': camera = Camera{}; break;
	case 'q': case 'Q': glfwSetWindowShouldClose(window, true); break;
	}
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE) {
		return;
	}
	// 임시 키: 카드 끌어 놓기·UI 버튼이 생기기 전까지 사용
	switch (key) {
	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
	case GLFW_KEY_ENTER:
		if (game.phase == Phase::Title || game.phase == Phase::Result) {
			StartGame(game);
			renderer.selectedHand = 0;
		}
		break;
	case GLFW_KEY_T:
		if (game.phase == Phase::Result) {
			GameInit(game);
		}
		break;
	case GLFW_KEY_SPACE:
		EndTurn(game);
		break;
	case GLFW_KEY_B:
		BuyPack(game);
		break;
	}
	if (key >= GLFW_KEY_1 && key <= GLFW_KEY_6) {
		renderer.selectedHand = key - GLFW_KEY_1;
	}
}

// 왼쪽 클릭: 고른 손패 카드를 가리키는 칸에 놓는다.
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS) {
		return;
	}
	if (PlaceUnit(game, renderer.selectedHand, renderer.hoverI, renderer.hoverJ)) {
		// 쓴 카드 자리로 다음 카드가 당겨지므로, 끝을 넘으면 마지막 카드를 고른다.
		if (renderer.selectedHand >= (int)game.hand.size()) {
			renderer.selectedHand = std::max(0, (int)game.hand.size() - 1);
		}
	}
}

// 임시 UI: 화면 글자가 생기기 전까지 창 제목에 상태를 보여준다.
void UpdateWindowTitle(GLFWwindow* window)
{
	const char* classNames[] = { "마법사", "궁수", "전사" };
	char text[512];
	int n = 0;

	switch (game.phase) {
	case Phase::Title:
		n = snprintf(text, sizeof(text), "카드 디펜스 | Enter: 시작");
		break;
	case Phase::Prepare:
	case Phase::Battle: {
		if (game.phase == Phase::Prepare) {
			n = snprintf(text, sizeof(text), "%d라운드 정비 | 칩 %d | Space 전투, B 팩(%d칩), 1~6 카드 고르고 클릭 | 손패",
				game.round, game.chips, gameData.boosterPrice);
		}
		else {
			float left = std::max(0.f, gameData.roundTimeLimit - game.timer);
			int remain = (int)game.spawnQueue.size() - game.spawnedCount + (int)game.mobs.size();
			n = snprintf(text, sizeof(text), "%d라운드 전투 | 남은 시간 %.0f초 | 남은 몹 %d | 칩 %d | 손패",
				game.round, left, remain, game.chips);
		}
		for (int k = 0; k < (int)game.hand.size() && n < (int)sizeof(text); ++k) {
			const UnitData& ud = gameData.units[game.hand[k]];
			const char* format = k == renderer.selectedHand ? " [%d:%s%d]" : " %d:%s%d";
			n += snprintf(text + n, sizeof(text) - n, format, k + 1, classNames[(int)ud.unitClass], ud.tier + 1);
		}
		break;
	}
	case Phase::Result:
		n = snprintf(text, sizeof(text), "%s | %d라운드, 처치 %d | Enter: 다시 하기, T: 타이틀",
			game.won ? "승리!" : "패배", game.round, game.kills);
		break;
	}
	glfwSetWindowTitle(window, text);
}

// 마우스가 가리키는 칸을 renderer.hoverI/J에 넣는다.
// 마우스를 안 움직여도 카메라가 돌면 가리키는 칸이 바뀌므로 콜백이 아니라 매 프레임 부른다.
void UpdateHover(GLFWwindow* window)
{
	renderer.hoverI = renderer.hoverJ = -1;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	// 마우스 좌표는 창 좌표 단위라 프레임버퍼가 아닌 창 크기로 나눈다 (화면 배율이 100%가 아니면 둘이 다르다).
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (width <= 0 || height <= 0) {
		return;
	}

	float aspect = (float)width / height;
	Ray ray = ScreenToRay(glm::vec2((float)x, (float)y), width, height,
		ViewMatrix(renderer.camera), ProjectionMatrix(renderer, aspect));

	glm::vec3 hit;
	int i, j;
	if (IntersectHorizontalPlane(ray, 0.f, hit) && WorldToTile(hit, i, j)) {
		renderer.hoverI = i;
		renderer.hoverJ = j;
	}
}

int main()
{
	if (!glfwInit()) {
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Card Defense", nullptr, nullptr);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		return -1;
	}

	if (!LoadGameData()) {
		glfwTerminate();
		return -1;
	}
	GameInit(game);
	RendererInit(renderer);

	glfwSetCharCallback(window, CharCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	float last = (float)glfwGetTime();
	float titleTimer = 0.f;
	while (!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
		float dt = now - last;
		last = now;
		if (dt > 0.1f) {
			dt = 0.1f;  // 창 드래그·중단점 뒤 큰 dt로 몹이 순간이동하지 않게
		}

		GameUpdate(game, dt);
		RendererUpdate(renderer, dt);
		UpdateHover(window);

		titleTimer -= dt;
		if (titleTimer <= 0.f) {
			UpdateWindowTitle(window);
			titleTimer = 0.2f;  // 창 제목은 자주 바꾸면 느려서 0.2초마다
		}

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		if (width > 0 && height > 0) {
			RenderGame(renderer, game, width, height);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
