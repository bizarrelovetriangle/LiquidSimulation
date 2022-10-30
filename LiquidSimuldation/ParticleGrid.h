#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <math.h>
#include <span>

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

	void Init(sf::Vector2i windowSize, float cellWidth) {
		_cellWidth = cellWidth;
		_windowSize = windowSize;
		_windowStart = -windowSize / 2;
		_gridColumns = (_windowSize.x / _cellWidth) + 1;
		_gridRows = (_windowSize.y / _cellWidth) + 1;
		grid = GridType(_gridRows * _gridColumns);
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
			_gridRows, std::vector<std::vector<size_t>>(_gridColumns));

		for (size_t i = 0; i < particles.size(); ++i) {
			sf::Vector2i gridPosition = getGridPosition(particles[i]);
			particles[i].gridPosition = gridPosition;
			indexGrid[gridPosition.y][gridPosition.x].push_back(i);
		}

		std::vector<Particle> particle_temp;
		particle_temp.reserve(particles.size());

		for (size_t y = 0; y < _gridRows; ++y) {
			for (size_t x = 0; x < _gridColumns; ++x) {
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

		range_a.x = std::clamp(range_a.x, 0, _gridColumns - 1);
		range_a.y = std::clamp(range_a.y, 0, _gridRows - 1);
		range_b.x = std::clamp(range_b.x, 0, _gridColumns - 1);
		range_b.y = std::clamp(range_b.y, 0, _gridRows - 1);

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

		range_a.x = std::clamp(range_a.x, 0, _gridColumns - 1);
		range_a.y = std::clamp(range_a.y, 0, _gridRows - 1);
		range_b.x = std::clamp(range_b.x, 0, _gridColumns - 1);
		range_b.y = std::clamp(range_b.y, 0, _gridRows - 1);

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

	int _cellWidth;
	int _gridColumns;
	int _gridRows;

private:
	sf::Vector2i _windowSize;
	sf::Vector2i _windowStart;

	sf::Vector2i getGridPosition(const Particle& particle) {
		return (sf::Vector2i(particle.position) + _windowSize / 2) / _cellWidth;
	}

	bool isOutsideWindow(const Particle& particle) {
		return 
			isnan(particle.position.x) || isnan(particle.position.y) ||
			isinf(particle.position.x) || isinf(particle.position.y) ||
			particle.position.x < _windowStart.x || particle.position.x > - _windowStart.x ||
			particle.position.y < _windowStart.y || particle.position.y > - _windowStart.y;
	}

	GridCell& GetGridCell(size_t y, size_t x) {
		return grid[y * _gridColumns + x];
	}
};