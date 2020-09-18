#pragma once
#include "Particle.h"
#include <iostream>
#include <list>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

using namespace boost::numeric::ublas;

class ParticleGrid {
public:
	ParticleGrid(std::vector<Particle*> particles, int cellWidth)
		: _particles(particles), _cellWidth(cellWidth)
	{
		_gridColumns = window_width / _cellWidth;
		_gridRows = window_height / _cellWidth;
		_gridCells = matrix<std::vector<Particle*>>(_gridColumns, _gridRows);
	}

	void updateParticleNeighbours() {
		for (auto particle : _particles) {
			int column = (particle->position.x + window_width / 2) / _cellWidth;
			int row = (particle->position.y + window_height / 2) / _cellWidth;

			if (column >= _gridColumns || row >= _gridRows || column < 0 || row < 0) {
				auto& originalCell = _gridCells(particle->gridColumn, particle->gridRow);

				removeAt(originalCell, particle->neightboursIndex);

				for (int i = particle->neightboursIndex - 1; i < originalCell.size(); i++) {
					originalCell[i]->neightboursIndex--;
				}

				particle->gridColumn = 0;
				particle->gridRow = 0;
				particle->neightboursIndex = 0;

				continue;
			}

			if (particle->gridColumn != column || particle->gridRow != row) {
				auto& originalCell = _gridCells(particle->gridColumn, particle->gridRow);
				auto& newCell = _gridCells(column, row);
				
				removeAt(originalCell, particle->neightboursIndex);
				
				for (int i = particle->neightboursIndex - 1; i < originalCell.size(); i++) {
					originalCell[i]->neightboursIndex--;
				}

				newCell.emplace_back(particle);
				particle->gridColumn = column;
				particle->gridRow = row;
				particle->neightboursIndex = newCell.size();
			}
		}

		int common = 0;
		for (int column = 0; column < _gridColumns; column++) {
			for (int row = 0; row < _gridRows; row++) {
				common += _gridCells(column, row).size();
			}
		}
		std::cout << common << std::endl;
	}

	matrix_range<matrix<std::vector<Particle*>>> getNeighbours(Particle* particle) {

		//{
		//	matrix<int> m(5, 5);
		//	for (int col = 0; col < 5; col++) {
		//		for (int row = 0; row < 5; row++) {
		//			m(col, row) = col + 10 * row;
		//		}
		//	}
		//
		//	matrix_range<matrix<int>> mr(m, range(2, 5), range(2, 3));
		//
		//	for (unsigned i = 0; i < mr.size1(); ++i)
		//		for (unsigned j = 0; j < mr.size2(); ++j)
		//			mr(i, j) = 3 * i + j;
		//
		//	int i = 5;
		//}

		int column = (particle->position.x + window_width / 2) / _cellWidth;
		int row = (particle->position.y + window_height / 2) / _cellWidth;
	
		int range_a_x = column - 1;
		int range_a_y = row - 1;
		int range_b_x = column + 1;
		int range_b_y = row + 1;

		if (range_a_x < 0) {
			range_a_x++;
		}
		if (range_a_y < 0) {
			range_a_y++;
		}
		if (range_b_x == _gridColumns) {
			range_b_x--;
		}
		if (range_b_y == _gridRows) {
			range_b_y--;
		}

		if (range_a_x > range_b_x || range_a_y > range_b_y) {
			int t = 9;
		}

		if (range_b_x >= _gridColumns || range_b_y >= _gridRows ||
			range_a_x < 0 || range_a_y < 0) {
			int t = 9;
		}

		try {
			matrix_range<matrix<std::vector<Particle*>>> mr(_gridCells,
				range(range_a_x, range_b_x), range(range_a_y, range_b_y));


			return mr;
		}
		catch (std::exception ex) {
			throw;
		}
	}

private:
	std::vector<Particle*> _particles;
	matrix<std::vector<Particle*>> _gridCells;
	int _cellWidth;
	int _gridColumns;
	int _gridRows;

	int window_width = 1000;
	int window_height = 800;

	int getGridIndex(Particle* particle) {
		int position_x = particle->position.x + window_width / 2;
		int position_y = particle->position.y + window_height / 2;

		int column = position_x / _gridColumns;
		int row = position_y / _gridColumns;

		return column + row * _gridColumns;
	}

	inline int getGridIndex(int column, int row) {
		return column + row * _gridColumns;
	}

	void removeAt(std::vector<Particle*>& vector, int index) {
		if (index-- == 0) {
			return;
		}

		vector.erase(vector.begin() + index);
	}
};