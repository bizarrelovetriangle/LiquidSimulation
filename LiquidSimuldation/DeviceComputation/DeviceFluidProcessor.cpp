#pragma once
#include "DeviceFluidProcessor.h"
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

DeviceFluidProcessor::DeviceFluidProcessor(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid), _pair_creator(particle_grid)
{
	CommonBuffers::GetInstance().config->Flush({ Config::GetInstance() });
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_update.comp" } });
	create_threads_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_threads.comp" } });
	particle_thread_counts_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/particle_thread_counts.comp" } });
	particle_thread_offsets_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/particle_thread_offsets.comp" } });
	particle_thread_offsets_skip_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/particle_thread_offsets_skip.comp" } });
	particle_thread_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/particle_thread_update.comp" } });

	int threads_count = 0;
	CommonBuffers::GetInstance().threads_count->Flush({ threads_count });
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CommonBuffers::GetInstance().particle_indexes->GetBufferId());
	glUniform1f(0, dt);

	glDispatchCompute(particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	particle_update_program.Wait();
}

void DeviceFluidProcessor::CreateThreads() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	if (particles.empty()) return;

	glUseProgram(create_threads_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().threads_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, CommonBuffers::GetInstance().particle_indexes->GetBufferId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config->GetBufferId());
	glUniform2i(1, _particle_grid.size.x, _particle_grid.size.y);

	glDispatchCompute(particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	create_threads_program.Wait();

	glGetNamedBufferSubData(CommonBuffers::GetInstance().threads_count->GetBufferId(), 0, sizeof(int), &threads_count);
}

void DeviceFluidProcessor::ParticleThreadsCount() {
	NeatTimer::GetInstance().StageBegin(__func__);
	particle_thread_counts.Clear();
	particle_thread_offsets.Clear();

	glUseProgram(particle_thread_counts_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().threads_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particle_thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_thread_offsets.GetBufferId());
	glUniform2i(0, _particle_grid.size.x, _particle_grid.size.y);
	glUniform1i(2, parallel);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(particle_thread_offsets_skip_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_thread_offsets.GetBufferId());
	glUniform1i(2, parallel);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(particle_thread_offsets_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().particle_threads->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particle_thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_thread_offsets.GetBufferId());
	glUniform1i(2, parallel);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	particle_thread_offsets_program.Wait();
}

void DeviceFluidProcessor::ParticleThreadsUpdate() {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(particle_thread_update_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().threads_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_temp->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, CommonBuffers::GetInstance().threads_count_temp->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particle_thread_offsets.GetBufferId());
	glUniform1i(2, parallel);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	particle_thread_update_program.Wait();

	std::swap(CommonBuffers::GetInstance().pairs->GetBufferId(), CommonBuffers::GetInstance().pairs_temp->GetBufferId());
	std::swap(CommonBuffers::GetInstance().threads_count->GetBufferId(), CommonBuffers::GetInstance().threads_count_temp->GetBufferId());
	CommonBuffers::GetInstance().threads_count_temp->Flush({0});

	//auto data = CommonBuffers::GetInstance().particle_threads->Retrive();
	//size_t size = CommonBuffers::GetInstance().threads_count->Retrive().front();
	//if (!size) return;
	//auto particles = CommonBuffers::GetInstance().particles->Retrive();
	//auto threads = CommonBuffers::GetInstance().pairs->Retrive(size);

}

void DeviceFluidProcessor::Update(float dt) {
	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	auto& particles = _particle_grid.particles;
	auto& particle_indexes = _particle_grid.particle_indexes;
	auto& grid = _particle_grid.grid;

	CommonBuffers::GetInstance().particles->Flush(particles);
	CommonBuffers::GetInstance().particle_indexes->Flush(particle_indexes);
	CommonBuffers::GetInstance().grid->Flush(grid);
	CommonBuffers::GetInstance().particle_threads->Resize(particles.size());

	particle_thread_counts.Resize(particle_indexes.size());
	particle_thread_offsets.Resize(particle_indexes.size());

	CreateThreads();
	ParticleThreadsCount();
	ParticleThreadsUpdate();

	_pair_creator.ComputePairs(dt);
	ParticleUpdate(dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");

 	glGetNamedBufferSubData(CommonBuffers::GetInstance().particles->GetBufferId(), 0, sizeof(Particle) * particles.size(), &particles[0]);
}