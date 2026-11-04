#pragma once
#include <gl/glm/glm.hpp>

// 월드 공간의 반직선. direction은 정규화되어 있어야 한다.
struct Ray
{
	glm::vec3 origin;
	glm::vec3 direction;
};

// 마우스 좌표(창 왼쪽 위가 원점, y는 아래로 증가)를 월드 공간 레이로 바꾼다.
Ray ScreenToRay(glm::vec2 mouse, int width, int height, const glm::mat4& view, const glm::mat4& projection);

// 레이와 수평면 y = planeY의 교점을 hit에 넣는다. 평면과 평행하거나 카메라 뒤쪽이면 false.
bool IntersectHorizontalPlane(const Ray& ray, float planeY, glm::vec3& hit);
