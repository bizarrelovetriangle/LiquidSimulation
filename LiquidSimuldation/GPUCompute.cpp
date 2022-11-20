#pragma once
#include "GPUCompute.h"
#include "ParticleGrid.h"

namespace
{
	void Wait() {
		auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		GLenum waitReturn = GL_UNSIGNALED;

		while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED) {
			waitReturn = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
		}
	}
}

GPUCompute::GPUCompute()
{
	particle_update_program.InitProgram({ { GL_COMPUTE_SHADER, "Shaders/particles_update.comp" } });
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "Shaders/create_particle_pairs.comp" } });
	particle_viscosity_program.InitProgram({ { GL_COMPUTE_SHADER, "Shaders/particles_viscosity.comp" } });
	particle_density_program.InitProgram({ { GL_COMPUTE_SHADER, "Shaders/particles_density.comp" } });
	particle_gravity_program.InitProgram({ { GL_COMPUTE_SHADER, "Shaders/particles_gravity.comp" } });

	glCreateBuffers(1, &config_buffer);
	glCreateBuffers(1, &particles_buffer);
	glCreateBuffers(1, &grid_buffer);
	glCreateBuffers(1, &pairs_buffer);

	size_t max_pairs = 1000000;
	glNamedBufferData(pairs_buffer, sizeof(int) * 2 + max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
	glNamedBufferData(config_buffer, sizeof(Config), &Config::GetInstance(), GL_DYNAMIC_DRAW);
}

GPUCompute& GPUCompute::GetInstance() {
	static GPUCompute gpu_compute;
	return gpu_compute;
}

void GPUCompute::ParticleUpdate(ParticleGrid& particle_grid, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	if (particles.empty()) return;

	glUseProgram(particle_update_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
	glUniform1f(0, dt);

	glDispatchCompute(particles.size(), 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Wait();
}

void GPUCompute::CreatePairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;
	if (particles.empty()) return;

	glUseProgram(create_pairs_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pairs_buffer);
	glUniform1i(0, particle_grid.cellWidth);
	glUniform2i(1, particle_grid.size.x, particle_grid.size.y);

	glDispatchCompute(particle_grid.size.x, particle_grid.size.y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	Wait();
}

void GPUCompute::SortPairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	//NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");
	int pairs_count;
	glGetNamedBufferSubData(pairs_buffer, 0, sizeof(int), &pairs_count);
	std::vector<PairData> pairs(pairs_count);
	glGetNamedBufferSubData(pairs_buffer, sizeof(int) * 2, sizeof(PairData) * pairs.size(), &pairs[0]);

	//NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;
	auto get_grid_pos = [&](auto& pair) -> auto&& { return particle_grid.particles[pair.first].gridPosition; };

	Algorithms::RedixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).x; });
	Algorithms::RedixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).y; });

	for (size_t i = 0; i < particle_grid.size.y * particle_grid.size.x; ++i) {
		grid[i].pairs_start = 0;
		grid[i].pairs_end = 0;
	}

	sf::Vector2i last_position = get_grid_pos(pairs[0]);
	for (size_t i = 1; i < pairs.size(); ++i) {
		auto& pair = pairs[i];
		auto& grid_position = get_grid_pos(pair);
		if (last_position == grid_position) continue;

		particle_grid.GetGridCell(last_position).pairs_end = i;
		particle_grid.GetGridCell(grid_position).pairs_start = i;
		last_position = grid_position;
	}
	particle_grid.GetGridCell(last_position).pairs_end = pairs.size();

	//NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - write data");
	glNamedBufferSubData(pairs_buffer, sizeof(int) * 2, pairs.size() * sizeof(PairData), &pairs[0]);
	glNamedBufferData(grid_buffer, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
}

void GPUCompute::GranularProcessPairs(const GPUProgramBase& program, ParticleGrid& particle_grid, float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;

	glUseProgram(program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pairs_buffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, config_buffer);
	glUniform1f(1, dt);
	glUniform2i(2, particle_grid.size.x, particle_grid.size.y);

	for (size_t y = 0; y < 3; ++y) {
		for (size_t x = 0; x < 3; ++x) {
			sf::Vector2i offset(x, y);
			sf::Vector2i compute_plane = (particle_grid.size - offset) / 3;
			glUniform2i(3, offset.x, offset.y);
			glDispatchCompute(compute_plane.x, compute_plane.y, 1);
		}
	}

	Wait();
}

std::vector<PairData> GPUCompute::Update(ParticleGrid& particle_grid, float dt) {
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;

	glNamedBufferData(particles_buffer, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);
	glNamedBufferData(grid_buffer, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
	int pairs_count = 0;
	glNamedBufferSubData(pairs_buffer, 0, sizeof(int), &pairs_count);

	CreatePairs(particle_grid);
	SortPairs(particle_grid);

	GranularProcessPairs(particle_viscosity_program, particle_grid, dt);
	GranularProcessPairs(particle_density_program, particle_grid, dt);
	GranularProcessPairs(particle_gravity_program, particle_grid, dt);
	ParticleUpdate(particle_grid, dt);

	NeatTimer::GetInstance().StageBegin(std::string(__func__) + " - read data");
	
	glGetNamedBufferSubData(particles_buffer, 0, sizeof(Particle) * particles.size(), &particles[0]);

	return {};
}

uint32_t GPUCompute::GetParticlesBuffer() {
	return particles_buffer;
}