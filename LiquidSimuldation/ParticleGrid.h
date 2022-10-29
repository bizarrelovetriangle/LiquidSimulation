#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <math.h>

using namespace boost::numeric::ublas;

class ParticleGrid {
public:
	void Init(sf::Vector2i windowSize, float cellWidth) {
		_cellWidth = cellWidth;
		_windowSize = windowSize;
		_windowStart = -windowSize / 2;
		_gridColumns = (_windowSize.x / _cellWidth) + 1;
		_gridRows = (_windowSize.y / _cellWidth) + 1;
		GridCells = matrix<std::vector<Particle>>(_gridColumns, _gridRows);
	}

	void addParticle(Particle& particle) {
		if (isOutsideWindow(particle)) {
			return;
		}

		sf::Vector2i gridPosition = getGridPosition(particle);
		auto& newCell = GridCells(gridPosition.x, gridPosition.y);
		particle.gridPosition = gridPosition;
		particle.index = lastIndex++;
		newCell.emplace_back(particle);
	}

	void updateParticleNeighbours() {
		for (auto& cell : GridCells.data()) {
			for (std::vector<Particle>::iterator iterator = cell.begin(); iterator != cell.end();) {
				auto& particle = *iterator;

				if (isOutsideWindow(particle)) {
					iterator = cell.erase(iterator);
					continue;
				}
			
				sf::Vector2i gridPosition = getGridPosition(particle);

				if (particle.gridPosition != gridPosition) {
					particle.gridPosition = gridPosition;
					auto& newCell = GridCells(gridPosition.x, gridPosition.y);
					newCell.emplace_back(particle);
					iterator = cell.erase(iterator);
				}
				else {
					iterator++;
				}
			}
		}
	}

	matrix_range<matrix<std::vector<Particle>>> getNeighbours(Particle& particle) {
		sf::Vector2i gridPosition = getGridPosition(particle);

		sf::Vector2i range_a = gridPosition - sf::Vector2i(1, 1);
		sf::Vector2i range_b = gridPosition + sf::Vector2i(1, 1);

		range_a.x = std::clamp(range_a.x, 0, _gridColumns - 1);
		range_a.y = std::clamp(range_a.y, 0, _gridRows - 1);
		range_b.x = std::clamp(range_b.x, 0, _gridColumns - 1);
		range_b.y = std::clamp(range_b.y, 0, _gridRows - 1);

		return matrix_range<matrix<std::vector<Particle>>>(GridCells,
			range(range_a.x, range_b.x + 1), range(range_a.y, range_b.y + 1));
	}

	matrix<std::vector<Particle>> GridCells;
private:
	int _cellWidth;
	int _gridColumns;
	int _gridRows;

	sf::Vector2i _windowSize;
	sf::Vector2i _windowStart;

	int lastIndex = 0;

	sf::Vector2i getGridPosition(Particle& particle) {
		return (sf::Vector2i(particle.position) + _windowSize / 2) / _cellWidth;
	}

	bool isOutsideWindow(Particle& particle) {
		return 
			isnan(particle.position.x) || isnan(particle.position.y) ||
			isinf(particle.position.x) || isinf(particle.position.y) ||
			particle.position.x < _windowStart.x || particle.position.x > - _windowStart.x ||
			particle.position.y < _windowStart.y || particle.position.y > - _windowStart.y;
	}
};