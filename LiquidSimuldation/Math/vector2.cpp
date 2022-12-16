#include "vector2.h"

const vector2 vector2::zero_vector(0);

vector2::vector2() : sf::Vector2f(0, 0)
{
}

vector2::vector2(const double& v) : sf::Vector2f((float) v, (float) v)
{
	x = v;
	y = v;
}

vector2::vector2(const double& x, const double& y) : sf::Vector2f((float) x, (float) y)
{
	this->x = x;
	this->y = y;
}

vector2::vector2(const sf::Vector2f& v) : sf::Vector2f(v)
{
}

vector2 vector2::rotate(const double& radians) const
{
	double _x = x * cos(radians) - y * sin(radians);
	double _y = x * sin(radians) + y * cos(radians);
	return vector2(_x, _y);
}

vector2 vector2::normalize() const
{
	double length = this->length();

	if (length == 0) {
		return vector2(0, -1);
	}

	return vector2(x / length, y / length);
}

vector2 vector2::clockwise_perpendicular() const
{
	return vector2(y, -x);
}

bool vector2::is_zero() const
{
	return x == 0 && y == 0;
}

bool vector2::is_nan() const
{
	return std::isnan(x) || std::isnan(y);
}

double vector2::length() const
{
	double _sqrt = sqrt(x * x + y * y);
	return std::isnan(_sqrt) ? 0 : _sqrt;
}

double vector2::distance(const vector2& o) const
{
	return (*this - o).length();
}
			
vector2 vector2::negate() const
{
	return vector2(-x, -y);
}

double vector2::dot_product(const vector2& o) const
{
	return x * o.x + y * o.y;
}

double vector2::cross_product(const vector2& o) const
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

vector2 vector2::operator*(const double& d) const
{
	return vector2(x * d, y * d);
}

void vector2::operator*=(const double& d)
{
	x = x * d;
	y = y * d;
}

vector2 vector2::operator/(const double& d) const
{
	return vector2(x / d, y / d);
}

void vector2::operator/=(const double& d)
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