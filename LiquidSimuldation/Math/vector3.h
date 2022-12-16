#pragma once
#include "vector2.h"

class vector3
{
public:
	float x, y, z;
	vector3();
	vector3(float x, float y, float z);
	float dot_product(const vector2& v, const float& z) const;
	void scale(const float& scale);
};
