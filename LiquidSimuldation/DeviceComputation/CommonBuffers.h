#pragma once;
#include <Config.h>
#include <DeviceComputation/PairCreator.h>

struct CommonBuffers {
	static CommonBuffers& GetInstance() {
		static CommonBuffers common_buffers;
		return common_buffers;
	}

	const size_t max_pairs = 1000000;
	uint32_t config_buffer;
	uint32_t particles_buffer;
	uint32_t grid_buffer;
	uint32_t pairs_count_buffer;
	uint32_t pairs_buffer;

private:
	CommonBuffers() {
		glCreateBuffers(1, &config_buffer);
		glCreateBuffers(1, &particles_buffer);
		glCreateBuffers(1, &grid_buffer);
		glCreateBuffers(1, &pairs_count_buffer);
		glCreateBuffers(1, &pairs_buffer);

		glNamedBufferData(pairs_buffer, max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
		glNamedBufferData(config_buffer, sizeof(Config), &Config::GetInstance(), GL_DYNAMIC_DRAW);
	}
};
