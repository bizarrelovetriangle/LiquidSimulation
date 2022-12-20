#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2.h>
#include <DeviceComputation/PairCreator.h>

class ParticleGrid;

class DeviceFluidProcessor {
private:
	DeviceFluidProcessor(ParticleGrid& particle_grid);

public:
	static DeviceFluidProcessor& GetInstance(ParticleGrid& particle_grid);
	void Update(float dt);

private:
	void ParticleUpdate(float dt);
	void CreateThreads();
	void ParticleThreadsCount();
	void ParticleThreadsUpdate();

	const int parallel = 200;
	int threads_count = 0;

	ParticleGrid& _particle_grid;
	PairCreator _pair_creator;

	ComputeProgram particle_update_program;
	ComputeProgram create_threads_program;
	ComputeProgram particle_thread_counts_program;
	ComputeProgram particle_thread_offsets_program;
	ComputeProgram particle_thread_offsets_skip_program;
	ComputeProgram particle_thread_update_program;

	DeviceBuffer<int> particle_thread_counts;
	DeviceBuffer<int> particle_thread_offsets;
};
