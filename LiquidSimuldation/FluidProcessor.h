#pragma once
#include <vector>
#include <Math/vector2.h>
#include <Elements/Particle.h>
#include <Elements/Wall.h>
#include <DeviceComputation/DeviceFluidProcessor.h>
#include <ParticleGrid.h>
#include <Elements/ParticlesThread.h>

class FluidProcessor {
public:
	FluidProcessor(vector2i windowSize);
	void WallCollicionHandling(const std::vector<Wall>& walls, float dt);
	std::vector<ParticlesThread> Createthreads();
	void ParticlesGravity(float& interval);
	void ApplyViscosity(float& interval);
	void CreateParticle(vector2 position);
	void Update(const std::vector<Wall>& walls, float dt);
	void Draw();

	std::vector<ParticlesThread> threads;
//private:
	ParticleGrid _particle_grid;
};