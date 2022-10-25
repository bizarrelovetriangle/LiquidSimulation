#include "vector3.h"

vector3::vector3()
	: x(0), y(0), z(0)
{
}

vector3::vector3(float x, float y, float z)
	: x(x), y(y), z(z)
{
}

double vector3::dot_product(const vector2& v, const double& z) const
{
	return this->x * v.x + this->y * v.y + this->z * z;
}

void vector3::scale(const double& scale) 
{
	x *= scale;
	y *= scale;
	z *= scale;
}

