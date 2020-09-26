#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Particle.h"
#include "Line.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"

class FluidProcessor {
public:
	FluidProcessor(sf::RenderWindow& window, ParticleGrid& particleGrid, float interactionRange)
		: _window(window), _particleGrid(particleGrid), _interactionRange(interactionRange)
	{

	}

	void wallCollicionHandling(std::vector<Line>& walls, float interval) {
		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				for (auto wall : walls) {
					auto wallVector = VectorFunctions::normalize(wall.a - wall.b);
					bool isClockwise = VectorFunctions::isClockwise(wall.a, wall.b, particle.position);
					auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);

					float distanse = VectorFunctions::linePointDistance(wall.a, wall.b, particle.position);

					if (distanse < particle.radius) {
						particle.position += wallPerp * (particle.radius - distanse);
						particle.velosity -= wallPerp * VectorFunctions::dotProduct(particle.velosity, wallPerp) * 1.5f;
						particle.position += particle.velosity * interval;
					}

					//drawWallNormal(wall, particle.position);
				}
			}
		}
	}

	float restDensity = 100;
	float k = 15;
	float k_near = 600;

	void particlesGravity(float interval) {
		float intervalTimeSquare = 0.5f * pow(interval, 2);

		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				particle.density = 0.f;
				particle.density_near = 0.f;
				particle.pressure = sf::Vector2f();

				auto neighborCells = _particleGrid.getNeighbours(particle);

				for (int colunm = 0; colunm < neighborCells.size1(); colunm++) {
					for (int row = 0; row < neighborCells.size2(); row++) {
						auto& neighbors = neighborCells(colunm, row);

						for (auto& neighbor : neighbors) {
							if (&neighbor == &particle) {
								continue;
							}

							float distance = VectorFunctions::distanse(neighbor.position, particle.position);

							if (distance < _interactionRange) {
								float proximityCoefficient = 1 - distance / _interactionRange;
								particle.density += pow(proximityCoefficient, 2);
								particle.density_near += pow(proximityCoefficient, 3);
							}
						}
					}
				}

				float pressureM = k * (particle.density - restDensity);
				float nearPressureM = k_near * particle.density_near;

				for (int colunm = 0; colunm < neighborCells.size1(); colunm++) {
					for (int row = 0; row < neighborCells.size2(); row++) {
						auto& neighbors = neighborCells(colunm, row);

						for (auto& neighbor : neighbors) {
							if (&neighbor == &particle) {
								continue;
							}

							sf::Vector2f vector = neighbor.position - particle.position;

							if (vector == sf::Vector2f(0, 0)) {
								continue;
							}

							float vectorLength = VectorFunctions::length(vector);

							if (vectorLength < _interactionRange) {
								sf::Vector2f vectorNormal = vector / vectorLength;
								float proximityCoefficient = 1 - vectorLength / _interactionRange;

								sf::Vector2f pressure =
									intervalTimeSquare *
									(pressureM * proximityCoefficient + 
										nearPressureM * pow(proximityCoefficient, 2)) * vectorNormal;

								particle.pressure -= pressure;
								neighbor.position += pressure;
							}
						}
					}
				}

				particle.position += particle.pressure;
			}
		}
	}

private:
	void drawWallNormal(Line& wall, sf::Vector2f point) {
		auto wallVector = VectorFunctions::normalize(wall.a - wall.b);
		bool isClockwise = VectorFunctions::isClockwise(wall.a, wall.b, point);
		auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);

		auto normal_a = (wall.a + wall.b) / 2.f;
		auto normal_b = normal_a + wallPerp * 10.f;

		Line line(_window, normal_a, normal_b);
		line.draw();
	}

	sf::RenderWindow& _window;
	ParticleGrid& _particleGrid;
	float _interactionRange = 30;
};