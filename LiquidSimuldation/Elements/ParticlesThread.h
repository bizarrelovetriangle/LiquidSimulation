#pragma once
#include <Math/vector2.h>

struct alignas(8) ParticlesThread {
	ParticlesThread(int first, int second, const vector2& normal, float proximity_coefficient)
		: first(first), second(second)
	{}

	ParticlesThread() {}

	int first;
	int second;
	float rest_length;
	bool torn;
};