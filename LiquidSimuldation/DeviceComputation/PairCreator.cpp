#include "PairCreator.h"
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <Utils/Algorithms.h>
#include <ParticleGrid.h>

PairCreator::PairCreator()
{
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_pairs.comp" } });
	sort_pairs_buckets_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/buckets_count.comp" } });
	sort_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/radix_sort.comp" } });

	glCreateBuffers(1, &pairs_temp_buffer);
	glCreateBuffers(1, &buckets_count);

	glNamedBufferData(pairs_temp_buffer, CommonBuffers::GetInstance().max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
}

void PairCreator::ComputePairs(ParticleGrid& particle_grid) {
	CreatePairs(particle_grid);
	//SortPairs(particle_grid);
	//SortPairsBucketsCount(particle_grid);
	GPUOneCoreSortPairs(particle_grid);
}

void PairCreator::CreatePairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;
	if (particles.empty()) return;

	glUseProgram(create_pairs_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_buffer);
	glUniform1i(0, particle_grid.cellWidth);
	glUniform2i(1, particle_grid.size.x, particle_grid.size.y);

	glDispatchCompute(particle_grid.size.x, particle_grid.size.y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	create_pairs_program.Wait();

	glGetNamedBufferSubData(CommonBuffers::GetInstance().pairs_count_buffer, 0, sizeof(int), &pairs_count);
}

void PairCreator::SortPairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	std::vector<PairData> pairs;
	pairs.resize(pairs_count);
	glGetNamedBufferSubData(CommonBuffers::GetInstance().pairs_buffer, 0, sizeof(PairData) * pairs.size(), &pairs[0]);

	auto& particles = particle_grid.particles;
	auto& grid = particle_grid.grid;
	auto get_grid_pos = [&](auto& pair) -> auto&& { return particle_grid.particles[pair.first].gridPosition; };

	Algorithms::RadixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).x; });
	Algorithms::RadixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).y; });

	for (size_t i = 0; i < particle_grid.size.y * particle_grid.size.x; ++i) {
		grid[i].pairs_start = 0;
		grid[i].pairs_end = 0;
	}

	sf::Vector2i last_position = get_grid_pos(pairs[0]);
	for (size_t i = 1; i < pairs.size(); ++i) {
		auto& pair = pairs[i];
		auto& grid_position = get_grid_pos(pair);
		if (last_position == grid_position) continue;

		particle_grid.GetGridCell(last_position).pairs_end = i;
		particle_grid.GetGridCell(grid_position).pairs_start = i;
		last_position = grid_position;
	}
	particle_grid.GetGridCell(last_position).pairs_end = pairs.size();

	glNamedBufferSubData(CommonBuffers::GetInstance().pairs_buffer, 0, pairs.size() * sizeof(PairData), &pairs[0]);
	glNamedBufferData(CommonBuffers::GetInstance().grid_buffer, grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
}

void PairCreator::SortPairsBucketsCount(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	size_t buckets_size = (1 << 8) * 4 * 2;
	std::vector<int> buckets(buckets_size, 0);
	glNamedBufferData(buckets_count, sizeof(int) * buckets.size(), &buckets[0], GL_DYNAMIC_DRAW);

	glUseProgram(sort_pairs_buckets_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, buckets_count);
	int parallel = 200;
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(0, chunk_size);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	sort_pairs_buckets_count_program.Wait();
}


void PairCreator::GPUOneCoreSortPairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(sort_pairs_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pairs_temp_buffer);
	glUniform2i(0, particle_grid.size.x, particle_grid.size.y);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	sort_pairs_program.Wait();
}