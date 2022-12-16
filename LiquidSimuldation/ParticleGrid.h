#pragma once
#include <Elements/Particle.h>
#include <math.h>
#include <span>
#include <Config.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2i.h>

class ParticleGrid {
public:
	struct GridCell {
		bool empty() {
			return particles_start == particles_end;
		}

		int particles_start;
		int particles_end;
	};

	using GridType = std::vector<GridCell>;

	void Init(vector2i windowSize);
	void AddParticle(const Particle& particle);
	void UpdateParticleNeighbours();
	std::vector<std::span<Particle>> GetNeighbours(const Particle& particle);
	std::vector<GridCell> GetNeighbourIndexes(const Particle& particle);
	GridCell& GetGridCell(const vector2i& grid_position);

	std::vector<Particle> particles;
	GridType grid;

	int cellWidth;
	vector2i size;

private:
	vector2i GetGridPosition(const Particle& particle);
	bool IsOutsideWindow(const Particle& particle);

	vector2i _windowSize;
	vector2i _windowStart;
};