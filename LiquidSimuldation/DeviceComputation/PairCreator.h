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

	void BucketsCount(ParticleGrid& particle_grid);
	void BucketIndexesCount(ParticleGrid& particle_grid);
	void DistributedBucketsCount(ParticleGrid& particle_grid, int dimension, int byte);
	void DistributedBucketIndexesCount(ParticleGrid& particle_grid, int dimension, int byte);
	void RadixSortPairs(ParticleGrid& particle_grid);

	void GridCount(ParticleGrid& particle_grid);
	void UpdateGrid(ParticleGrid& particle_grid);

	const int parallel = 200;
	int pairs_count = 0;

	ComputeProgram create_pairs_program;
	ComputeProgram sort_pairs_program;
	ComputeProgram buckets_count_program;
	ComputeProgram buckets_distributed_count_program;
	ComputeProgram bucket_indexes_count_program;
	ComputeProgram bucket_indexes_distributed_count_program;

	ComputeProgram grid_count_program;
	ComputeProgram grid_offsets_count_program;
	ComputeProgram update_grid_program;

	uint32_t pairs_temp_buffer;
	uint32_t global_buckets_buffer;
	uint32_t global_bucket_indexes_buffer;
	uint32_t distributed_buckets_buffer;
	uint32_t distributed_bucket_indexes_buffer;
	uint32_t singular_buckets_buffer;

	uint32_t grid_counts_buffer;
	uint32_t grid_column_counts_buffer;
	uint32_t grid_column_offsets_buffer;
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