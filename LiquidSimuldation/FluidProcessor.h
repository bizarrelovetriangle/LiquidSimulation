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

	void createPairs() {
		pairs.clear();

		for (auto& particles : _particleGrid.GridCells.data()) {
			for (auto& particle : particles) {
				particle.density = 0.f;
				particle.density_near = 0.f;
				auto neighborCells = _particleGrid.getNeighbours(particle);

				for (int colunm = 0; colunm < neighborCells.size1(); ++colunm) {
					for (int row = 0; row < neighborCells.size2(); ++row) {
						auto& neighbors = neighborCells(colunm, row);

						for (auto& neighbor : neighbors) {
							if (particle.index <= neighbor.index) continue;

							sf::Vector2f vector = neighbor.position - particle.position;
							float vectorLength = VectorFunctions::length(vector);

							if (vectorLength < _interactionRange) {
								float proximityCoefficient = 1 - vectorLength / _interactionRange;
								pairs.emplace_back(&particle, &neighbor, vector / vectorLength, proximityCoefficient);
							}
						}
					}
				}
			}
		}
	}

	void particlesGravity(float& interval) {
		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;
		
			float proximityCoefficient2 = pow(proximityCoefficient, 2);
			float proximityCoefficient3 = proximityCoefficient2 * proximityCoefficient;
			first->density += proximityCoefficient2;
			first->density_near += proximityCoefficient3;
			second->density += proximityCoefficient2;
			second->density_near += proximityCoefficient3;
		}
		
		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;
		
			float pressureM = k * (first->density - restDensity);
			float nearPressureM = k_near * first->density_near;
		
			sf::Vector2f pressure = float(
				interval *
				(pressureM * proximityCoefficient + nearPressureM * pow(proximityCoefficient, 2))) *
				normal;
		
			first->velosity -= pressure;
			second->velosity += pressure;
		}
	}

	void applyViscosity(float& interval) {
		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;

			float inertia = VectorFunctions::dotProduct(first->velosity - second->velosity, normal);
			if (inertia <= 0) continue;

			sf::Vector2f inertiaViscocity = float(
				0.5f * interval * proximityCoefficient *
				(kLinearViscocity * inertia + kQuadraticViscocity * pow(inertia, 2))) *
				normal;

			first->velosity -= inertiaViscocity;
			second->velosity += inertiaViscocity;
		}
	}

private:
	ParticleGrid& _particleGrid;
	
	struct PairData {
		PairData(Particle* first, Particle* second, const sf::Vector2f& normal, float proximityCoefficient)
			: first(first), second(second), normal(normal), proximityCoefficient(proximityCoefficient)
		{
		}

		Particle* first;
		Particle* second;
		sf::Vector2f normal;
		float proximityCoefficient;
	};
	std::vector<PairData> pairs;
	
	float _interactionRange;

	float restDensity = 50;
	float k = 15;
	float k_near = 800;
	float kLinearViscocity = 0.042;
	float kQuadraticViscocity = 0.5;
};