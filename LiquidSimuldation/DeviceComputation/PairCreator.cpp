#include "PairCreator.h"
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <Utils/Algorithms.h>
#include <ParticleGrid.h>
#include <unordered_set>

PairCreator::PairCreator(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid)
{
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_pairs.comp" } });
	buckets_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/buckets_count.comp" } });
	bucket_indexes_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/bucket_indexes_count.comp" } });
	buckets_distributed_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/buckets_distributed_count.comp" } });
	bucket_indexes_distributed_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/bucket_indexes_distributed_count.comp" } });
	sort_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/radix_sort.comp" } });

	grid_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/grid_count.comp" } });
	grid_offsets_count_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/grid_offsets_count.comp" } });
	update_grid_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/update_grid.comp" } });

	size_t buckets_size = 1 << 8;
	size_t global_buckets_size = buckets_size * 4 * 2;
	size_t distributed_buckets_size = buckets_size * parallel;

	pairs_temp = std::make_unique<DeviceBuffer<PairData>>(CommonBuffers::GetInstance().max_pairs);
	global_buckets = std::make_unique<DeviceBuffer<int>>(global_buckets_size);
	global_bucket_indexes = std::make_unique<DeviceBuffer<int>>(global_buckets_size);
	distributed_buckets = std::make_unique<DeviceBuffer<int>>(distributed_buckets_size);
	distributed_bucket_indexes = std::make_unique<DeviceBuffer<int>>(distributed_buckets_size);
	singular_buckets = std::make_unique<DeviceBuffer<int>>(4 * 2);

	auto grid_size = _particle_grid.size;
	grid_counts = std::make_unique<DeviceBuffer<int>>(grid_size.x * grid_size.y);
	grid_column_counts = std::make_unique<DeviceBuffer<int>>(grid_size.y);
	grid_column_offsets = std::make_unique<DeviceBuffer<int>>(grid_size.y);
}

void PairCreator::ComputePairs() {
	CreatePairs();
	//SortPairs();
	BucketsCount();
	BucketIndexesCount();
	RadixSortPairs();
	
	GridCount();
	UpdateGrid();
}

void PairCreator::CreatePairs() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	if (particles.empty()) return;

	glUseProgram(create_pairs_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glUniform1i(0, _particle_grid.cellWidth);
	glUniform2i(1, _particle_grid.size.x, _particle_grid.size.y);

	glDispatchCompute(_particle_grid.size.x, _particle_grid.size.y, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	create_pairs_program.Wait();

	glGetNamedBufferSubData(CommonBuffers::GetInstance().pairs_count->GetBufferId(), 0, sizeof(int), &pairs_count);
}

void PairCreator::SortPairs() {
	NeatTimer::GetInstance().StageBegin(__func__);
	std::vector<PairData> pairs;
	pairs.resize(pairs_count);
	glGetNamedBufferSubData(CommonBuffers::GetInstance().pairs->GetBufferId(), 0, sizeof(PairData) * pairs.size(), &pairs[0]);

	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	auto get_grid_pos = [&](auto& pair) -> auto&& { return _particle_grid.particles[pair.first].gridPosition; };

	Algorithms::RadixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).x; });
	Algorithms::RadixSort(pairs, [&](auto& pair) -> auto&& { return get_grid_pos(pair).y; });

	for (size_t i = 0; i < _particle_grid.size.y * _particle_grid.size.x; ++i) {
		grid[i].pairs_start = 0;
		grid[i].pairs_end = 0;
	}

	sf::Vector2i last_position = get_grid_pos(pairs[0]);
	for (size_t i = 1; i < pairs.size(); ++i) {
		auto& pair = pairs[i];
		auto& grid_position = get_grid_pos(pair);
		if (last_position == grid_position) continue;

		_particle_grid.GetGridCell(last_position).pairs_end = i;
		_particle_grid.GetGridCell(grid_position).pairs_start = i;
		last_position = grid_position;
	}
	_particle_grid.GetGridCell(last_position).pairs_end = pairs.size();

	glNamedBufferSubData(CommonBuffers::GetInstance().pairs->GetBufferId(), 0, pairs.size() * sizeof(PairData), &pairs[0]);
	glNamedBufferData(CommonBuffers::GetInstance().grid->GetBufferId(), grid.size() * sizeof(ParticleGrid::GridType), &grid[0], GL_DYNAMIC_DRAW);
}

