#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

using namespace boost::numeric::ublas;

class ParticleGrid {
public:
	ParticleGrid(std::vector<Particle*>& particles, sf::Vector2i windowSize, float cellWidth)
		: _particles(particles), _cellWidth(cellWidth), _windowSize(windowSize)
	{
		_gridColumns = (_windowSize.x / _cellWidth) + 1;
		_gridRows = (_windowSize.y / _cellWidth) + 1;
		_gridCells = matrix<std::vector<Particle*>>(_gridColumns, _gridRows);
	}

	void updateParticleNeighbours() {
		for (int i = _particles.size() - 1; i >= 0; i--) {
			auto particle = _particles[i];

			if (isOutsideWindow(particle)) {
				if (particle->neightboursIndex != -1) {
					auto& originalCell = _gridCells(particle->gridPosition.x, particle->gridPosition.y);

					removeAt(originalCell, particle->neightboursIndex);

					for (int i = particle->neightboursIndex; i < originalCell.size(); i++) {
						originalCell[i]->neightboursIndex--;
					}
				}

				removeAt(_particles, i);
				delete particle;
			}
		}

		for (auto particle : _particles) {
			sf::Vector2i gridPosition = getGridPosition(particle);

			if (particle->gridPosition != gridPosition) {
				auto& originalCell = _gridCells(particle->gridPosition.x, particle->gridPosition.y);
				auto& newCell = _gridCells(gridPosition.x, gridPosition.y);

				if (particle->neightboursIndex != -1) {
					removeAt(originalCell, particle->neightboursIndex);

					for (int i = particle->neightboursIndex; i < originalCell.size(); i++) {
						originalCell[i]->neightboursIndex--;
					}
				}

				newCell.emplace_back(particle);
				particle->gridPosition = gridPosition;
				particle->neightboursIndex = newCell.size() - 1;
			}
		}

		int common = 0;
		for (int column = 0; column < _gridColumns; column++) {
			for (int row = 0; row < _gridRows; row++) {
				common += _gridCells(column, row).size();
			}
		}
		//std::cout << common << std::endl;
	}

	matrix_range<matrix<std::vector<Particle*>>> getNeighbours(Particle* particle) {
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
			matrix<std::vector<Particle*>> m;
			return matrix_range<matrix<std::vector<Particle*>>>(m, range(0, 0), range(0, 0));
		}

		return matrix_range<matrix<std::vector<Particle*>>>(_gridCells,
			range(range_a.x, range_b.x + 1), range(range_a.y, range_b.y + 1));
	}

private:
	std::vector<Particle*>& _particles;
	matrix<std::vector<Particle*>> _gridCells;
	int _cellWidth;
	int _gridColumns;
	int _gridRows;

	sf::Vector2i _windowSize;

	void removeAt(std::vector<Particle*>& vector, int index) {
		vector.erase(vector.begin() + index);
	}

	sf::Vector2i getGridPosition(Particle* particle) {
		return (sf::Vector2i(particle->position) + _windowSize / 2) / _cellWidth;
	}

	bool isOutsideWindow(Particle* particle) {
		sf::Vector2i window_start = - _windowSize / 2;
		sf::Vector2i window_end = _windowSize / 2;

		return 
			particle->position.x < window_start.x || particle->position.x > window_end.x || 
			particle->position.y < window_start.y || particle->position.y > window_end.y;
	}
};