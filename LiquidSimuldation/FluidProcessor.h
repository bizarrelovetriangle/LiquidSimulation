#pragma once
#include <math.h>
#include "Particle.h"
#include "Wall.h"
#include "VectorFunctions.h"
#include "ParticleGrid.h"
#include "GPUCompute.h"
#include "Config.h"
#include "NeatTimer.h"

class FluidProcessor {
public:
	FluidProcessor(sf::Vector2i windowSize);
	void WallCollicionHandling(const std::vector<Wall>& walls, double interval);
	std::vector<PairData> CreatePairs();
	void ParticlesGravity(float& interval);
	void ApplyViscosity(float& interval);
	void CreateParticle(vector2 position);
	void Update(const std::vector<Wall>& walls, float dt);
	void Draw();

	std::vector<PairData> pairs;
private:
	ParticleGrid _particle_grid;
};