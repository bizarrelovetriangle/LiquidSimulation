#include "matrix3x3.h"

matrix3x3::matrix3x3()
{
	i.x = 1;
	j.y = 1;
	k.z = 1;
}

matrix3x3::matrix3x3(const vector3& i, const vector3& j, const vector3& k)
	: i(i), j(j), k(k)
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
	auto trans = mat.transpose();
	return matrix3x3(operator*(trans.i), operator*(trans.j), operator*(trans.k)).transpose();
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

matrix3x3 matrix3x3::transpose() const
{
	return matrix3x3(
		vector3(i.x, j.x, k.x),
		vector3(i.y, j.y, k.y),
		vector3(i.z, j.z, k.z));
}
