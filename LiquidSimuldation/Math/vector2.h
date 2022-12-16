#pragma once
#include "math.h"
#include <SFML/Graphics.hpp>

class vector2 : public sf::Vector2f{
public:
	static const vector2 zero_vector;
	vector2();
	vector2(const double& v);
	vector2(const double& x, const double& y);
	vector2(const sf::Vector2f& v);
	vector2 rotate(const double& radians) const;
	vector2 normalize() const;
	vector2 clockwise_perpendicular() const;
	bool is_zero() const;
	bool is_nan() const;
	double length() const;
	double distance(const vector2& o) const;
	vector2 negate() const;
	double dot_product(const vector2& o) const;
	double cross_product(const vector2& o) const;
	bool is_clockwise(const vector2& o) const;
	bool is_clockwise(const vector2& o, const vector2& relative) const;
	vector2 projection_to(const vector2& v) const;
	vector2 projection_to(const vector2& a, const vector2& b) const;
	float distance_to_line(vector2 a, vector2 b) const;
	vector2 operator*(const double& d) const;
	void operator*=(const double& d);
	vector2 operator/(const double& d) const;
	void operator/=(const double& d);
	vector2 operator+(const vector2& v) const;
	void operator+=(const vector2& v);
	vector2 operator-(const vector2& v) const;
	void operator-=(const vector2& v);
	void operator=(const vector2& v);
	vector2 operator-() const;
	bool operator==(const vector2& a) const;
};