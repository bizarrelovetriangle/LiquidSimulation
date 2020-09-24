#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>

class VectorFunctions {
public:
	//const static sf::Vector2f zero(0, 0);

	static inline sf::Vector2f normalize(sf::Vector2f vector) {
		float len = length(vector);
		return len == 0 ? sf::Vector2f() : vector / length(vector);
	}

	static inline sf::Vector2f perpendicular(sf::Vector2f vector, bool isClockwise) {
		return isClockwise ? sf::Vector2f(vector.y, -vector.x) : sf::Vector2f(-vector.y, vector.x);
	}

	static inline bool isClockwise(sf::Vector2f line_a, sf::Vector2f line_b, sf::Vector2f point) {
		return crossProduct(line_a - line_b, point - line_b) < 0;
	}

	static inline float vectorPointDistance(sf::Vector2f line_a, sf::Vector2f line_b, sf::Vector2f point) {
		sf::Vector2f line_vector_normal = normalize(line_a - line_b);
		sf::Vector2f projection_point = dotProduct(line_vector_normal, point - line_b) * line_vector_normal + line_b;
		return length(point - projection_point);
	}

	static inline float linePointDistance(sf::Vector2f line_a, sf::Vector2f line_b, sf::Vector2f point) {
		sf::Vector2f line_vector_normal = normalize(line_a - line_b);
		float dot_product = dotProduct(line_vector_normal, point - line_b);
		
		if (dot_product < 0) {
			return length(point - line_b);
		}
		else if (dot_product > length(line_a - line_b)) {
			return length(point - line_a);
		}

		sf::Vector2f proejction_point = dot_product * line_vector_normal + line_b;
		return length(point - proejction_point);
	}

	static inline float length(sf::Vector2f vector) {
		float res = sqrt(pow(vector.x, 2) + pow(vector.y, 2));
		return isnan(res) ? 0 : res;
	}

	static inline float distanse(sf::Vector2f point_a, sf::Vector2f point_b) {
		return length(point_a - point_b);
	}

	static inline float crossProduct(
		sf::Vector2f vector_a, sf::Vector2f vector_b)
	{
		return vector_a.x * vector_b.y - vector_a.y * vector_b.x;
	}

	static inline float dotProduct(
		sf::Vector2f vector_a, sf::Vector2f vector_b)
	{
		return vector_a.x * vector_b.x + vector_a.y * vector_b.y;
	}
};