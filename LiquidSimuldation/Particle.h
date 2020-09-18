#pragma once
#include <SFML/Graphics.hpp>

class Particle {
public:
	Particle(sf::RenderWindow& window, sf::Vector2f position, float radius = 100)
		: _window(window), _shape(radius), position(position), radius(radius)
	{
		_shape.setFillColor(sf::Color::White);
	}

	void update() {
		velosity += acceleration;
		position += velosity;
	}

	void draw() {
		_shape.setPosition(position - sf::Vector2f(radius, radius));
		_window.draw(_shape);
	}

	std::vector<Particle*> neightbours;
	int gridColumn;
	int gridRow;
	int neightboursIndex;

	sf::Vector2f position;
	sf::Vector2f velosity;
	sf::Vector2f acceleration;
	float radius;

	float density;
	float density_near;
	sf::Vector2f pressure;
	
	sf::Vector2f position_prev;

private:
	sf::RenderWindow& _window;
	sf::CircleShape _shape;
};