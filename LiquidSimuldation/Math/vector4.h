#pragma once
#include "vector2.h"

class vector4
{
public:
	float x = 0., y = 0., z = 0., a = 0.;
	vector4() {};
	vector4(float x, float y, float z, float a)
		: x(x), y(y), z(z), a(a)
	{};
};
