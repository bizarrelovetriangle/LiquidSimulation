#pragma once
#include "DeviceFluidProcessor.h"
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

DeviceFluidProcessor::DeviceFluidProcessor()
{
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_update.comp" } });
	particle_viscosity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_viscosity.comp" } });
	particle_density_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_density.comp" } });
	particle_gravity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_gravity.comp" } });
}

DeviceFluidProcessor& DeviceFluidProcessor::GetInstance() {
	static DeviceFluidProcessor gpu_compute;
	return gpu_compute;
}

void DeviceFluidProcessor::ParticleUpdate(ParticleGrid& particle_grid, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	if (particles.empty()) return;

	glUseProgram(particle_update_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glUniform1f(0, dt);

	glDispatchCompute(particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	particle_update_program.Wait();
}

void DeviceFluidProcessor::GranularProcessPairs(const ComputeProgram& program, ParticleGrid& particle_grid, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config_buffer);
	glUniform1f(1, dt);
	glUniform2i(2, particle_grid.size.x, particle_grid.size.y);

	for (size_t y = 0; y < 2; ++y) {
		for (size_t x = 0; x < 2; ++x) {
			sf::Vector2i offset(x, y);
			sf::Vector2i compute_plane = (particle_grid.size - offset) / 2;
			glUniform2i(3, offset.x, offset.y);
			glDispatchCompute(compute_plane.x, compute_plane.y, 1);
		}
	}

	program.Wait();
}

std::vector<PairData> DeviceFluidProcessor::Update(ParticleGrid& particle_grid, float dt) {
	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;

	glNamedBufferData(CommonBuffers::GetInstance().particles_buffer, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);
	glNamedBufferData(CommonBuffers::GetInstance().grid_buffer, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
	int pairs_count = 0;
	glNamedBufferData(CommonBuffers::GetInstance().pairs_count_buffer, sizeof(int), &pairs_count, GL_DYNAMIC_DRAW);

	_pair_creator.ComputePairs(particle_grid);

	GranularProcessPairs(particle_viscosity_program, particle_grid, dt);
	GranularProcessPairs(particle_density_program, particle_grid, dt);
	GranularProcessPairs(particle_gravity_program, particle_grid, dt);
	ParticleUpdate(particle_grid, dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");

	glGetNamedBufferSubData(CommonBuffers::GetInstance().particles_buffer, 0, sizeof(Particle) * particles.size(), &particles[0]);

	return {};
}