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
		newCell.emplace_back(particle);
	}

	void updateParticleNeighbours() {
		for (auto& cell : GridCells.data()) {
			for (std::vector<Particle>::iterator iterator = cell.begin(); iterator != cell.end();) {
				auto& particle = *iterator._Ptr;

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

		if (range_a.x < 0) {
			range_a.x++;
		}
		if (range_a.y < 0) {
			range_a.y++;
		}
		if (range_b.x == _gridColumns) {
			range_b.x--;
		}
		if (range_b.y == _gridRows) {
			range_b.y--;
		}

		if (range_a.x > range_b.x || range_a.y > range_b.y) {
			matrix<std::vector<Particle>> m;
			return matrix_range<matrix<std::vector<Particle>>>(m, range(0, 0), range(0, 0));
		}

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

	sf::Vector2i getGridPosition(Particle& particle) {
		return (sf::Vector2i(particle.position) + _windowSize / 2) / _cellWidth;
	}

	bool isOutsideWindow(Particle& particle) {
		return 
			particle.position.x < _windowStart.x || particle.position.x > - _windowStart.x ||
			particle.position.y < _windowStart.y || particle.position.y > - _windowStart.y;
	}
};