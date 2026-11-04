#pragma once

// 현재 그려진 화면을 24비트 BMP로 저장한다. glfwSwapBuffers 전에 호출해야 한다.
// 실패하면 false.
bool SaveScreenshotBmp(const char* path, int width, int height);
