#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Particle.h"
#include "Line.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"

class FluidProcessor {
public:
	FluidProcessor(sf::RenderWindow& window, std::vector<Particle*> particles)
		: _window(window), _particles(particles), _particleGrid(particles, interactionRange)
	{

	}

	void wallCollicionHandling(std::vector<Line*>& walls) {
		for (auto particle : _particles) {
			for (auto wall : walls) {
				auto wallVector = VectorFunctions::normalize(wall->a - wall->b);
				bool isClockwise = VectorFunctions::isClockwise(wall->a, wall->b, particle->position);
				auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);

				float distanse = VectorFunctions::linePointDistance(wall->a, wall->b, particle->position);
				
				if (distanse < particle->radius) {
					particle->position += wallPerp * (particle->radius - distanse);
					particle->velosity -= wallPerp * VectorFunctions::dotProduct(particle->velosity, wallPerp) * 1.5f;
					particle->position += particle->velosity;
				}

				//drawWallNormal(wall, particle->position);
			}
		}
	}

	float interactionRange = 30;
	float restDensity = 130;
	float k = 0.0001;
	float k_near = 0.0001 * 70;

	void particlesGravity() {
		_particleGrid.updateParticleNeighbours();
		//return;
		for (auto particle : _particles) {
			particle->density = 0.f;
			particle->density_near = 0.f;

			auto neighbors = *_particleGrid.getNeighbours(particle);
			//auto neighbors = _particles;
			for (auto neighbor : neighbors) {
				float distance = VectorFunctions::distanse(particle->position, neighbor->position);
				float proximityCoefficient = 1 - distance / interactionRange;

				if (distance < interactionRange) {
					particle->density += pow(proximityCoefficient, 2);
					particle->density_near += pow(proximityCoefficient, 3);
				}
			}

			particle->density -= restDensity;
		}

		//return;
		for (auto particle : _particles) {
			particle->pressure = sf::Vector2f();

			auto neighbors = *_particleGrid.getNeighbours(particle);
			//auto neighbors = _particles;
			for (auto neighbor : neighbors) {
				float distance = VectorFunctions::distanse(particle->position, neighbor->position);

				if (distance < interactionRange) {
					sf::Vector2f normal = VectorFunctions::normalize(particle->position - neighbor->position);
					float proximityCoefficient = 1 - distance / interactionRange;
					float pressureM = k * neighbor->density * (proximityCoefficient);
					float nearPressureM = k_near * neighbor->density_near * pow(proximityCoefficient, 2);

					sf::Vector2f pressure = (pressureM + nearPressureM) * normal;

					particle->pressure += pressure;
					neighbor->position -= pressure;
				}
			}

			particle->position += particle->pressure;
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
	std::vector<Particle*> _particles;
	ParticleGrid _particleGrid;
};