#include "vector2.h"
#include <Math/vector2i.h>
#include <Math/vector3.h>
#include <cmath>

const vector2 vector2::zero_vector(0);

vector2::vector2()
{
}

vector2::vector2(const float& v) : x(v), y(v)
{
}

vector2::vector2(const float& x, const float& y) : x(x), y(y)
{
}

vector2::vector2(const vector2i& v) : x(v.x), y(v.y)
{
}

vector2::vector2(const vector3& v) : x(v.x), y(v.y)
{
}

vector2 vector2::normalize() const
{
	float length = this->length();
	if (length == 0) return vector2(0, -1);
	return vector2(x / length, y / length);
}

vector2 vector2::perpendicular(bool clockwise) const
{
	return clockwise ? vector2(y, -x) : vector2(-y, x);
}

bool vector2::is_zero() const
{
	return x == 0 && y == 0;
}

bool vector2::is_nan() const
{
	return std::isnan(x) || std::isnan(y);
}

float vector2::length() const
{
	float _sqrt = sqrt(x * x + y * y);
	return std::isnan(_sqrt) ? 0 : _sqrt;
}

float vector2::distance(const vector2& o) const
{
	return (*this - o).length();
}
			
vector2 vector2::negate() const
{
	return vector2(-x, -y);
}

float vector2::dot_product(const vector2& o) const
{
	return x * o.x + y * o.y;
}

float vector2::cross_product(const vector2& o) const
{
	return x * o.y - y * o.x;
}

bool vector2::is_clockwise(const vector2& o) const
{
	return cross_product(o) < 0;
}

bool vector2::is_clockwise(const vector2& o, const vector2& relative) const
{
	return (*this - relative).cross_product(o - relative) < 0;
}

vector2 vector2::projection_to(const vector2& v) const
{
	auto v_normalize = v.normalize();
	return v_normalize * v_normalize.dot_product(*this);
}

vector2 vector2::projection_to(const vector2& a, const vector2& b) const
{
	auto a_b_normalize = (a - b).normalize();
	return a_b_normalize * a_b_normalize.dot_product(*this - b) + b;
}

float vector2::distance_to_line(vector2 a, vector2 b) const
{
	vector2 a_b = a - b;
	vector2 o_b = *this - b;
	float a_b_length = a_b.length();
	vector2 a_b_normal = a_b / a_b_length;

	float dot = a_b_normal.dot_product(o_b);
	if (dot < 0) return o_b.length();
	else if (dot > a_b_length) return (*this - a).length();
	return std::abs(a_b_normal.cross_product(o_b));
}

vector2 vector2::operator*(const float& d) const
{
	return vector2(x * d, y * d);
}

void vector2::operator*=(const float& d)
{
	x = x * d;
	y = y * d;
}

vector2 vector2::operator/(const float& d) const
{
	return vector2(x / d, y / d);
}

void vector2::operator/=(const float& d)
{
	x = x / d;
	y = y / d;
}

vector2 vector2::operator+(const vector2& v) const
{
	return vector2(x + v.x, y + v.y);
}

void vector2::operator+=(const vector2& v)
{
	x = x + v.x;
	y = y + v.y;
}

vector2 vector2::operator-(const vector2& v) const
{
	return vector2(x - v.x, y - v.y);
}

void vector2::operator-=(const vector2& v)
{
	x = x - v.x;
	y = y - v.y;
}

void vector2::operator=(const vector2& v)
{
	x = v.x;
	y = v.y;
}

vector2 vector2::operator-() const
{
	return vector2(-x, -y);
}

bool vector2::operator==(const vector2& a) const
{
	return x == a.x && y == a.y;
}