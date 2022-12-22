#pragma once
#include "vector2.h"

class vector3
{
public:
	float x, y, z;
	vector3();
	vector3(float x, float y, float z);
	vector3(const vector2& vec, float z);
	float dot_product(const vector3& vec) const;
	void operator*=(const float& v);
	vector3 operator*(const float& v) const;
};
