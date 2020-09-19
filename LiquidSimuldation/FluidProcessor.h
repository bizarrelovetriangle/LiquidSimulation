#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Particle.h"
#include "Line.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"

class FluidProcessor {
public:
	FluidProcessor(sf::RenderWindow& window, 
		std::vector<Particle*>& particles, ParticleGrid& particleGrid, float interactionRange)
		: _window(window), _particles(particles), _particleGrid(particleGrid), _interactionRange(interactionRange)
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

	float restDensity = 100;
	float k = 0.0001;
	float k_near = 0.0001 * 40;

	void particlesGravity() {
		//return;
		for (auto particle : _particles) {
			particle->density = 0.f;
			particle->density_near = 0.f;
			particle->pressure = sf::Vector2f();

			auto neighborCells = _particleGrid.getNeighbours(particle);
			
			for (int colunm = 0; colunm < neighborCells.size1(); colunm++) {
				for (int row = 0; row < neighborCells.size2(); row++) {
					auto& neighbors = neighborCells(colunm, row);

					for (auto neighbor : neighbors) {
						if (neighbor == particle) {
							continue;
						}

						float distance = VectorFunctions::distanse(neighbor->position, particle->position);

						if (distance < _interactionRange) {
							float proximityCoefficient = 1 - distance / _interactionRange;
							particle->density += pow(proximityCoefficient, 2);
							particle->density_near += pow(proximityCoefficient, 3);
						}
					}
				}
			}

			float pressureM = k * (particle->density - restDensity);
			float nearPressureM = k_near * particle->density_near;

			for (int colunm = 0; colunm < neighborCells.size1(); colunm++) {
				for (int row = 0; row < neighborCells.size2(); row++) {
					auto& neighbors = neighborCells(colunm, row);

					for (auto neighbor : neighbors) {
						if (neighbor == particle) {
							continue;
						}

						sf::Vector2f vector = neighbor->position - particle->position;
						float vectorLength = VectorFunctions::length(vector);
						
						if (vectorLength < _interactionRange) {
							sf::Vector2f vectorNormal = vector / vectorLength;
							float proximityCoefficient = 1 - vectorLength / _interactionRange;

							sf::Vector2f pressure = 
								(pressureM * proximityCoefficient + 
								nearPressureM * pow(proximityCoefficient, 2)) * vectorNormal;

							particle->pressure -= pressure;
							neighbor->position += pressure;
						}
					}
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
	std::vector<Particle*>& _particles;
	ParticleGrid& _particleGrid;
	float _interactionRange = 30;
};