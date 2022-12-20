#include "PairCreator.h"
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <Utils/Algorithms.h>
#include <ParticleGrid.h>
#include <unordered_set>

PairCreator::PairCreator(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid)
{
	compute_dencity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/compute_dencity.comp" } });
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_pairs.comp" } });
}

void PairCreator::ComputePairs(float dt) {
	ComputeDencity();
	CreatePairs(dt);
}

void PairCreator::ComputeDencity() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	if (particles.empty()) return;

	glUseProgram(compute_dencity_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().threads_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CommonBuffers::GetInstance().particle_indexes->GetBufferId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config->GetBufferId());
	glUniform2i(1, _particle_grid.size.x, _particle_grid.size.y);

	glDispatchCompute(_particle_grid.particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	compute_dencity_program.Wait();
}

void PairCreator::CreatePairs(float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	if (particles.empty()) return;

	glUseProgram(create_pairs_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().threads_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CommonBuffers::GetInstance().particle_indexes->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, CommonBuffers::GetInstance().particle_threads->GetBufferId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config->GetBufferId());
	glUniform2i(1, _particle_grid.size.x, _particle_grid.size.y);
	glUniform1f(2, dt);

	glDispatchCompute(_particle_grid.particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	create_pairs_program.Wait();
}
