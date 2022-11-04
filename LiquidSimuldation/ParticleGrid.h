#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <math.h>
#include <span>
#include "Config.h"

using namespace boost::numeric::ublas;

class ParticleGrid {
public:
	struct GridCell {
		bool empty() {
			return start == end;
		}

		int start;
		int end;
	};

	using GridType = std::vector<GridCell>;

	void Init(sf::Vector2i windowSize) {
		cellWidth = Config::interactionRange;
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
		auto it = std::remove_if(std::begin(particles), std::end(particles),
			[this](auto& particle) { return isOutsideWindow(particle); });
		particles.erase(it, std::end(particles));

		auto indexGrid = std::vector<std::vector<std::vector<size_t>>>(
			size.y, std::vector<std::vector<size_t>>(size.x));

		for (size_t i = 0; i < particles.size(); ++i) {
			sf::Vector2i gridPosition = getGridPosition(particles[i]);
			particles[i].gridPosition = gridPosition;
			indexGrid[gridPosition.y][gridPosition.x].push_back(i);
		}

		std::vector<Particle> particle_temp;
		particle_temp.reserve(particles.size());

		for (size_t y = 0; y < size.y; ++y) {
			for (size_t x = 0; x < size.x; ++x) {
				auto& indexes = indexGrid[y][x];
				auto& cell = GetGridCell(y, x);
				cell.start = particle_temp.size();
				cell.end = particle_temp.size() + indexes.size();
				for (auto index : indexes) {
					particle_temp.push_back(particles[index]);
				}
			}
		}

		std::swap(particle_temp, particles);
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
				auto& cell = GetGridCell(y, x);
				if (!cell.empty()) {
					auto start_it = std::next(std::begin(particles), cell.start);
					auto end_it = std::next(std::begin(particles), cell.end);
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
				auto& cell = GetGridCell(y, x);
				if (!cell.empty()) {
					result.emplace_back(cell);
				}
			}
		}

		return result;
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

	GridCell& GetGridCell(size_t y, size_t x) {
		return grid[y * size.x + x];
	}
};