#pragma once
#include <SFML/Graphics.hpp>

class Line {
public:
	Line(sf::RenderWindow& window, sf::Vector2f a, sf::Vector2f b) 
		: _window(window), _shape(sf::LinesStrip, 2), a(a), b(b)
	{
		_shape[0] = a;
		_shape[1] = b;
	}


	void draw() {
		_window.draw(_shape);
	}

	sf::Vector2f a, b;
private:
	sf::RenderWindow& _window;
	sf::VertexArray _shape;
};