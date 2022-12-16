#pragma once
#include <Math/vector2.h>
#include <vector>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <OpenGL/DeviceBuffer.h>

class PairData;
class ParticleGrid;

class PairCreator {
public:
	PairCreator(ParticleGrid& particle_grid);
	void ComputePairs(float dt);

private:
	void ComputeDencity();
	void CreatePairs(float dt);

	const int parallel = 200;
	int pairs_count = 0;

	ParticleGrid& _particle_grid;

	ComputeProgram compute_dencity_program;
	ComputeProgram create_pairs_program;
};

struct alignas(8) PairData {
	PairData(int first, int second, const vector2& normal, float proximity_coefficient)
		: first(first), second(second), normal(normal), proximity_coefficient(proximity_coefficient)
	{}

	PairData() {}

	int first;
	int second;
	vector2 normal;
	float proximity_coefficient;
};