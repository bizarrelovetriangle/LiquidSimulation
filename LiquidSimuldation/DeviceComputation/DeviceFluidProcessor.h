#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2.h>
#include <OpenGL/DeviceBuffer.h>

class ParticleGrid;

class DeviceFluidProcessor {
private:
	DeviceFluidProcessor(ParticleGrid& particle_grid);

public:
	static DeviceFluidProcessor& GetInstance(ParticleGrid& particle_grid);
	void Update(float dt);

private:
	void CreateThreads();
	void ParticleThreadsUpdate();

	void ParticlesComputeDencity();
	void ParticlesComputeForce();

	const int parallel = 200;
	int threads_count = 0;

	ParticleGrid& _particle_grid;

	ComputeProgram create_threads_program;
	ComputeProgram thread_counts_program;
	ComputeProgram thread_offsets_program;
	ComputeProgram thread_offsets_skip_program;
	ComputeProgram thread_update_program;

	ComputeProgram compute_dencity_program;
	ComputeProgram particles_compute_force_program;

	DeviceBuffer<int> thread_counts;
	DeviceBuffer<int> thread_offsets;
};
