#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2.h>

class ParticleGrid;

struct alignas(8) PairData {
	PairData(int first, int second, const sf::Vector2f& normal, float proximityCoefficient)
		: first(first), second(second), normal(normal), proximityCoefficient(proximityCoefficient)
	{}

	PairData() {}

	int first;
	int second;
	sf::Vector2f normal;
	float proximityCoefficient;
};

class DeviceFluidProcessor {
private:
	DeviceFluidProcessor();
public:
	static DeviceFluidProcessor& GetInstance();
	void ParticleUpdate(ParticleGrid& particle_grid, float dt);
	void CreatePairs(ParticleGrid& particle_grid);
	void SortPairs(ParticleGrid& particle_grid);

	void SortPairsBucketsCount(ParticleGrid& particle_grid);
	void GPUOneCoreSortPairs(ParticleGrid& particle_grid);

	void GranularProcessPairs(const DeviceProgram& program, ParticleGrid& particle_grid, float dt);
	std::vector<PairData> Update(ParticleGrid& particle_grid, float dt);
	uint32_t GetParticlesBuffer();
private:
	ComputeProgram create_pairs_program;

	ComputeProgram sort_pairs_program;
	ComputeProgram sort_pairs_buckets_count_program;

	ComputeProgram particle_viscosity_program;
	ComputeProgram particle_update_program;
	ComputeProgram particle_density_program;
	ComputeProgram particle_gravity_program;

	uint32_t config_buffer;
	uint32_t particles_buffer;
	uint32_t grid_buffer;
	uint32_t pairs_count_buffer;
	uint32_t pairs_buffer;
	uint32_t pairs_temp_buffer;
	uint32_t buckets_count;

	std::vector<PairData> pairs;
};
