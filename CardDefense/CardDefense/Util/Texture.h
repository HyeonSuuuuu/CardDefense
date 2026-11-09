#pragma once
#include <gl/glew.h>

// PNG·JPG를 읽어 OpenGL 텍스처를 만든다. 실패하면 콘솔에 이유를 출력하고 0을 돌려준다.
// 이미지 위아래를 뒤집어서 읽는다 (OBJ의 UV는 왼쪽 아래가 원점).
GLuint LoadTexture(const char* path);
