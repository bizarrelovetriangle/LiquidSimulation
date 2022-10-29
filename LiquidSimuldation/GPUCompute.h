#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "ComputeProgram.h"

class GPUCompute {
public:
    GPUCompute()
    {
        compute_shader.InitProgram({{ GL_COMPUTE_SHADER, "Shaders/compute.glsl" }});
    }

    void Run(ParticleGrid& grid, float dt)
    {
        //for (auto& particles : grid.GridCells.data()) {
        //    compute_shader.Compute(particles, dt);
        //}
    }

private:
    ComputeProgram compute_shader;
};
