#pragma once
#include "math.h"

class vector2i;

class vector2 {
public:
	static const vector2 zero_vector;

	float x = 0.;
	float y = 0.;

	vector2();
	vector2(const float& v);
	vector2(const float& x, const float& y);
	vector2(const vector2i& v);
	vector2 rotate(const float& radians) const;
	vector2 normalize() const;
	vector2 clockwise_perpendicular() const;
	bool is_zero() const;
	bool is_nan() const;
	float length() const;
	float distance(const vector2& o) const;
	vector2 negate() const;
	float dot_product(const vector2& o) const;
	float cross_product(const vector2& o) const;
	bool is_clockwise(const vector2& o) const;
	bool is_clockwise(const vector2& o, const vector2& relative) const;
	vector2 projection_to(const vector2& v) const;
	vector2 projection_to(const vector2& a, const vector2& b) const;
	float distance_to_line(vector2 a, vector2 b) const;
	vector2 operator*(const float& d) const;
	void operator*=(const float& d);
	vector2 operator/(const float& d) const;
	void operator/=(const float& d);
	vector2 operator+(const vector2& v) const;
	void operator+=(const vector2& v);
	vector2 operator-(const vector2& v) const;
	void operator-=(const vector2& v);
	void operator=(const vector2& v);
	vector2 operator-() const;
	bool operator==(const vector2& a) const;
};