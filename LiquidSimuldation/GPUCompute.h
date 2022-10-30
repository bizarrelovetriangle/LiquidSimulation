#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "ComputeProgram.h"

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
		particle_update_program.InitProgram({{ GL_COMPUTE_SHADER, "Shaders/compute.glsl" }});
		create_pairs_program.InitProgram({{ GL_COMPUTE_SHADER, "Shaders/create_pairs.glsl" }});

		glGenBuffers(1, &particles_buffer);
		glGenBuffers(1, &grid_buffer);
		glGenBuffers(1, &pairs_buffer);
	}

	void ParticleUpdate(ParticleGrid& particle_grid, float dt) {
		auto& particles = particle_grid.particles;
		if (particles.empty()) return;

		glUseProgram(particle_update_program.program_id);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);
		glUniform1f(0, dt);

		glDispatchCompute(particles.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		auto data = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
		std::vector<Particle> readData(data, data + particles.size());
		particles = readData;
	}

	std::vector<PairData> CreatePairs(ParticleGrid& particle_grid) {
		auto& particles = particle_grid.particles;
		auto& grid = particle_grid.grid;
		size_t max_pairs = 1000000;

		if (particles.empty()) return {};

		glUseProgram(create_pairs_program.program_id);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particles_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle), &particles[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pairs_buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) + max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
		int pairs_count = 0;
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &pairs_count);

		glUniform1i(0, particle_grid._cellWidth);
		glUniform1i(1, particle_grid._gridColumns);
		glUniform1i(2, particle_grid._gridRows);

		glDispatchCompute(particles.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, pairs_buffer);

		struct PairsOutput {
			int pairs_count[2];
			PairData pairs[];
		};

		auto data = (PairsOutput*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
		pairs_count = data->pairs_count[0];
		std::vector<PairData> result(data->pairs, data->pairs + pairs_count);

		return result;
	}

private:
	ComputeProgram particle_update_program;
	ComputeProgram create_pairs_program;
	uint32_t particles_buffer;
	uint32_t grid_buffer;
	uint32_t pairs_buffer;
};
