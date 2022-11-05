#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <math.h>
#include <span>
#include "Config.h"
#include "NeatTimer.h"
#include "Algorithms.h"

using namespace boost::numeric::ublas;

class ParticleGrid {
public:
	struct GridCell {
		bool empty() {
			return particles_start == particles_end;
		}

		int particles_start;
		int particles_end;

		int pairs_start;
		int pairs_end;
	};

	using GridType = std::vector<GridCell>;

	void Init(sf::Vector2i windowSize) {
		cellWidth = Config::GetInstance().interactionRange;
		_windowSize = windowSize;
		_windowStart = -windowSize / 2;
		size.x = (_windowSize.x / cellWidth) + 1;
		size.y = (_windowSize.y / cellWidth) + 1;
		grid = GridType(size.y * size.x);
	}

	void addParticle(const Particle& particle) {
		if (!isOutsideWindow(particle)) {
			particles.push_back(particle);
		}
	}

	void updateParticleNeighbours() {
		NeatTimer::GetInstance().StageBegin(__func__);
		auto it = std::remove_if(std::begin(particles), std::end(particles),
			[this](auto& particle) { return isOutsideWindow(particle); });
		particles.erase(it, std::end(particles));

		if (particles.empty()) return;

		for (auto& particle : particles) {
			particle.gridPosition = getGridPosition(particle);
		}

		for (size_t i = 0; i < size.y * size.x; ++i) {
			grid[i].particles_start = 0;
			grid[i].particles_end = 0;
		}

		Algorithms::RedixSort(particles, [](auto& obj) -> auto&& { return obj.gridPosition.x; });
		Algorithms::RedixSort(particles, [](auto& obj) -> auto&& { return obj.gridPosition.y; });

		sf::Vector2i last_position = particles[0].gridPosition;
		for (size_t i = 1; i < particles.size(); ++i) {
			auto& particle = particles[i];
			if (last_position == particle.gridPosition) continue;

			GetGridCell(last_position).particles_end = i;
			GetGridCell(particle.gridPosition).particles_start = i;
			last_position = particle.gridPosition;
		}
		GetGridCell(last_position).particles_end = particles.size();
	}

	std::vector<std::span<Particle>> getNeighbours(const Particle& particle) {
		sf::Vector2i gridPosition = getGridPosition(particle);

		sf::Vector2i range_a = gridPosition - sf::Vector2i(1, 1);
		sf::Vector2i range_b = gridPosition + sf::Vector2i(1, 1);

		range_a.x = std::clamp(range_a.x, 0, size.x - 1);
		range_a.y = std::clamp(range_a.y, 0, size.y - 1);
		range_b.x = std::clamp(range_b.x, 0, size.x - 1);
		range_b.y = std::clamp(range_b.y, 0, size.y - 1);

		std::vector<std::span<Particle>> result;

		for (size_t y = range_a.y; y <= range_b.y; ++y) {
			for (size_t x = range_a.x; x <= range_b.x; ++x) {
				auto& cell = grid[y * size.x + x];
				if (!cell.empty()) {
					auto start_it = std::next(std::begin(particles), cell.particles_start);
					auto end_it = std::next(std::begin(particles), cell.particles_end);
					result.emplace_back(start_it, end_it);
				}
			}
		}

		return result;
	}

	std::vector<GridCell> getNeighbourIndexes(const Particle& particle) {
		sf::Vector2i range_a = particle.gridPosition - sf::Vector2i(1, 1);
		sf::Vector2i range_b = particle.gridPosition + sf::Vector2i(1, 1);

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

	GridCell& GetGridCell(const sf::Vector2i& grid_position) {
		return grid[grid_position.y * size.x + grid_position.x];
	}

	std::vector<Particle> particles;
	GridType grid;

	int cellWidth;
	sf::Vector2i size;

private:
	sf::Vector2i _windowSize;
	sf::Vector2i _windowStart;

	sf::Vector2i getGridPosition(const Particle& particle) {
		return (sf::Vector2i(particle.position) + _windowSize / 2) / cellWidth;
	}

	bool isOutsideWindow(const Particle& particle) {
		return 
			isnan(particle.position.x) || isnan(particle.position.y) ||
			isinf(particle.position.x) || isinf(particle.position.y) ||
			particle.position.x < _windowStart.x || particle.position.x > - _windowStart.x ||
			particle.position.y < _windowStart.y || particle.position.y > - _windowStart.y;
	}
};