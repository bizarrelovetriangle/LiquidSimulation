#pragma once
#include "vector2.h"

class vector3
{
public:
	float x, y, z;
	vector3();
	vector3(float x, float y, float z);
	double dot_product(const vector2& v, const double& z) const;
	void scale(const double& scale);
};
