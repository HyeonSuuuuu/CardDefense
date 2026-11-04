#pragma once
#include <cmath>

// 0~1 진행도 t를 받아 0~1(넘칠 수 있음) 모양으로 바꾸는 함수. 유닛 낙하, 카드 이동, 몹 점프 연출에 쓴다.
// 그래프: https://easings.net

inline float Clamp01(float t)
{
	return t < 0.f ? 0.f : (t > 1.f ? 1.f : t);
}

// 천천히 시작해 천천히 끝남
inline float EaseInOutQuad(float t)
{
	t = Clamp01(t);
	return t < 0.5f ? 2.f * t * t : 1.f - std::pow(-2.f * t + 2.f, 2.f) * 0.5f;
}

// 빠르게 시작해 부드럽게 멈춤
inline float EaseOutCubic(float t)
{
	t = Clamp01(t);
	return 1.f - std::pow(1.f - t, 3.f);
}

// 목표를 살짝 넘었다가 돌아옴 (유닛이 떨어져 튕기는 느낌)
inline float EaseOutBack(float t)
{
	t = Clamp01(t);
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.f;
	return 1.f + c3 * std::pow(t - 1.f, 3.f) + c1 * std::pow(t - 1.f, 2.f);
}
