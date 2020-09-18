#pragma once
#include "Particle.h"
#include <iostream>
#include <list>

class ParticleGrid {
public:
	ParticleGrid(std::vector<Particle*> particles, int gridCellWidth)
		: _particles(particles), _gridCellWidth(gridCellWidth)
	{
		_gridWidth = window_width / _gridCellWidth;
		int gridSize = (window_width * window_height) / _gridWidth;
		_gridCells = std::vector<std::vector<Particle*>*>(gridSize);

		for (int i = 0; i < gridSize; i++) {
			_gridCells[i] = new std::vector<Particle*>();
		}
	}

	void updateParticleNeighbours() {
		for (auto particle : _particles) {
			int cellIndex = getGridIndex(particle);
		
			if (particle->gridCellIndex != cellIndex) {
				auto/*&*/ originalCell = _gridCells[particle->gridCellIndex];
				
				removeAt(originalCell, particle->neightboursIndex);
		
				for (int i = particle->neightboursIndex - 1; i < originalCell->size(); i++) {
					(*originalCell)[i]->neightboursIndex--;
				}

				_gridCells[cellIndex]->emplace_back(particle);
				particle->gridCellIndex = cellIndex;
				particle->neightboursIndex = _gridCells[cellIndex]->size();
			}
		}

		int common = 0;
		for (auto cell : _gridCells) {
			common += cell->size();
		}
		//std::cout << common << std::endl;
	}

	std::vector<Particle*>* getNeighbours(Particle* particle) {
		int particleCellIndex = getGridIndex(particle);
		
		int x_index = particleCellIndex % _gridWidth;
		int y_index = particleCellIndex / _gridWidth;

		std::vector<Particle*>* res = new std::vector<Particle*>();
		
		for (int x = x_index - 1; x <= x_index + 1; x++) {
			for (int y = y_index - 1; y <= y_index + 1; y++) {
				int cellIndex = getGridIndex(x, y);
				auto cell = *(_gridCells[cellIndex]);
				res->insert(res->end(), cell.begin(), cell.end());
			}
		}
		
		return res;
	}

private:
	std::vector<Particle*> _particles;
	std::vector<std::vector<Particle*>*> _gridCells;
	int _gridCellWidth;
	int _gridWidth;

	int window_width = 1000;
	int window_height = 800;

	int getGridIndex(Particle* particle) {
		int position_x = particle->position.x + window_width / 2;
		int position_y = particle->position.y + window_height / 2;

		int column = position_x / _gridWidth;
		int row = position_y / _gridWidth;

		return column + row * _gridWidth;
	}

	inline int getGridIndex(int column, int row) {
		return column + row * _gridWidth;
	}

	void removeAt(std::vector<Particle*>* vector, int index) {
		if (index-- == 0) {
			return;
		}

		vector->erase(vector->begin() + index);
	}
};