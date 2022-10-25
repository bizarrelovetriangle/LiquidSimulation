#include "matrix3x3.h"

matrix3x3::matrix3x3()
{
	i.x = 1;
	j.y = 1;
	k.z = 1;
}

vector2 matrix3x3::multiply(const vector2& v, const double& z) const
{
	return vector2(i.dot_product(v, z), j.dot_product(v, z));
}

void matrix3x3::scale(const double& scale)
{
	i.scale(scale);
	j.scale(scale);
	k.scale(scale);
}

void matrix3x3::scale(const vector3& scale)
{
	i.scale(scale.x);
	j.scale(scale.y);
	k.scale(scale.z);
}

void matrix3x3::rotate(const double& radians)
{
	i.x = cos(radians);
	i.y = -sin(radians);
	j.x = sin(radians);
	j.y = cos(radians);
}

void matrix3x3::transfer(const vector2& v)
{
	i.z += v.x;
	j.z += v.y;
}
