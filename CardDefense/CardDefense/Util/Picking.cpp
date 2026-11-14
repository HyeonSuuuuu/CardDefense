#include "Picking.h"

Ray ScreenToRay(glm::vec2 mouse, int width, int height, const glm::mat4& view, const glm::mat4& projection)
{
	auto x = 2 * mouse.x / width - 1;
	auto y = 1 - 2 * mouse.y / height;
	
	glm::vec4 near = glm::vec4(x, y, -1, 1);
	glm::vec4 far = glm::vec4(x, y, 1, 1);

	glm::mat4 inv = glm::inverse(projection * view);
	near = inv * near;
	near /= near.w;
	far = inv * far;
	far/= far.w;

	Ray ray;
	ray.origin = glm::vec3(near);
	ray.direction = glm::normalize(glm::vec3(far) - glm::vec3(near));

	return ray;
}

bool IntersectHorizontalPlane(const Ray& ray, float planeY, glm::vec3& hit)
{
	const auto& origin = ray.origin;
	const auto& dir = ray.direction;
	if (glm::abs(dir.y) < 1e-6f) // dir.y가 0인지부터 체크
		return false;
	auto t = (planeY - origin.y) / dir.y;
	if (t < 0)
		return false;
	hit = origin + t * dir;
	return true;
}
