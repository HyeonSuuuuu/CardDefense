#pragma once
#include "../Common.h"

// OBJ 모델 하나 = 메시 + 텍스처(MTL의 map_Kd) + 크기 정보
struct Model
{
	Mesh mesh;
	GLuint texture = 0;              // 없으면 0
	glm::vec3 boundsMin, boundsMax;  // 원본 좌표의 경계 상자 (크기 맞추기에 사용)
};

// OBJ를 읽는다. v, vt, vn, f(삼각형·사각형, v / v/vt / v//vn / v/vt/vn)와 mtllib의 map_Kd만 처리한다.
// 실패하면 콘솔에 이유를 출력하고 false를 돌려준다.
bool LoadModel(const char* objPath, Model& model);

// 텍스처가 있으면 묶어서 그린다. 색은 uColor와 곱해진다.
void DrawModel(GLuint program, const Model& model, const glm::mat4& transform);
