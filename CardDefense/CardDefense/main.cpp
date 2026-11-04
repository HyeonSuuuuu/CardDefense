#include "Game.h"
#include "Renderer.h"

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

		GameUpdate(game, dt);
		RendererUpdate(renderer, dt);

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
