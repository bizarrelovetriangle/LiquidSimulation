#pragma once
#include <math.h>
#include "Particle.h"
#include "Wall.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"
#include "GPUCompute.h"
#include "Config.h"
#include "NeatTimer.h"

class FluidProcessor {
public:
	FluidProcessor(sf::Vector2i windowSize)
	{
		_particle_grid.Init(windowSize);
	}

	void wallCollicionHandling(const std::vector<Wall>& walls, double interval) {
		NeatTimer::GetInstance().StageBegin(__func__);
		for (auto& particle : _particle_grid.particles) {
			for (auto& wall : walls) {
				// take out of loop
				auto wallVector = VectorFunctions::normalize(wall.a - wall.b);
				bool isClockwise = VectorFunctions::isClockwise(wall.a, wall.b, particle.position);
				auto wallPerp = (vector2)VectorFunctions::perpendicular(wallVector, isClockwise);
					
				auto particleWallVelosity = -VectorFunctions::dotProduct(particle.velosity, wallPerp);
				double distanse = VectorFunctions::linePointDistance(wall.a, wall.b, particle.position);

				distanse -= particleWallVelosity * interval;

				if (distanse < particle.radius) {
					particle.position -= wallPerp * (distanse - particle.radius);
					particle.velosity -= wallPerp * (double)VectorFunctions::dotProduct(particle.velosity, wallPerp) * 1.5;
				}
			}
		}
	}

	std::vector<PairData> createPairs() {
		NeatTimer::GetInstance().StageBegin(__func__);
		std::vector<PairData> result;

		for (size_t i = 0; i < _particle_grid.particles.size(); ++i) {
			auto& particle = _particle_grid.particles[i];
			auto neighbour_cells = _particle_grid.getNeighbourIndexes(particle);

			for (auto& cell : neighbour_cells) {
				for (size_t j = cell.start; j < cell.end; ++j) {
					auto& neighbour = _particle_grid.particles[j];
					if (particle.index <= neighbour.index) continue;

					sf::Vector2f vector = neighbour.position - particle.position;
					float vectorLength = VectorFunctions::length(vector);

					if (vectorLength < Config::interactionRange) {
						float proximityCoefficient = 1 - vectorLength / Config::interactionRange;
						result.emplace_back(i, j, vector / vectorLength, proximityCoefficient);
					}
				}
			}
		}

		return result;
	}

	void particlesGravity(float& interval) {
		NeatTimer::GetInstance().StageBegin(__func__);
		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;
			auto& first_particle = _particle_grid.particles[first];
			auto& second_particle = _particle_grid.particles[second];

			float proximityCoefficient2 = pow(proximityCoefficient, 2);
			float proximityCoefficient3 = proximityCoefficient2 * proximityCoefficient;
			first_particle.density += proximityCoefficient2;
			first_particle.density_near += proximityCoefficient3;
			second_particle.density += proximityCoefficient2;
			second_particle.density_near += proximityCoefficient3;
		}

		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;
			auto& first_particle = _particle_grid.particles[first];
			auto& second_particle = _particle_grid.particles[second];

			float pressureM = Config::k * (first_particle.density - Config::restDensity + second_particle.density - Config::restDensity);
			float nearPressureM = Config::k_near * (first_particle.density_near + second_particle.density_near);

			vector2 pressure = float(
				interval *
				(pressureM * proximityCoefficient + nearPressureM * pow(proximityCoefficient, 2))) *
				normal;

			first_particle.velosity -= pressure;
			second_particle.velosity += pressure;
		}
	}

	void applyViscosity(float& interval) {
		NeatTimer::GetInstance().StageBegin(__func__);
		for (auto& pair : pairs) {
			auto& [first, second, normal, proximityCoefficient] = pair;
			auto& first_particle = _particle_grid.particles[first];
			auto& second_particle = _particle_grid.particles[second];

			float inertia = VectorFunctions::dotProduct(first_particle.velosity - second_particle.velosity, normal);
			if (inertia <= 0) continue;

			vector2 inertiaViscocity = float(
				0.5f * interval * proximityCoefficient *
				(Config::kLinearViscocity * inertia + Config::kQuadraticViscocity * pow(inertia, 2))) *
				normal;

			first_particle.velosity -= inertiaViscocity;
			second_particle.velosity += inertiaViscocity;
		}
	}

	void createParticle(vector2 position) {
		Particle particle(position);
		particle.acceleration = vector2(0, -200);
		_particle_grid.addParticle(particle);
	}

	void Update(const std::vector<Wall>& walls, float dt) {
		sf::Clock clock;

		for (auto& particle : _particle_grid.particles) {
			particle.density = 0.f;
			particle.density_near = 0.f;
		}

		_particle_grid.updateParticleNeighbours();
		wallCollicionHandling(walls, dt);
		pairs = gpu_compute.CreatePairs(_particle_grid);
		//pairs = createPairs();
		applyViscosity(dt);
		particlesGravity(dt);
		gpu_compute.ParticleUpdate(_particle_grid, dt);
	}

	void Draw() {
		for (auto& particle : _particle_grid.particles) {
			particle.draw();
		}
	}

private:
	GPUCompute gpu_compute;
	ParticleGrid _particle_grid;
	std::vector<PairData> pairs;
};