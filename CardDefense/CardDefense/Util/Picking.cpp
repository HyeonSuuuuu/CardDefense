#include "Picking.h"

Ray ScreenToRay(glm::vec2 mouse, int width, int height, const glm::mat4& view, const glm::mat4& projection)
{
	// TODO(이현수): 창 좌표 → NDC(-1~1, y 뒤집기) → 니어·파 평면의 두 점을
	// inverse(projection * view)로 월드 좌표로 되돌리고(w로 나누기), 두 점으로 레이를 만든다.
	return { glm::vec3(0.f), glm::vec3(0.f, -1.f, 0.f) };
}

bool IntersectHorizontalPlane(const Ray& ray, float planeY, glm::vec3& hit)
{
	// TODO(이현수): origin.y + t * direction.y = planeY 를 t에 대해 푼다. 평행하거나 t < 0 이면 false.
	return false;
}
