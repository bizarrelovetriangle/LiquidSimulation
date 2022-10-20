#pragma once
#include <SFML/Graphics.hpp>

class Particle {
public:
	Particle(sf::RenderWindow& window, sf::Vector2f position, float radius = 100)
		: _window(&window), _shape(radius), position(position), radius(radius)
	{
		_shape = sf::CircleShape(radius);
		_shape.setFillColor(sf::Color::White);
		static int _index = 0;
		index = _index++;
	}

	void update(float& interval) {
		velosity += acceleration * interval;
		position += velosity * interval;
	}

	void draw() {
		_shape.setPosition(position - sf::Vector2f(radius, radius));
		_window->draw(_shape);
	}

	sf::Vector2i gridPosition;

	sf::Vector2f position;
	sf::Vector2f velosity;
	sf::Vector2f acceleration;
	float radius;

	float density = 0;
	float density_near = 0;

	int index;
private:
	sf::RenderWindow* _window;
	sf::CircleShape _shape;
};