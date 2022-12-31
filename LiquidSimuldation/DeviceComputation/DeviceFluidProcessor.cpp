#pragma once
#include "DeviceFluidProcessor.h"
#include <ParticleGrid.h>
#include <DeviceComputation/CommonBuffers.h>

DeviceFluidProcessor::DeviceFluidProcessor(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid)
{
	CommonBuffers::GetInstance().config.Flush({ Config::GetInstance() });
	create_threads_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_threads/create_threads.comp" } });
	thread_counts_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_threads/thread_counts.comp" } });
	thread_offsets_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_threads/thread_offsets.comp" } });
	thread_offsets_skip_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_threads/thread_offsets_skip.comp" } });
	thread_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_threads/thread_update.comp" } });

	particles_applied_force_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_applied_force.comp" } });
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particles/particles_update.comp" } });

	CommonBuffers::GetInstance().threads_count.Flush({ 0 });
	CommonBuffers::GetInstance().threads_torn.Flush({ 0 });
}

DeviceFluidProcessor& DeviceFluidProcessor::GetInstance(ParticleGrid& particle_grid) {
	static DeviceFluidProcessor gpu_compute(particle_grid);
	return gpu_compute;
}

void DeviceFluidProcessor::CreateThreads() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particle_indexes = _particle_grid.particle_indexes;
	if (particle_indexes.empty()) return;

	create_threads_program.Use();
	glDispatchCompute(particle_indexes.size(), 1, 1);
	create_threads_program.Wait();
	threads_count = CommonBuffers::GetInstance().threads_count.Retrive().front();
}

void DeviceFluidProcessor::ParticleThreadsCount() {
	NeatTimer::GetInstance().StageBegin(__func__);
	thread_counts.Clear();
	thread_offsets.Clear();

	thread_counts_program.Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, thread_offsets.GetBufferId());
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	thread_offsets_skip_program.Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, thread_offsets.GetBufferId());
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	thread_offsets_program.Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, thread_offsets.GetBufferId());
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	thread_offsets_program.Wait();
}

void DeviceFluidProcessor::ParticleThreadsUpdate() {
	NeatTimer::GetInstance().StageBegin(__func__);
	thread_update_program.Use();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, thread_counts.GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, thread_offsets.GetBufferId());
	glDispatchCompute(parallel, 1, 1);
	thread_update_program.Wait();

	std::swap(CommonBuffers::GetInstance().threads.GetBufferId(), CommonBuffers::GetInstance().threads_temp.GetBufferId());
	std::swap(CommonBuffers::GetInstance().threads_count.GetBufferId(), CommonBuffers::GetInstance().threads_count_temp.GetBufferId());
	CommonBuffers::GetInstance().threads_count_temp.Flush({ 0 });
}

void DeviceFluidProcessor::ParticlesAppliedForce() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particle_indexes = _particle_grid.particle_indexes;
	auto& grid = _particle_grid.grid;
	if (particle_indexes.empty()) return;

	particles_applied_force_program.Use();
	glDispatchCompute(_particle_grid.particle_indexes.size(), 1, 1);
	particles_applied_force_program.Wait();
}

void DeviceFluidProcessor::ParticleUpdate(float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particle_indexes = _particle_grid.particle_indexes;
	if (particle_indexes.empty()) return;

	particle_update_program.Use();
	glUniform1f(0, dt);
	glDispatchCompute(particle_indexes.size(), 1, 1);
	particle_update_program.Wait();
}

void DeviceFluidProcessor::Update(float dt) {
	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	auto& particles = _particle_grid.particles;
	auto& particle_indexes = _particle_grid.particle_indexes;
	auto& grid = _particle_grid.grid;

	CommonBuffers::GetInstance().particles.Flush(particles);
	CommonBuffers::GetInstance().particle_indexes.Flush(particle_indexes);
	CommonBuffers::GetInstance().grid.Flush(grid);

	CommonBuffers::GetInstance().particle_threads.Resize(particles.size());
	thread_counts.Resize(particles.size());
	thread_offsets.Resize(particles.size());

	CreateThreads();

	constexpr int threads_torn_threshold = 1000;
	bool remove_torn = CommonBuffers::GetInstance().threads_torn.Retrive().front() > threads_torn_threshold;

	if (_particle_grid.particles_updated || remove_torn) {
		ParticleThreadsCount();
		ParticleThreadsUpdate();
		_particle_grid.particles_updated = false;
		CommonBuffers::GetInstance().threads_torn.Flush({ 0 });
	}

	ParticlesAppliedForce();
	ParticleUpdate(dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");

 	glGetNamedBufferSubData(CommonBuffers::GetInstance().particles.GetBufferId(), 0, sizeof(Particle) * particles.size(), &particles[0]);
}