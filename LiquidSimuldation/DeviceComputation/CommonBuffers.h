#pragma once;
#include <Config.h>
#include <OpenGl/DeviceBuffer.h>
#include <Elements/Particle.h>
#include <ParticleGrid.h>
#include <Elements/ParticlesThread.h>

struct CommonBuffers {
	static CommonBuffers& GetInstance() {
		static CommonBuffers common_buffers;
		return common_buffers;
	}

	const size_t max_threads = 1000000;
	DeviceBuffer<Config> config;
	DeviceBuffer<Particle> particles;
	DeviceBuffer<int> particle_indexes;
	DeviceBuffer<ParticleGrid::GridCell> grid;
	DeviceBuffer<int> threads_count;
	DeviceBuffer<int> threads_count_temp;
	DeviceBuffer<ParticleGrid::GridCell> particle_threads;
	DeviceBuffer<ParticlesThread> threads;
	DeviceBuffer<ParticlesThread> threads_temp;
	DeviceBuffer<int> threads_torn;

private:
	CommonBuffers() {
		config = DeviceBuffer<Config>(1);
		particles = DeviceBuffer<Particle>(0);
		particle_indexes = DeviceBuffer<int>(0);
		grid = DeviceBuffer<ParticleGrid::GridCell>(0);
		threads_count = DeviceBuffer<int>(1);
		threads_count_temp = DeviceBuffer<int>(1);
		particle_threads = DeviceBuffer<ParticleGrid::GridCell>(0);
		threads = DeviceBuffer<ParticlesThread>(max_threads);
		threads_temp = DeviceBuffer<ParticlesThread>(max_threads);
		threads_torn = DeviceBuffer<int>(1);
	}
};