void PairCreator::BucketsCount() {
	NeatTimer::GetInstance().StageBegin(__func__);
	global_buckets->FlushData();

	glUseProgram(buckets_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, global_buckets->GetBufferId());
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(0, chunk_size);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	buckets_count_program.Wait();
}

void PairCreator::BucketIndexesCount() {
	NeatTimer::GetInstance().StageBegin(__func__);
	global_bucket_indexes->FlushData();
	singular_buckets->FlushData();

	glUseProgram(bucket_indexes_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, global_buckets->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, global_bucket_indexes->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, singular_buckets->GetBufferId());
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	bucket_indexes_count_program.Wait();
}

void PairCreator::DistributedBucketsCount(int dimension, int byte) {
	NeatTimer::GetInstance().StageBegin(__func__);
	distributed_buckets->FlushData();
	glUseProgram(buckets_distributed_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, distributed_buckets->GetBufferId());
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(0, chunk_size);
	glUniform1i(1, dimension);
	glUniform1i(2, byte);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	buckets_distributed_count_program.Wait();
}

void PairCreator::DistributedBucketIndexesCount(int dimension, int byte) {
	NeatTimer::GetInstance().StageBegin(__func__);

	distributed_bucket_indexes->FlushData();

	glUseProgram(bucket_indexes_distributed_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, distributed_buckets->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, global_bucket_indexes->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, distributed_bucket_indexes->GetBufferId());
	glUniform1i(0, parallel);
	glUniform1i(1, dimension);
	glUniform1i(2, byte);
	size_t _buckets_size_ = 1 << 8;
	glDispatchCompute(_buckets_size_, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	bucket_indexes_distributed_count_program.Wait();
}

void PairCreator::RadixSortPairs() {
	NeatTimer::GetInstance().StageBegin(__func__);
	singular_buckets->RetriveData();
	auto& singular_buckets_data = singular_buckets->GetData();

	for (int dimension = 0; dimension < 2; ++dimension) {
		for (int byte = 0; byte < 4; ++byte) {
			if (singular_buckets_data[dimension * 4 + byte]) continue;
			DistributedBucketsCount(dimension, byte);
			DistributedBucketIndexesCount(dimension, byte);

			NeatTimer::GetInstance().StageBegin(__func__);
			glUseProgram(sort_pairs_program.program_id);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs_count->GetBufferId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs->GetBufferId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, pairs_temp->GetBufferId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, distributed_bucket_indexes->GetBufferId());
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, distributed_buckets->GetBufferId());
			glUniform1i(1, dimension);
			glUniform1i(2, byte);
			int chunk_size = std::ceil(float(pairs_count) / parallel);
			glUniform1i(3, chunk_size);
			glDispatchCompute(parallel, 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			sort_pairs_program.Wait();
			std::swap(pairs_temp->GetBufferId(), CommonBuffers::GetInstance().pairs->GetBufferId());
		}
	}

	for (auto& bucket : singular_buckets_data) bucket = 0;
	singular_buckets->FlushData();
}

void PairCreator::GridCount() {
	NeatTimer::GetInstance().StageBegin(__func__);
	grid_counts->FlushData();
	grid_column_counts->FlushData();
	grid_column_offsets->FlushData();

	glUseProgram(grid_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, CommonBuffers::GetInstance().pairs_count->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, CommonBuffers::GetInstance().pairs->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, grid_counts->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, grid_column_counts->GetBufferId());
	glUniform2i(0, _particle_grid.size.x, _particle_grid.size.y);
	int chunk_size = std::ceil(float(pairs_count) / parallel);
	glUniform1i(1, chunk_size);
	glDispatchCompute(parallel, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	grid_count_program.Wait();

	glUseProgram(grid_offsets_count_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, grid_column_counts->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_column_offsets->GetBufferId());
	glUniform2i(0, _particle_grid.size.x, _particle_grid.size.y);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	grid_offsets_count_program.Wait();
}

void PairCreator::UpdateGrid() {
	NeatTimer::GetInstance().StageBegin(__func__);
	glUseProgram(update_grid_program.program_id);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().grid->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, grid_counts->GetBufferId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, grid_column_offsets->GetBufferId());
	glUniform2i(0, _particle_grid.size.x, _particle_grid.size.y);
	glDispatchCompute(_particle_grid.size.y, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	update_grid_program.Wait();
}
