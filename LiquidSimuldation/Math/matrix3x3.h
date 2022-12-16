#pragma once
#include "vector3.h"
#include "vector2.h"

class matrix3x3
{
public:
	vector3 i, j, k;
	matrix3x3();
	vector2 multiply(const vector2& v, const float& z) const;
	void scale(const float& scale);
	void scale(const vector3& scale);
	void rotate(const float& radians);
	void transfer(const vector2& v);
};
