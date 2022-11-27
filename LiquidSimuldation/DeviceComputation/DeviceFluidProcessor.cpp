#pragma once
#include "DeviceFluidProcessor.h"
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

DeviceFluidProcessor::DeviceFluidProcessor(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid), _pair_creator(particle_grid)
{
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_update.comp" } });
	particle_viscosity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_viscosity.comp" } });
	particle_density_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_density.comp" } });
	particle_gravity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_gravity.comp" } });
}

DeviceFluidProcessor& DeviceFluidProcessor::GetInstance(ParticleGrid& particle_grid) {
	static DeviceFluidProcessor gpu_compute(particle_grid);
	return gpu_compute;
}

void DeviceFluidProcessor::ParticleUpdate(float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	if (particles.empty()) return;

	glUseProgram(particle_update_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glUniform1f(0, dt);

	glDispatchCompute(particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	particle_update_program.Wait();
}

void DeviceFluidProcessor::GranularProcessPairs(const ComputeProgram& program, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config->GetBufferId());
	glUniform1f(1, dt);
	glUniform2i(2, _particle_grid.size.x, _particle_grid.size.y);

	for (size_t y = 0; y < 2; ++y) {
		for (size_t x = 0; x < 2; ++x) {
			sf::Vector2i offset(x, y);
			sf::Vector2i compute_plane = (_particle_grid.size - offset) / 2;
			glUniform2i(3, offset.x, offset.y);
			glDispatchCompute(compute_plane.x, compute_plane.y, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}
	}

	program.Wait();
}

std::vector<PairData> DeviceFluidProcessor::Update(float dt) {
	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	int pairs_count = 0;

	CommonBuffers::GetInstance().particles->FlushData(particles);
	CommonBuffers::GetInstance().grid->FlushData(grid);
	CommonBuffers::GetInstance().pairs_count->FlushData({ pairs_count });

	_pair_creator.ComputePairs();

	GranularProcessPairs(particle_viscosity_program, dt);
	GranularProcessPairs(particle_density_program, dt);
	GranularProcessPairs(particle_gravity_program, dt);
	ParticleUpdate(dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");

	glGetNamedBufferSubData(CommonBuffers::GetInstance().particles->GetBufferId(), 0, sizeof(Particle) * particles.size(), &particles[0]);

	return {};
}