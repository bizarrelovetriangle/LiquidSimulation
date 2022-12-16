#include <FluidProcessor.h>
#include <math.h>
#include <Config.h>
#include <Math/vector2.h>

FluidProcessor::FluidProcessor(vector2i windowSize)
{
	_particle_grid.Init(windowSize);
}

void FluidProcessor::WallCollicionHandling(const std::vector<Wall>& walls, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);

	for (auto& wall : walls) {
		auto wallVector = (wall.a - wall.b).normalize();
		auto wall_center = (wall.a + wall.b) / 2;

		for (auto& particle : _particle_grid.particles) {
			float offset = 8;
			float max_dist = particle.radius + offset;
			float dist = particle.position.distance_to_line(wall.a, wall.b);

			if (dist > max_dist) continue;

			auto wallPerp = wallVector.is_clockwise(particle.position - wall_center)
				? wallVector.clockwise_perpendicular()
				: -wallVector.clockwise_perpendicular();
			particle.position += wallPerp * (max_dist - dist);

			auto particleWallVelosity = -wallPerp.dot_product(particle.velosity);
			if (particleWallVelosity < 0) continue;
			particle.velosity += wallPerp * particleWallVelosity * 1.5;
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

				vector2 vector = neighbour.position - particle.position;
				float vectorLength = vector.length();

				if (vectorLength < Config::GetInstance().interactionRange) {
					float proximity_coefficient = 1 - vectorLength / Config::GetInstance().interactionRange;
					result.emplace_back(i, j, vector / vectorLength, proximity_coefficient);
				}
			}
		}
	}

	return result;
}

void FluidProcessor::ParticlesGravity(float& dt) {
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

		vector2 pressure = pair.normal *
			dt * (pressureM * pair.proximity_coefficient + nearPressureM * powf(pair.proximity_coefficient, 2));

		first_particle.velosity -= pressure;
		second_particle.velosity += pressure;
	}
}

void FluidProcessor::ApplyViscosity(float& dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	for (auto& pair : pairs) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float inertia = pair.normal.dot_product(first_particle.velosity - second_particle.velosity);
		if (inertia <= 0) continue;

		vector2 inertiaViscocity = pair.normal *
			0.5f * dt * pair.proximity_coefficient *
			(Config::GetInstance().kLinearViscocity * inertia + Config::GetInstance().kQuadraticViscocity * powf(inertia, 2));

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
	_particle_grid.UpdateParticleNeighbours();
	WallCollicionHandling(walls, dt);
	DeviceFluidProcessor::GetInstance(_particle_grid).Update(dt);
}

void FluidProcessor::Draw() {
	auto particle_shared_data = DataFactory<ElementSharedData<Particle>>::GetData();
	particle_shared_data->render_program.Use();
	glDrawElementsInstanced(GL_TRIANGLES, particle_shared_data->indexes.size(), GL_UNSIGNED_INT, 0, _particle_grid.particles.size());
}
