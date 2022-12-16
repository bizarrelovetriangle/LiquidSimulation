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
	DeviceFluidProcessor(ParticleGrid& particle_grid);

public:
	static DeviceFluidProcessor& GetInstance(ParticleGrid& particle_grid);
	void Update(float dt);

private:
	void ParticleUpdate(float dt);

	ParticleGrid& _particle_grid;
	PairCreator _pair_creator;

	ComputeProgram particle_update_program;
};
