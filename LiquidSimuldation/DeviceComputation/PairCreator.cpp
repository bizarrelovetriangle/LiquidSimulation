#include "PairCreator.h"
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <Utils/Algorithms.h>
#include <ParticleGrid.h>
#include <unordered_set>

PairCreator::PairCreator()
{
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_pairs.comp" } });
	buckets_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/buckets_count.comp" } });
	buckets_distributed_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/buckets_distributed_count.comp" } });
	bucket_indexes_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/bucket_indexes_count.comp" } });
	bucket_indexes_distributed_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/bucket_indexes_distributed_count.comp" } });
	sort_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/radix_sort.comp" } });
	update_grid_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/update_grid.comp" } });

	glCreateBuffers(1, &pairs_temp_buffer);
	glCreateBuffers(1, &global_buckets_buffer);
	glCreateBuffers(1, &distributed_buckets_buffer);
	glCreateBuffers(1, &global_bucket_indexes_buffer);
	glCreateBuffers(1, &distributed_bucket_indexes_buffer);
	glCreateBuffers(1, &singular_buckets_buffer);

	glNamedBufferData(pairs_temp_buffer, CommonBuffers::GetInstance().max_pairs * sizeof(PairData), nullptr, GL_DYNAMIC_DRAW);
}

void PairCreator::ComputePairs(ParticleGrid& particle_grid) {
	CreatePairs(particle_grid);
	//SortPairs(particle_grid);
	BucketsCount(particle_grid);
	BucketIndexesCount(particle_grid);
	GPUOneCoreSortPairs(particle_grid);
	UpdateGrid(particle_grid);
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

void PairCreator::BucketsCount(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	size_t buckets_size = (1 << 8) * 4 * 2;
	std::vector<int> global_buckets(buckets_size, 0);
	glNamedBufferData(global_buckets_buffer, sizeof(int) * global_buckets.size(), &global_buckets[0], GL_DYNAMIC_DRAW);

	std::vector<int> distributed_buckets((1 << 8) * parallel, 0);
	glNamedBufferData(distributed_buckets_buffer, sizeof(int) * distributed_buckets.size(), &distributed_buckets[0], GL_DYNAMIC_DRAW);

	glUseProgram(buckets_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, global_buckets_buffer);
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(0, chunk_size);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	buckets_count_program.Wait();
}

void PairCreator::BucketIndexesCount(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	size_t buckets_size = (1 << 8) * 4 * 2;
	std::vector<int> global_bucket_indexes(buckets_size, 0);
	glNamedBufferData(global_bucket_indexes_buffer, sizeof(int) * global_bucket_indexes.size(), &global_bucket_indexes[0], GL_DYNAMIC_DRAW);

	std::vector<int> singular_buckets(4 * 2, 0);
	glNamedBufferData(singular_buckets_buffer, sizeof(int) * singular_buckets.size(), &singular_buckets[0], GL_DYNAMIC_DRAW);

	glUseProgram(bucket_indexes_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, global_buckets_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, global_bucket_indexes_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, singular_buckets_buffer);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	bucket_indexes_count_program.Wait();

}

void PairCreator::DistributedBucketsCount(ParticleGrid& particle_grid, int dimension, int byte) {
	NeatTimer::GetInstance().StageBegin(__func__);
	size_t buckets_size = (1 << 8) * 4 * 2;

	std::vector<int> distributed_buckets((1 << 8) * parallel, 0);
	glNamedBufferData(distributed_buckets_buffer, sizeof(int) * distributed_buckets.size(), &distributed_buckets[0], GL_DYNAMIC_DRAW);

	glUseProgram(buckets_distributed_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, distributed_buckets_buffer);
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(0, chunk_size);
	glUniform1i(1, dimension);
	glUniform1i(2, byte);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	buckets_distributed_count_program.Wait();
}

void PairCreator::DistributedBucketIndexesCount(ParticleGrid& particle_grid, int dimension, int byte) {
	std::vector<int> distributed_bucket_indexes((1 << 8) * parallel, 0);
	glNamedBufferData(distributed_bucket_indexes_buffer, sizeof(int) * distributed_bucket_indexes.size(), &distributed_bucket_indexes[0], GL_DYNAMIC_DRAW);

	glUseProgram(bucket_indexes_distributed_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, distributed_buckets_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, global_bucket_indexes_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, distributed_bucket_indexes_buffer);
	glUniform1i(0, parallel);
	glUniform1i(1, dimension);
	glUniform1i(2, byte);
	size_t _buckets_size_ = 1 << 8;
	glDispatchCompute(_buckets_size_, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	bucket_indexes_distributed_count_program.Wait();
}

void PairCreator::GPUOneCoreSortPairs(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);

	std::vector<int> singular_buckets(4 * 2);
	glGetNamedBufferSubData(singular_buckets_buffer, 0, sizeof(int) * singular_buckets.size(), &singular_buckets[0]);

	for (int dimension = 0; dimension < 2; ++dimension) {
		for (int byte = 0; byte < 4; ++byte) {
			if (singular_buckets[dimension * 4 + byte]) continue;
			DistributedBucketsCount(particle_grid, dimension, byte);
			DistributedBucketIndexesCount(particle_grid, dimension, byte);
			glUseProgram(sort_pairs_program.program_id);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pairs_temp_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, distributed_bucket_indexes_buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, distributed_buckets_buffer);
			glUniform2i(0, particle_grid.size.x, particle_grid.size.y);
			glUniform1i(1, dimension);
			glUniform1i(2, byte);
			int chunk_size = std::ceil(float(pairs_count) / parallel);
			glUniform1i(3, chunk_size);
			glDispatchCompute(parallel, 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			sort_pairs_program.Wait();
			std::swap(pairs_temp_buffer, CommonBuffers::GetInstance().pairs_buffer);
		}
	}
}

void PairCreator::UpdateGrid(ParticleGrid& particle_grid) {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(update_grid_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs_buffer);
	glUniform2i(0, particle_grid.size.x, particle_grid.size.y);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	update_grid_program.Wait();
}