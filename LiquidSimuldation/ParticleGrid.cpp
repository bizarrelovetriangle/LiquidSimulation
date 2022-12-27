#include "ParticleGrid.h"
#include <Math/vector2i.h>

void ParticleGrid::Init(vector2i windowSize) {
	cellWidth = Config::GetInstance().interactionRange;
	_windowSize = windowSize;
	_windowStart = -windowSize / 2;
	size.x = (_windowSize.x / cellWidth) + 1;
	size.y = (_windowSize.y / cellWidth) + 1;
	grid = std::vector<GridCell>(size.y * size.x);
}

void ParticleGrid::AddParticle(const Particle& particle) {
	if (!IsOutsideWindow(particle)) {
		particles.push_back(particle);
		particle_indexes.push_back(particles.size() - 1);
	}
}

void ParticleGrid::UpdateParticleNeighbours() {
	NeatTimer::GetInstance().StageBegin(__func__);

	auto it = std::partition(std::begin(particle_indexes), std::end(particle_indexes),
		[this](int index) { return !IsOutsideWindow(particles[index]); });
	std::for_each(it, std::end(particle_indexes),
		[this](int index) { particles[index].state = Particle::State::Deactive; });
	particle_indexes.erase(it, std::end(particle_indexes));

	if (particles.empty()) return;

	for (auto index : particle_indexes) {
		auto& particle = particles[index];
		particle.gridPosition = GetGridPosition(particle);
	}

	std::vector<int> grid_counts(grid.size());
	for (auto index : particle_indexes) {
		auto& particle = particles[index];
		int grid_index = particle.gridPosition.y * size.x + particle.gridPosition.x;
		++grid_counts[grid_index];
	}

	grid[0].start = 0;
	grid[0].end = grid_counts[0];
	for (size_t i = 1; i < grid.size(); ++i) {
		grid[i].start = grid[i - 1].end;
		grid[i].end = grid[i].start + grid_counts[i];
	}

	auto grid_temp = grid;

	for (auto index : std::vector<int>(particle_indexes)) {
		auto& particle = particles[index];
		int grid_index = particle.gridPosition.y * size.x + particle.gridPosition.x;
		int bucket = grid_temp[grid_index].start++;
		particle_indexes[bucket] = index;
	}
}

std::vector<std::span<Particle>> ParticleGrid::GetNeighbours(const Particle& particle) {
	vector2i gridPosition = GetGridPosition(particle);

	vector2i range_a = gridPosition - vector2i(1, 1);
	vector2i range_b = gridPosition + vector2i(1, 1);

	range_a.x = std::clamp(range_a.x, 0, size.x - 1);
	range_a.y = std::clamp(range_a.y, 0, size.y - 1);
	range_b.x = std::clamp(range_b.x, 0, size.x - 1);
	range_b.y = std::clamp(range_b.y, 0, size.y - 1);

	std::vector<std::span<Particle>> result;

	for (size_t y = range_a.y; y <= range_b.y; ++y) {
		for (size_t x = range_a.x; x <= range_b.x; ++x) {
			auto& cell = grid[y * size.x + x];
			if (!cell.empty()) {
				auto start_it = std::next(std::begin(particles), cell.start);
				auto end_it = std::next(std::begin(particles), cell.end);
				result.emplace_back(start_it, end_it);
			}
		}
	}

	return result;
}

std::vector<ParticleGrid::GridCell> ParticleGrid::GetNeighbourIndexes(const Particle& particle) {
	vector2i range_a = particle.gridPosition - vector2i(1, 1);
	vector2i range_b = particle.gridPosition + vector2i(1, 1);

	range_a.x = std::clamp(range_a.x, 0, size.x - 1);
	range_a.y = std::clamp(range_a.y, 0, size.y - 1);
	range_b.x = std::clamp(range_b.x, 0, size.x - 1);
	range_b.y = std::clamp(range_b.y, 0, size.y - 1);

	std::vector<GridCell> result;

	for (size_t y = range_a.y; y <= range_b.y; ++y) {
		for (size_t x = range_a.x; x <= range_b.x; ++x) {
			auto& cell = grid[y * size.x + x];
			if (!cell.empty()) {
				result.emplace_back(cell);
			}
		}
	}

	return result;
}

ParticleGrid::GridCell& ParticleGrid::GetGridCell(const vector2i& grid_position) {
	return grid[grid_position.y * size.x + grid_position.x];
}

vector2i ParticleGrid::GetGridPosition(const Particle& particle) {
	return (vector2i(particle.position) + _windowSize / 2) / cellWidth;
}

bool ParticleGrid::IsOutsideWindow(const Particle& particle) {
	return
		isnan(particle.position.x) || isnan(particle.position.y) ||
		isinf(particle.position.x) || isinf(particle.position.y) ||
		particle.position.x < _windowStart.x || particle.position.x > -_windowStart.x ||
		particle.position.y < _windowStart.y || particle.position.y > -_windowStart.y;
}