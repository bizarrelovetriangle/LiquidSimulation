#pragma once
#include <Elements/Particle.h>
#include <math.h>
#include <span>
#include <Config.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2i.h>
#include <deque>

class ParticleGrid {
public:
	struct GridCell {
		bool empty() {
			return start == end;
		}

		int start;
		int end;
	};

	void Init(vector2i windowSize);
	void AddParticle(const Particle& particle);
	void UpdateParticleNeighbours();
	std::vector<std::span<Particle>> GetNeighbours(const Particle& particle);
	std::vector<GridCell> GetNeighbourIndexes(const Particle& particle);
	GridCell& GetGridCell(const vector2i& grid_position);

	std::deque<int> deactivated_indexes;
	std::vector<Particle> particles;
	std::vector<int> particle_indexes;
	std::vector<GridCell> grid;

	int cellWidth;
	vector2i size;

	bool particles_updated = false;

private:
	vector2i GetGridPosition(const Particle& particle);
	bool IsOutsideWindow(const Particle& particle);

	vector2i _windowSize;
	vector2i _windowStart;
};