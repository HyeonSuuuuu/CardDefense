// 판 원화용 구조 렌더: 화면 밖 1920x1080 프레임버퍼에 기본 카메라로 판만 그려 BMP로 저장 (말·적·UI 없음).
// 원화 투사(PaintMatrix)와 같은 행렬로 찍으므로 이 그림 위에 그린 원화가 판에 그대로 맞는다.
#include "Renderer.h"
#include "Util/Capture.h"
#include <cstdio>
#include <cstdlib>
#include <string>

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("사용: capture_board.exe <저장할 bmp> [paint [yaw [post]]]\n");
		return 1;
	}
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(320, 180, "capture_board", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	if (!LoadGameData()) {
		return 1;
	}
	Game game;
	Renderer renderer;
	GameInit(game);
	RendererInit(renderer);
	// 기본은 원화를 끄고 원래 칸 텍스처로 찍는다. paint를 주면 원화 투사 확인용 (yaw: 카메라를 돌려서)
	renderer.paint = argc > 2 && std::string(argv[2]) == "paint";
	renderer.camera.yaw = argc > 3 ? (float)atof(argv[3]) : 0.f;
	renderer.painterly = argc > 4 && std::string(argv[4]) == "post";  // 회화풍 후처리까지 거친 화면
	renderer.shadows = false;  // 그림자 패스가 기본 프레임버퍼로 되돌리지 않게 (말이 없어 그림자도 없음)
	// 창은 화면 크기에 잘리므로 크기를 정확히 맞춘 프레임버퍼에 그린다
	const int width = 1920, height = 1080;
	GLuint fbo, color, depth;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &color);
	glBindRenderbuffer(GL_RENDERBUFFER, color);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);
	glGenRenderbuffers(1, &depth);
	glBindRenderbuffer(GL_RENDERBUFFER, depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("framebuffer incomplete\n");
		return 1;
	}
	BeginScene(renderer, width, height);
	RenderGame(renderer, game, width, height);
	EndScene(renderer, width, height);
	bool ok = SaveScreenshotBmp(argv[1], width, height);
	printf("%s %dx%d\n", ok ? "saved" : "failed", width, height);
	glfwTerminate();
	return ok ? 0 : 1;
}
