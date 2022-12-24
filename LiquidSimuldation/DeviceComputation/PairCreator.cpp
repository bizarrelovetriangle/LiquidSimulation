#include "PairCreator.h"
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <Utils/Algorithms.h>
#include <ParticleGrid.h>
#include <unordered_set>

PairCreator::PairCreator(ParticleGrid& particle_grid)
	: _particle_grid(particle_grid)
{
	compute_dencity_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/compute_dencity.comp" } });
	create_pairs_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/particle_pairs/create_pairs.comp" } });
}

void PairCreator::ComputePairs(float dt) {
	ComputeDencity();
	CreatePairs(dt);
}

void PairCreator::ComputeDencity() {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	if (particles.empty()) return;

	compute_dencity_program.Use();
	glDispatchCompute(_particle_grid.particles.size(), 1, 1);
	compute_dencity_program.Wait();
}

void PairCreator::CreatePairs(float dt) {
	NeatTimer::GetInstance().StageBegin(__func__);
	auto& particles = _particle_grid.particles;
	auto& grid = _particle_grid.grid;
	if (particles.empty()) return;

	create_pairs_program.Use();
	glUniform1f(0, dt);
	glDispatchCompute(_particle_grid.particles.size(), 1, 1);
	create_pairs_program.Wait();
}
