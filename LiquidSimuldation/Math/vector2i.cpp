#include "Math/vector2i.h"
#include "Math/vector2.h"

vector2i::vector2i()
{
}

vector2i::vector2i(const int& v) : x(v), y(v)
{
}

vector2i::vector2i(const int& x, const int& y) : x(x), y(y)
{
}

vector2i::vector2i(const vector2& v) : x(v.x), y(v.y)
{
}

vector2i vector2i::operator+(const vector2i& v) const
{
	return vector2i(x + v.x, y + v.y);
}

void vector2i::operator+=(const vector2i& v)
{
	x += v.x;
	y += v.y;
}

vector2i vector2i::operator-() const
{
	return vector2i(-x, -y);
}

vector2i vector2i::operator-(const vector2i& v) const
{
	return vector2i(x - v.x, y - v.y);
}

void vector2i::operator-=(const vector2i& v)
{
	x -= v.x;
	y -= v.y;
}

vector2i vector2i::operator*(const int& v) const
{
	return vector2i(x * v, y * v);
}

void vector2i::operator*=(const int& v)
{
	x *= v;
	y *= v;
}

vector2i vector2i::operator/(const int& v) const
{
	return vector2i(x / v, y / v);
}

void vector2i::operator/=(const int& v)
{
	x /= v;
	y /= v;
}

bool vector2i::operator==(const vector2i& v) const
{
	return x == v.x && y == v.y;
}
