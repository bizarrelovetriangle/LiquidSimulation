#pragma once
#include <math.h>
#include "Particle.h"
#include "Wall.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"
#include "GPUCompute.h"

class FluidProcessor {
public:
	FluidProcessor(sf::Vector2i windowSize)
	{
		_particle_grid.Init(windowSize, _interactionRange);
	}

	void wallCollicionHandling(const std::vector<Wall>& walls, double interval) {
		for (auto& particle : _particle_grid.particles) {
			for (auto& wall : walls) {
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

	void createPairs() {
		pairs.clear();

		for (auto& particle : _particle_grid.particles) {
			particle.density = 0.f;
			particle.density_near = 0.f;
			auto neighbour_cells = _particle_grid.getNeighbours(particle);

			for (auto& neighbours : neighbour_cells) {
				for (auto& neighbour : neighbours) {
					if (particle.index <= neighbour.index) continue;

					sf::Vector2f vector = neighbour.position - particle.position;
					float vectorLength = VectorFunctions::length(vector);

					if (vectorLength < _interactionRange) {
						float proximityCoefficient = 1 - vectorLength / _interactionRange;
						pairs.emplace_back(&particle, &neighbour, vector / vectorLength, proximityCoefficient);
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
		
			vector2 pressure = float(
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

			vector2 inertiaViscocity = float(
				0.5f * interval * proximityCoefficient *
				(kLinearViscocity * inertia + kQuadraticViscocity * pow(inertia, 2))) *
				normal;

			first->velosity -= inertiaViscocity;
			second->velosity += inertiaViscocity;
		}
	}

	void createParticle(vector2 position) {
		Particle particle(position);
		particle.acceleration = vector2(0, -200);
		_particle_grid.addParticle(particle);
	}

	void Update(const std::vector<Wall>& walls, float dt) {
		sf::Clock clock;

		_particle_grid.updateParticleNeighbours();
		float updateParticleNeighbours = clock.restart().asSeconds();

		wallCollicionHandling(walls, dt);
		float wallCollicionHandling = clock.restart().asSeconds();

		createPairs();
		float createPairs = clock.restart().asSeconds();

		applyViscosity(dt);
		float applyViscosity = clock.restart().asSeconds();

		particlesGravity(dt);
		float particlesGravity = clock.restart().asSeconds();

		//gpu_compute.Run(_particle_grid, dt);

		for (auto& particle : _particle_grid.particles) {
			particle.update(dt);
		}
		float particlesUpdate = clock.restart().asSeconds();

		float overall = updateParticleNeighbours + wallCollicionHandling + createPairs + applyViscosity + particlesGravity + particlesUpdate;

		static size_t counter = 0;
		if (counter++ % 100 == 0) {
			std::cout <<
				"updateParticleNeighbours: '" + std::to_string(updateParticleNeighbours / overall) + "'," << std::endl <<
				"wallCollicionHandling: '" + std::to_string(wallCollicionHandling / overall) + "'," << std::endl <<
				"createPairs: '" + std::to_string(createPairs / overall) + "'," << std::endl <<
				"applyViscosity: '" + std::to_string(applyViscosity / overall) + "'," << std::endl <<
				"particlesGravity: '" + std::to_string(particlesGravity / overall) + "'," << std::endl <<
				"particlesUpdate: '" + std::to_string(particlesUpdate / overall) + "'," << std::endl << std::endl;
		}
	}

	void Draw() {
		for (auto& particle : _particle_grid.particles) {
			particle.draw();
		}
	}

private:
	GPUCompute gpu_compute;
	ParticleGrid _particle_grid;
	
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
	
	float _interactionRange = 30;

	float restDensity = 50;
	float k = 15;
	float k_near = 800;
	float kLinearViscocity = 0.042;
	float kQuadraticViscocity = 0.05;
};