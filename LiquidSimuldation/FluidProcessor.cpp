#include <FluidProcessor.h>
#include <math.h>
#include <Config.h>
#include <Math/vector2.h>
#include <OpenGL/DeviceProgram/DeviceProgram.h>
#define _USE_MATH_DEFINES
#include <math.h>

FluidProcessor::FluidProcessor(vector2i windowSize)
{
	_particle_grid.Init(windowSize);
	DeviceProgram::_particle_grid = &_particle_grid;
}

void FluidProcessor::WallCollicionHandling(const std::vector<Wall>& walls, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);

	for (auto& wall : walls) {
		auto wall_vector = (wall.a() - wall.b()).normalize();
		auto wall_length = (wall.a() - wall.b()).length();
		auto wall_center = (wall.a() + wall.b()) / 2;

		for (auto& particle : _particle_grid.particles) {
			float max_dist = 10;

			auto wall_perp = wall_vector.is_clockwise(particle.position - wall_center)
				? wall_vector.clockwise_perpendicular()
				: -wall_vector.clockwise_perpendicular();

			float dist = particle.position.distance_to_line(wall.a(), wall.b());

			if (dist < max_dist) {
				particle.position += wall_perp * (max_dist - dist);
			}

			auto particle_wall_proj = wall_vector * wall_vector.dot_product(particle.position - wall_center);
			if (particle_wall_proj.length() > wall_length / 2) continue;
			auto rotate_vector = wall.rotate_speed > 0
				? -particle_wall_proj.clockwise_perpendicular()
				: particle_wall_proj.clockwise_perpendicular();
			auto wall_velocity = rotate_vector * wall.rotate_speed;

			float particle_wall_approximation = wall_perp.dot_product(wall_velocity - particle.velosity);

			if (dist - particle_wall_approximation * dt < max_dist && particle_wall_approximation > 0) {
				particle.velosity += wall_perp * particle_wall_approximation * 1.5;
			}
		}
	}
}

std::vector<ParticlesThread> FluidProcessor::Createthreads() {
	NeatTimer::GetInstance().StageBegin(__func__);
	std::vector<ParticlesThread> result;

	for (size_t i = 0; i < _particle_grid.particles.size(); ++i) {
		auto& particle = _particle_grid.particles[i];
		auto neighbour_cells = _particle_grid.GetNeighbourIndexes(particle);

		for (auto& cell : neighbour_cells) {
			for (size_t j = cell.start; j < cell.end; ++j) {
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
	for (auto& pair : threads) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float proximity_coefficient = 0;

		float proximityCoefficient2 = pow(proximity_coefficient, 2);
		float proximityCoefficient3 = proximityCoefficient2 * proximity_coefficient;
		first_particle.density += proximityCoefficient2;
		first_particle.density_near += proximityCoefficient3;
		second_particle.density += proximityCoefficient2;
		second_particle.density_near += proximityCoefficient3;
	}

	for (auto& pair : threads) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float pressureM = Config::GetInstance().k * (first_particle.density - Config::GetInstance().restDensity + second_particle.density - Config::GetInstance().restDensity);
		float nearPressureM = Config::GetInstance().k_near * (first_particle.density_near + second_particle.density_near);
		
		float proximity_coefficient = 0;
		vector2 normal;

		vector2 pressure = normal *
			dt * (pressureM * proximity_coefficient + nearPressureM * powf(proximity_coefficient, 2));

		first_particle.velosity -= pressure;
		second_particle.velosity += pressure;
	}
}

void FluidProcessor::ApplyViscosity(float& dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	for (auto& pair : threads) {
		auto& first_particle = _particle_grid.particles[pair.first];
		auto& second_particle = _particle_grid.particles[pair.second];

		float proximity_coefficient = 0;
		vector2 normal;

		float inertia = normal.dot_product(first_particle.velosity - second_particle.velosity);
		if (inertia <= 0) continue;

		vector2 inertiaViscocity = normal *
			0.5f * dt * proximity_coefficient *
			(Config::GetInstance().kLinearViscocity * inertia + Config::GetInstance().kQuadraticViscocity * powf(inertia, 2));

		first_particle.velosity -= inertiaViscocity;
		second_particle.velosity += inertiaViscocity;
	}
}

void FluidProcessor::CreateParticle(vector2 position) {
	Particle particle(position);
	particle.external_force = vector2(0, -200);
	_particle_grid.AddParticle(particle);
}

void FluidProcessor::Update(const std::vector<Wall>& walls, float dt) {
	_particle_grid.UpdateParticleNeighbours();
	WallCollicionHandling(walls, dt);

	if (_particle_grid.particle_indexes.size() == 0) return;
	DeviceFluidProcessor::GetInstance(_particle_grid).Update(dt);
}

void FluidProcessor::Draw() {
	if (_particle_grid.particle_indexes.size() == 0) return;

	auto particle_shared_data = DataFactory<ElementSharedData<Particle>>::GetData();
	particle_shared_data->render_program.Use();
	glDrawElementsInstanced(GL_TRIANGLES, particle_shared_data->indexes.size(), GL_UNSIGNED_INT, 0, _particle_grid.particle_indexes.size());
}
