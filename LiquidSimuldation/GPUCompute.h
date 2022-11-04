#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "ComputeProgram.h"
#include "NeatTimer.h"

struct alignas(8) PairData {
	PairData(int first, int second, const sf::Vector2f& normal, float proximityCoefficient)
		: first(first), second(second), normal(normal), proximityCoefficient(proximityCoefficient)
	{}

	int first;
	int second;
	sf::Vector2f normal;
	float proximityCoefficient;
};

class GPUCompute {
public:
	GPUCompute()
	{
		particle_update_program.InitProgram({{ GL_COMPUTE_SHADER, "Shaders/compute.comp" }});
		create_pairs_program.InitProgram({{ GL_COMPUTE_SHADER, "Shaders/create_pairs.comp" }});

		glCreateBuffers(1, &particles_buffer);
		glCreateBuffers(1, &grid_buffer);
		glCreateBuffers(1, &pairs_buffer);

		size_t max_pairs = 1000000;
		glNamedBufferData(pairs_buffer, sizeof(int) + max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
	}

	void ParticleUpdate(ParticleGrid& particle_grid, float dt) {
		NeatTimer::GetInstance().StageBegin(__func__);
		auto& particles = particle_grid.particles;
		if (particles.empty()) return;
		
		glNamedBufferData(particles_buffer, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);

		glUseProgram(particle_update_program.program_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
		glUniform1f(0, dt);

		glDispatchCompute(particles.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		auto data = (Particle*)glMapNamedBuffer(particles_buffer, GL_READ_WRITE);
		std::vector<Particle> readData(data, data + particles.size());
		glUnmapNamedBuffer(particles_buffer);
		particles = readData;
	}

	std::vector<PairData> CreatePairs(ParticleGrid& particle_grid) {
		NeatTimer::GetInstance().StageBegin(__func__);
		auto& particles = particle_grid.particles;
		auto& grid = particle_grid.grid;
		if (particles.empty()) return {};

		glNamedBufferData(particles_buffer, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);
		glNamedBufferData(grid_buffer, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
		int pairs_count = 0;
		glNamedBufferSubData(pairs_buffer, 0, sizeof(int), &pairs_count);

		glUseProgram(create_pairs_program.program_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pairs_buffer);
		glUniform1i(0, particle_grid.cellWidth);
		glUniform2i(1, particle_grid.size.x, particle_grid.size.y);

		glDispatchCompute(particle_grid.size.x, particle_grid.size.y, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		auto data = (PairsOutput*)glMapNamedBuffer(pairs_buffer, GL_READ_WRITE);
		pairs_count = data->pairs_count[0];
		std::vector<PairData> result(data->pairs, data->pairs + pairs_count);
		glUnmapNamedBuffer(pairs_buffer);

		return result;
	}

private:
	ComputeProgram particle_update_program;
	ComputeProgram create_pairs_program;
	uint32_t particles_buffer;
	uint32_t grid_buffer;
	uint32_t pairs_buffer;

	struct PairsOutput {
		int pairs_count[2];
		PairData pairs[];
	};
};
