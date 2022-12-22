#pragma once
#include "vector3.h"
#include "vector2.h"

class matrix3x3
{
public:
	vector3 i, j, k;
	matrix3x3();
	matrix3x3(const vector3 col1, const vector3 col2, const vector3 col3);
	vector3 operator*(const vector3& vec) const;
	vector2 operator*(const vector2& vec) const;
	matrix3x3 operator*(const matrix3x3& mat) const;
	void operator*=(const float& scale);
	void operator*=(const vector3& scale);
	static matrix3x3 identity();
	static matrix3x3 rotate(const float& radians);
	static matrix3x3 transfer(const vector2& v);
};
