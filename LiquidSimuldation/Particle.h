#pragma once
#include <SFML/Graphics.hpp>

class Particle {
public:
	Particle(sf::RenderWindow& window, sf::Vector2f position, float radius = 100)
		: _window(&window), _shape(radius), position(position), radius(radius)
	{
		_shape = sf::CircleShape(radius);
		_shape.setFillColor(sf::Color::White);
	}

	void update(float& interval) {
		position_prev = position;
		velosity += acceleration * interval;
		position += velosity * interval;
	}

	void relaxVelosity(float& interval) {
		velosity = (position - position_prev) / interval;
	}

	void draw() {
		_shape.setPosition(position - sf::Vector2f(radius, radius));
		_window->draw(_shape);
	}

	std::vector<Particle*> neightbours;
	sf::Vector2i gridPosition;
	int neightboursIndex = -1;

	sf::Vector2f position;
	sf::Vector2f velosity;
	sf::Vector2f acceleration;
	float radius;

	float density = 0;
	float density_near = 0;
	sf::Vector2f pressure;
	
	sf::Vector2f position_prev;

private:
	sf::RenderWindow* _window;
	sf::CircleShape _shape;
};