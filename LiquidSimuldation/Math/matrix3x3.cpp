#include "matrix3x3.h"

matrix3x3::matrix3x3()
{
	i.x = 1;
	j.y = 1;
	k.z = 1;
}

matrix3x3::matrix3x3(const vector3 col1, const vector3 col2, const vector3 col3)
	: i(col1.x, col2.x, col3.x),
	  j(col1.y, col2.y, col3.y),
	  k(col1.z, col2.z, col3.z)
{
}

vector3 matrix3x3::operator*(const vector3& vec) const
{
	return vector3(i.dot_product(vec), j.dot_product(vec), k.dot_product(vec));
}

vector2 matrix3x3::operator*(const vector2& vec) const
{
	vector3 vec3(vec, 1);
	return vector2(i.dot_product(vec3), j.dot_product(vec3));
}

matrix3x3 matrix3x3::operator*(const matrix3x3& mat) const
{
	vector3 col1(mat.i.x, mat.j.x, mat.k.x);
	vector3 col2(mat.i.y, mat.j.y, mat.k.y);
	vector3 col3(mat.i.z, mat.j.z, mat.k.z);
	return matrix3x3(operator*(col1), operator*(col2), operator*(col3));
}

void matrix3x3::operator*=(const float& scale)
{
	i *= scale;
	j *= scale;
	k *= scale;
}

void matrix3x3::operator*=(const vector3& scale)
{
	i *= scale.x;
	j *= scale.y;
	k *= scale.z;
}

matrix3x3 matrix3x3::identity()
{
	return matrix3x3();
}

matrix3x3 matrix3x3::rotate(const float& radians)
{
	matrix3x3 mat;
	mat.i.x = cos(radians);
	mat.i.y = -sin(radians);
	mat.j.x = sin(radians);
	mat.j.y = cos(radians);
	return mat;
}

matrix3x3 matrix3x3::transfer(const vector2& v)
{
	matrix3x3 mat;
	mat.i.z += v.x;
	mat.j.z += v.y;
	return mat;
}
