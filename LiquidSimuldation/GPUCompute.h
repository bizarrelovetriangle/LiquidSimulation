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
        compute_shader.Init("Shaders/compute.glsl");
    }

    void Run()
    {
        compute_shader.Compute();
    }

private:
    ComputeProgram compute_shader;
};
