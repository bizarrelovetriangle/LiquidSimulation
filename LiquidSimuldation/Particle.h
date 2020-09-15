#pragma once
#include <SFML/Graphics.hpp>

class Particle {
public:
	Particle(sf::RenderWindow& window, sf::Vector2f position, float radius = 100)
		: _window(window), _shape(radius), _position(position), _radius(radius)
	{
		_shape.setFillColor(sf::Color::White);
		_velosity.y = 3.1;
	}

	void update() {
		_position += _velosity;
	}

	void draw() {
		_shape.setPosition(_position - sf::Vector2f(_radius, _radius));
		_window.draw(_shape);
	}

private:
	sf::RenderWindow& _window;
	sf::CircleShape _shape;
	sf::Vector2f _prevPosition;
	sf::Vector2f _position;
	sf::Vector2f _velosity;
	float _radius;
};