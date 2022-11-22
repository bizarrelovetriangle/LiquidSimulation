#pragma once
#include <Elements/Particle.h>
#include <math.h>
#include <span>
#include <Config.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>

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

	void Init(sf::Vector2i windowSize);
	void AddParticle(const Particle& particle);
	void UpdateParticleNeighbours();
	std::vector<std::span<Particle>> GetNeighbours(const Particle& particle);
	std::vector<GridCell> GetNeighbourIndexes(const Particle& particle);
	GridCell& GetGridCell(const sf::Vector2i& grid_position);

	std::vector<Particle> particles;
	GridType grid;

	int cellWidth;
	sf::Vector2i size;

private:
	sf::Vector2i GetGridPosition(const Particle& particle);
	bool IsOutsideWindow(const Particle& particle);

	sf::Vector2i _windowSize;
	sf::Vector2i _windowStart;
};