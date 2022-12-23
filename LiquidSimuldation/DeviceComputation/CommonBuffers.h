#pragma once;
#include <Config.h>
#include <DeviceComputation/PairCreator.h>
#include <OpenGl/DeviceBuffer.h>
#include <Elements/Particle.h>
#include <ParticleGrid.h>

struct CommonBuffers {
	static CommonBuffers& GetInstance() {
		static CommonBuffers common_buffers;
		return common_buffers;
	}

	const size_t max_pairs = 1000000;
	std::unique_ptr<DeviceBuffer<Config>> config;
	std::unique_ptr<DeviceBuffer<Particle>> particles;
	std::unique_ptr<DeviceBuffer<int>> particle_indexes;
	std::unique_ptr<DeviceBuffer<ParticleGrid::GridCell>> grid;
	std::unique_ptr<DeviceBuffer<int>> threads_count;
	std::unique_ptr<DeviceBuffer<int>> threads_count_temp;
	std::unique_ptr<DeviceBuffer<ParticleGrid::GridCell>> particle_threads;
	std::unique_ptr<DeviceBuffer<PairData>> pairs;
	std::unique_ptr<DeviceBuffer<PairData>> pairs_temp;

private:
	CommonBuffers() {
		config = std::make_unique<DeviceBuffer<Config>>(1);
		particles = std::make_unique<DeviceBuffer<Particle>>(0);
		particle_indexes = std::make_unique<DeviceBuffer<int>>(0);
		grid = std::make_unique<DeviceBuffer<ParticleGrid::GridCell>>(0);
		threads_count = std::make_unique<DeviceBuffer<int>>(1);
		threads_count_temp = std::make_unique<DeviceBuffer<int>>(1);
		particle_threads = std::make_unique<DeviceBuffer<ParticleGrid::GridCell>>(0);
		pairs = std::make_unique<DeviceBuffer<PairData>>(max_pairs);
		pairs_temp = std::make_unique<DeviceBuffer<PairData>>(max_pairs);
	}
};
