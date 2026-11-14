#include "Game.h"
#include "Renderer.h"
#include "Util/Picking.h"

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
	switch (key) {
	case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, true); break;
	// 임시 키: Start 카드 끌어 놓기가 생기기 전까지 사용
	case GLFW_KEY_ENTER:
		if (game.phase == Phase::Title) {
			game.phase = Phase::Prepare;
		}
		break;
	case GLFW_KEY_SPACE:
		if (game.phase == Phase::Prepare) {
			EndTurn(game);
		}
		break;
	}
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

	GameInit(game);
	RendererInit(renderer);

	glfwSetCharCallback(window, CharCallback);
	glfwSetKeyCallback(window, KeyCallback);

	float last = (float)glfwGetTime();
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
