#pragma once
#include <SFML/Graphics.hpp>
#include <math.h>
#include "Particle.h"
#include "Line.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"

class FluidProcessor {
public:
	FluidProcessor(ParticleGrid& particleGrid, float interactionRange)
		: _particleGrid(particleGrid), _interactionRange(interactionRange)
	{

	}

	void wallCollicionHandling(std::vector<Line>& walls, float interval) {
		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				for (auto& wall : walls) {
					auto wallVector = VectorFunctions::normalize(wall.a - wall.b);
					bool isClockwise = VectorFunctions::isClockwise(wall.a, wall.b, particle.position);
					auto wallPerp = VectorFunctions::perpendicular(wallVector, isClockwise);
					
					auto particleWallVelosity = -VectorFunctions::dotProduct(particle.velosity * interval, wallPerp);
					float distanse = VectorFunctions::linePointDistance(wall.a, wall.b, particle.position);

					distanse -= particleWallVelosity;

					if (distanse < particle.radius) {
						particle.position += wallPerp * (particle.radius - distanse);
						particle.velosity -= wallPerp * VectorFunctions::dotProduct(particle.velosity, wallPerp) * 1.5f;
						particle.position += particle.velosity * interval;
					}
				}
			}
		}
	}

	void particlesGravity(float& interval) {
		float intervalTimeSquare = 0.5f * pow(interval, 2);

		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				particle.density = 0.f;
				particle.density_near = 0.f;

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
										nearPressureM * pow(proximityCoefficient, 2)) * 
									vectorNormal;

								particle.position -= pressure;
								neighbor.position += pressure;
							}
						}
					}
				}
			}
		}
	}

	void applyViscosity(float& interval) {
		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				auto neighborCells = _particleGrid.getNeighbours(particle);

				for (int colunm = 0; colunm < neighborCells.size1(); colunm++) {
					for (int row = 0; row < neighborCells.size2(); row++) {
						auto& neighbors = neighborCells(colunm, row);

						for (auto& neighbor : neighbors) {
							if (&neighbor == &particle) {
								continue;
							}
							
							if (particle.index < neighbor.index) continue;

							sf::Vector2f vector = particle.position - neighbor.position;

							if (vector == sf::Vector2f(0, 0)) {
								continue;
							}

							float vectorLength = VectorFunctions::length(vector);

							if (vectorLength < _interactionRange) {
								sf::Vector2f vectorNormal = vector / vectorLength;
								float proximityCoefficient = 1 - vectorLength / _interactionRange;

								float inertia = VectorFunctions::dotProduct(
									particle.velosity - neighbor.velosity, vectorNormal);

								if (inertia > 0) {
									sf::Vector2f inertiaViscocity = 0.5f * interval * proximityCoefficient *
										(kLinearViscocity * inertia +
											kQuadraticViscocity * pow(inertia, 2)) *
										vectorNormal;

									particle.velosity -= inertiaViscocity;
									neighbor.velosity += inertiaViscocity;
								}
							}
						}
					}
				}
			}
		}
	}

private:
	ParticleGrid& _particleGrid;
	float _interactionRange;

	float restDensity = 100;
	float k = 15;
	float k_near = 600;
	float kLinearViscocity = 0.001;
	float kQuadraticViscocity = 0.1;
};