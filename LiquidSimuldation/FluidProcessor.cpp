#include "FluidProcessor.h"

#include <math.h>
#include <Math/VectorFunctions.h>
#include <Config.h>

FluidProcessor::FluidProcessor(sf::Vector2i windowSize)
{
	_particle_grid.Init(windowSize);
}

void FluidProcessor::WallCollicionHandling(const std::vector<Wall>& walls, double interval) {
	NeatTimer::GetInstance().StageBegin(__func__);
	for (auto& particle : _particle_grid.particles) {
		for (auto& wall : walls) {
			// take out of loop
			auto wallVector = VectorFunctions::normalize(wall.a - wall.b);
			bool isClockwise = VectorFunctions::isClockwise(wall.a, wall.b, particle.position);
			auto wallPerp = (vector2)VectorFunctions::perpendicular(wallVector, isClockwise);

			auto particleWallVelosity = -VectorFunctions::dotProduct(particle.velosity, wallPerp);
			double distanse = VectorFunctions::linePointDistance(wall.a, wall.b, particle.position);

			float offset = 8;
			distanse -= particleWallVelosity * interval + particle.radius + offset;

			if (distanse < 0) {
				particle.position -= wallPerp * distanse;
				particle.velosity -= wallPerp * (double)VectorFunctions::dotProduct(particle.velosity, wallPerp) * 1.5;
			}
		}
	}
}

std::vector<PairData> FluidProcessor::CreatePairs() {
	NeatTimer::GetInstance().StageBegin(__func__);
	std::vector<PairData> result;

	for (size_t i = 0; i < _particle_grid.particles.size(); ++i) {
		auto& particle = _particle_grid.particles[i];
		auto neighbour_cells = _particle_grid.GetNeighbourIndexes(particle);

		for (auto& cell : neighbour_cells) {
			for (size_t j = cell.particles_start; j < cell.particles_end; ++j) {
				auto& neighbour = _particle_grid.particles[j];
				if (particle.index <= neighbour.index) continue;

				sf::Vector2f vector = neighbour.position - particle.position;
				float vectorLength = VectorFunctions::length(vector);

				if (vectorLength < Config::GetInstance().interactionRange) {
					float proximity_coefficient = 1 - vectorLength / Config::GetInstance().interactionRange;
					result.emplace_back(i, j, vector / vectorLength, proximity_coefficient);
				}
			}
		}
	}

	return result;
}

void FluidProcessor::ParticlesGravity(float& interval) {
	NeatTimer::GetInstance().StageBegin(__func__);
	for (auto& pair : pairs) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float proximityCoefficient2 = pow(pair.proximity_coefficient, 2);
		float proximityCoefficient3 = proximityCoefficient2 * pair.proximity_coefficient;
		first_particle.density += proximityCoefficient2;
		first_particle.density_near += proximityCoefficient3;
		second_particle.density += proximityCoefficient2;
		second_particle.density_near += proximityCoefficient3;
	}

	for (auto& pair : pairs) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float pressureM = Config::GetInstance().k * (first_particle.density - Config::GetInstance().restDensity + second_particle.density - Config::GetInstance().restDensity);
		float nearPressureM = Config::GetInstance().k_near * (first_particle.density_near + second_particle.density_near);

		vector2 pressure = float(
			interval *
			(pressureM * pair.proximity_coefficient + nearPressureM * pow(pair.proximity_coefficient, 2))) *
			pair.normal;

		first_particle.velosity -= pressure;
		second_particle.velosity += pressure;
	}
}

void FluidProcessor::ApplyViscosity(float& interval) {
	NeatTimer::GetInstance().StageBegin(__func__);
	for (auto& pair : pairs) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float inertia = VectorFunctions::dotProduct(first_particle.velosity - second_particle.velosity, pair.normal);
		if (inertia <= 0) continue;

		vector2 inertiaViscocity = float(
			0.5f * interval * pair.proximity_coefficient *
			(Config::GetInstance().kLinearViscocity * inertia + Config::GetInstance().kQuadraticViscocity * pow(inertia, 2))) *
			pair.normal;

		first_particle.velosity -= inertiaViscocity;
		second_particle.velosity += inertiaViscocity;
	}
}

void FluidProcessor::CreateParticle(vector2 position) {
	Particle particle(position);
	particle.acceleration = vector2(0, -200);
	_particle_grid.AddParticle(particle);
}

void FluidProcessor::Update(const std::vector<Wall>& walls, float dt) {
	for (auto& particle : _particle_grid.particles) {
		particle.density = 0.f;
		particle.density_near = 0.f;
	}

	_particle_grid.UpdateParticleNeighbours();
	WallCollicionHandling(walls, dt);

	pairs = DeviceFluidProcessor::GetInstance(_particle_grid).Update(dt);
	//pairs = gpu_compute.CreatePairs(_particle_grid);
	//pairs = createPairs();

	//applyViscosity(dt);
	//particlesGravity(dt);

	//gpu_compute.ParticleUpdate(_particle_grid, dt);
}

void FluidProcessor::Draw() {
	auto particle_shared_data = DataFactory<ElementSharedData<Particle>>::GetData();
	particle_shared_data->render_program.Use();
	glDrawElementsInstanced(GL_TRIANGLES, particle_shared_data->indexes.size(), GL_UNSIGNED_INT, 0, _particle_grid.particles.size());
}
