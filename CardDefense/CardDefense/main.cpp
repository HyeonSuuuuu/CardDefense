#include "Game.h"
#include "Renderer.h"
#include "Util/Picking.h"
#include <algorithm>

Game game;
Renderer renderer;

// 카메라가 움직일 수 있는 범위. 판이 화면을 벗어나거나 옆·아래에서 보이지 않게 막는다.
constexpr float MinYaw = -45.f, MaxYaw = 45.f;
constexpr float MinPitch = 30.f, MaxPitch = 75.f;
constexpr float MinDistance = 6.f, MaxDistance = 14.f;

void ZoomCamera(float amount)
{
	Camera& camera = renderer.camera;
	camera.distance = glm::clamp(camera.distance + amount, MinDistance, MaxDistance);
}

void CharCallback(GLFWwindow* window, unsigned int c)
{
	switch (c) {
	case '+': ZoomCamera(-0.5f); break;
	case '-': ZoomCamera(0.5f); break;
	case 'r': case 'R': renderer.camera = Camera{}; break;
	case 'p': renderer.isOrtho = !renderer.isOrtho; break;
	case 't': case 'T': renderer.toon = !renderer.toon; break;
	case 'g': case 'G': renderer.shadows = !renderer.shadows; break;
	case 'f': case 'F': renderer.speed = renderer.speed % 3 + 1; break;
	case 'q': case 'Q': glfwSetWindowShouldClose(window, true); break;
	}
}

// 휠: 앞으로 굴리면 가까이
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ZoomCamera(-(float)yoffset * 0.7f);
}

// 오른쪽 버튼을 누른 채 끌면 카메라 회전. 왼쪽 버튼은 카드·말 조작에 쓴다.
double lastCursorX = 0.0, lastCursorY = 0.0;
void CursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		Camera& camera = renderer.camera;
		camera.yaw = glm::clamp(camera.yaw - (float)(x - lastCursorX) * 0.3f, MinYaw, MaxYaw);
		camera.pitch = glm::clamp(camera.pitch + (float)(y - lastCursorY) * 0.2f, MinPitch, MaxPitch);
	}
	lastCursorX = x;
	lastCursorY = y;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// 누르고 있을 때의 반복 입력(GLFW_REPEAT)은 무시한다: B를 누르고 있으면 팩이 여러 개 사지는 문제
	if (action != GLFW_PRESS) {
		return;
	}
	// 마우스 조작의 단축키
	switch (key) {
	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
	case GLFW_KEY_SPACE: EndTurn(game); break;
	case GLFW_KEY_B: BuyPack(game); break;
	// 발표용
	case GLFW_KEY_F1: DemoNextRound(game); break;
	case GLFW_KEY_F2: DemoAddChips(game); break;
	case GLFW_KEY_F3: DemoBossRound(game); break;
	case GLFW_KEY_F4: DemoReplayEffects(game); break;
	}
}

// 왼쪽 버튼: 손패 카드를 눌러 끌고, 판 위에서 놓으면 사용한다. 팩·턴 종료 버튼은 클릭.
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT || renderer.transition.active) {
		return;
	}
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::vec2 mouse = renderer.mouse;

	if (action == GLFW_PRESS) {
		int card = HandCardAt(game, mouse, width, height);
		if (card >= 0) {
			renderer.dragCard = card;
		}
		else if (Contains(PackRect(width, height), mouse)) {
			BuyPack(game);
		}
		else if (Contains(GoldPackRect(width, height), mouse)) {
			BuyPack(game, true);
		}
		else if (Contains(EndTurnRect(width, height), mouse)) {
			EndTurn(game);
		}
		else if (Contains(SpeedRect(width, height), mouse) && (game.phase == Phase::Prepare || game.phase == Phase::Battle)) {
			renderer.speed = renderer.speed % 3 + 1;
		}
	}
	else if (action == GLFW_RELEASE && renderer.dragCard >= 0) {
		if (HandCardAt(game, mouse, width, height) >= 0) {
			// 손패 위에 다시 놓으면 취소
		}
		else if (Contains(PackRect(width, height), mouse)) {
			SellCard(game, renderer.dragCard);
		}
		else if (renderer.hoverI >= 0) {
			int handIndex = renderer.dragCard;
			int i = renderer.hoverI, j = renderer.hoverJ;
			if (game.hand[handIndex] < 0 && CanPlayCard(game, handIndex, i, j)) {
				// 화면 전환 카드: 원이 화면을 다 덮은 뒤에 카드를 쓴다 (main 루프)
				StartTransition(renderer, mouse, handIndex, i, j);
				game.events.push_back({ EventType::Summon, TileCenter(i, j) });
			}
			else {
				PlayCard(game, handIndex, i, j);
			}
		}
		renderer.dragCard = -1;
	}
}

// 마우스가 가리키는 칸을 renderer.hoverI/J에 넣는다.
// 마우스를 안 움직여도 카메라가 돌면 가리키는 칸이 바뀌므로 콜백이 아니라 매 프레임 부른다.
void UpdateHover(GLFWwindow* window)
{
	renderer.hoverI = renderer.hoverJ = -1;

	double x, y;
	glfwGetCursorPos(window, &x, &y);
	renderer.mouse = glm::vec2((float)x, (float)y);

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
	GLFWwindow* window = glfwCreateWindow(1280, 720, "카드 디펜스", nullptr, nullptr);
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
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetCursorPosCallback(window, CursorPosCallback);

	float last = (float)glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		float now = (float)glfwGetTime();
		float dt = now - last;
		last = now;
		if (dt > 0.1f) {
			dt = 0.1f;  // 창 드래그·중단점 뒤 큰 dt로 몹이 순간이동하지 않게
		}

		Phase before = game.phase;
		// 배속: dt를 키우지 않고 같은 dt로 여러 번 진행해야 공격·충돌을 건너뛰지 않는다
		for (int step = 0; step < renderer.speed; ++step) {
			GameUpdate(game, dt);
		}
		// 끄는 중에 결과 화면으로 넘어가면 손패가 바뀌므로 끌기를 취소한다 (다른 카드가 놓이지 않게)
		if (game.phase != before) {
			renderer.dragCard = -1;
		}
		RendererUpdate(renderer, game, dt);
		game.events.clear();  // 연출이 다 읽었으니 비운다
		if (TransitionCovered(renderer)) {
			const Renderer::Transition& t = renderer.transition;
			PlayCard(game, t.handIndex, t.i, t.j);
			renderer.transition.applied = true;
			renderer.camera = Camera{};
			if (game.quit) {
				glfwSetWindowShouldClose(window, true);
			}
		}
		UpdateHover(window);

		// 3D는 프레임버퍼 크기로, UI는 마우스와 같은 창 좌표로 그린다 (화면 배율이 100%가 아니면 둘이 다르다).
		int width, height, windowWidth, windowHeight;
		glfwGetFramebufferSize(window, &width, &height);
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		if (width > 0 && height > 0 && windowWidth > 0 && windowHeight > 0) {
			RenderGame(renderer, game, width, height);
			RenderUi(renderer, game, windowWidth, windowHeight);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}
