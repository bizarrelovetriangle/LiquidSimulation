#include "vector3.h"

vector3::vector3()
	: x(0), y(0), z(0)
{
}

vector3::vector3(float x, float y, float z)
	: x(x), y(y), z(z)
{
}

vector3::vector3(const vector2& vec, float z)
	: vector3(vec.x, vec.y, z)
{}

float vector3::dot_product(const vector3& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}

void vector3::operator*=(const float& v) 
{
	x *= v;
	y *= v;
	z *= v;
}

vector3 vector3::operator*(const float& v) const
{
	vector3 vec = *this;
	vec *= v;
	return vec;
}