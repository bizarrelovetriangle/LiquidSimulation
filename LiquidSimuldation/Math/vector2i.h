#pragma once

class vector2;

class vector2i {
public:
	int x = 0;
	int y = 0;

	vector2i();
	vector2i(const int& v);
	vector2i(const int& x, const int& y);
	vector2i(const vector2& v);
	vector2i operator+(const vector2i& v) const;
	void operator+=(const vector2i& v);
	vector2i operator-() const;
	vector2i operator-(const vector2i& v) const;
	void operator-=(const vector2i& v);
	vector2i operator*(const int& v) const;
	void operator*=(const int& v);
	vector2i operator/(const int& v) const;
	void operator/=(const int& v);
	bool operator==(const vector2i& v) const;
};
