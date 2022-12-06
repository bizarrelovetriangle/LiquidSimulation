#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <OpenGL/DeviceBuffer.h>

class PairData;
class ParticleGrid;

class PairCreator {
public:
	PairCreator(ParticleGrid& particle_grid);
	void ComputePairs();

private:
	void CreatePairs();
	void SortPairs();

	void BucketsCount();
	void BucketIndexesCount();
	void DistributedBucketsCount(int dimension, int byte);
	void DistributedBucketIndexesCount(int dimension, int byte);
	void RadixSortPairs();

	void GridCount();
	void UpdateGrid();

	const int parallel = 200;
	int pairs_count = 0;

	ParticleGrid& _particle_grid;

	ComputeProgram create_pairs_program;
	ComputeProgram sort_pairs_program;
	ComputeProgram buckets_count_program;
	ComputeProgram buckets_distributed_count_program;
	ComputeProgram bucket_indexes_count_program;
	ComputeProgram bucket_indexes_distributed_count_program;

	ComputeProgram grid_count_program;
	ComputeProgram grid_offsets_count_program;
	ComputeProgram update_grid_program;

	std::unique_ptr<DeviceBuffer<PairData>> pairs_temp;
	std::unique_ptr<DeviceBuffer<int>> global_buckets;
	std::unique_ptr<DeviceBuffer<int>> global_bucket_indexes;
	std::unique_ptr<DeviceBuffer<int>> distributed_buckets;
	std::unique_ptr<DeviceBuffer<int>> distributed_bucket_indexes;
	std::unique_ptr<DeviceBuffer<int>> singular_buckets;

	std::unique_ptr<DeviceBuffer<int>> grid_counts;
	std::unique_ptr<DeviceBuffer<int>> grid_column_counts;
	std::unique_ptr<DeviceBuffer<int>> grid_column_offsets;
};

struct alignas(8) PairData {
	PairData(int first, int second, const sf::Vector2f& normal, float proximity_coefficient)
		: first(first), second(second), normal(normal), proximity_coefficient(proximity_coefficient)
	{}

	PairData() {}

	int first;
	int second;
	sf::Vector2f normal;
	float proximity_coefficient;
	float proximity_coefficient_2;
	float proximity_coefficient_3;
	sf::Vector2f viscosity_inertia;
	sf::Vector2i grid_position;
};