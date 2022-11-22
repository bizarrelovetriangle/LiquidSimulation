#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <OpenGL/DeviceProgram/ComputeProgram.h>

class PairData;
class ParticleGrid;

class PairCreator {
public:
	PairCreator();
	void ComputePairs(ParticleGrid& particle_grid);

private:
	void CreatePairs(ParticleGrid& particle_grid);
	void SortPairs(ParticleGrid& particle_grid);

	void SortPairsBucketsCount(ParticleGrid& particle_grid);
	void GPUOneCoreSortPairs(ParticleGrid& particle_grid);

	int pairs_count = 0;

	ComputeProgram create_pairs_program;
	ComputeProgram sort_pairs_program;
	ComputeProgram sort_pairs_buckets_count_program;

	uint32_t pairs_temp_buffer;
	uint32_t buckets_count;
};

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