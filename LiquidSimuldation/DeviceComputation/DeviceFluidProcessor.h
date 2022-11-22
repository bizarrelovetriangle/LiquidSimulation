#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <OpenGL/DeviceProgram/ComputeProgram.h>
#include <Utils/NeatTimer.h>
#include <Utils/Algorithms.h>
#include <Math/vector2.h>
#include <DeviceComputation/PairCreator.h>

class ParticleGrid;

class DeviceFluidProcessor {
private:
	DeviceFluidProcessor();

public:
	static DeviceFluidProcessor& GetInstance();
	std::vector<PairData> Update(ParticleGrid& particle_grid, float dt);

private:
	void ParticleUpdate(ParticleGrid& particle_grid, float dt);
	void GranularProcessPairs(const ComputeProgram& program, ParticleGrid& particle_grid, float dt);

	PairCreator _pair_creator;

	ComputeProgram particle_viscosity_program;
	ComputeProgram particle_update_program;
	ComputeProgram particle_density_program;
	ComputeProgram particle_gravity_program;
};