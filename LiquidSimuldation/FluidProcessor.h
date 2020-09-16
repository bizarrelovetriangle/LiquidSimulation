#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Particle.h"
#include "Line.h"
#include "VectorFunctions.h"

class FluidProcessor {
public:
	FluidProcessor(sf::RenderWindow& window) : _window(window)
	{

	}

	void wallCollicionHandling(std::vector<Particle*>& particles, std::vector<Line*>& walls) {
		for (auto particle : particles) {
			for (auto wall : walls) {
				auto wallVector = VectorFunctions::normalize(wall->a - wall->b);
				bool isClockwise = VectorFunctions::isClockwise(wall->a, wall->b, particle->position);
				auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);

				float dis = VectorFunctions::linePointDistance(wall->a, wall->b, particle->position);
				
				if (VectorFunctions::linePointDistance(wall->a, wall->b, particle->position) < particle->radius) {
					particle->velosity -= wallPerp * VectorFunctions::dotProduct(particle->velosity, wallPerp) * 2.f;
				}

				drawWallNormal(wall, particle->position);
			}
		}
	}

	float interactionRange = 100;
	float restDensity = -100;
	float k = 0.0000005;
	float k_near = 0;

	void particlesGravity(std::vector<Particle*>& particles) {
		for (auto particle : particles) {
			particle->density = 0.f;
			particle->density_near = 0.f;

			for (auto neighbor : particles) {
				float distance = VectorFunctions::distanse(
					particle->position, neighbor->position);

				if (distance < interactionRange) {
					particle->density += pow(1 - distance / interactionRange, 2) - restDensity;
					particle->density_near += pow(1 - distance / interactionRange, 3);
				}
			}
		}

		for (auto particle : particles) {
			particle->pressure = sf::Vector2f();

			for (auto neighbor : particles) {
				float distance = VectorFunctions::distanse(
					particle->position, neighbor->position);

				if (distance < interactionRange) {
					particle->pressure += 
						k * (neighbor->density * (1 - distance / interactionRange) + 
						k_near * neighbor->density_near * pow(1 - distance / interactionRange, 2)) *
						VectorFunctions::normalize(particle->position - neighbor->position);

					particle->position_prev = particle->position;
					particle->position += particle->pressure;
					particle->velosity += particle->position_prev - particle->position;
				}
			}
		}
	}

private:
	void drawWallNormal(Line* wall, sf::Vector2f point) {
		auto wallVector = VectorFunctions::normalize(wall->a - wall->b);
		bool isClockwise = VectorFunctions::isClockwise(wall->a, wall->b, point);
		auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);

		auto normal_a = (wall->a + wall->b) / 2.f;
		auto normal_b = normal_a + wallPerp * 10.f;

		Line line(_window, normal_a, normal_b);
		line.draw();
	}

	sf::RenderWindow& _window;
};