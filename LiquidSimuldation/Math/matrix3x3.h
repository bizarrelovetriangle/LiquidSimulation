#pragma once
#include "vector3.h"
#include "vector2.h"

class matrix3x3
{
public:
	vector3 i, j, k;
	matrix3x3();
	vector2 multiply(const vector2& v, const double& z) const;
	void scale(const double& scale);
	void scale(const vector3& scale);
	void rotate(const double& radians);
	void transfer(const vector2& v);
};
