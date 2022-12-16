#pragma once
#include "DeviceFluidProcessor.h"
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

DeviceFluidProcessor::DeviceFluidProcessor(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid), _pair_creator(particle_grid)
{
	CommonBuffers::GetInstance().config->Flush({ Config::GetInstance() });
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_update.comp" } });
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

void DeviceFluidProcessor::Update(float dt) {
	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	int pairs_count = 0;

	CommonBuffers::GetInstance().particles->Flush(particles);
	CommonBuffers::GetInstance().grid->Flush(grid);
	CommonBuffers::GetInstance().pairs_count->Flush({ pairs_count });

	_pair_creator.ComputePairs(dt);
	ParticleUpdate(dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");

	glGetNamedBufferSubData(CommonBuffers::GetInstance().particles->GetBufferId(), 0, sizeof(Particle) * particles.size(), &particles[0]);
}