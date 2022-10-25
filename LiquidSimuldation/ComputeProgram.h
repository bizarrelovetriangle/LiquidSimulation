#pragma once
#include "GPUProgramBase.h"

class ComputeProgram : private GPUProgramBase {
public:
	void Init(const std::string& path)
	{
        auto computeShader = CreateShader(path, GL_COMPUTE_SHADER);

        int success;
        program_id = glCreateProgram();
        glAttachShader(program_id, computeShader);
        glLinkProgram(program_id);
        glGetProgramiv(program_id, GL_LINK_STATUS, &success);
        LogErrors(success);

        glDeleteShader(computeShader);
	}

    void Compute() {
        size_t width = 10;
        size_t height = 10;
        glUseProgram(program_id);

        struct Vector2
        {
            float x = 10;
            float y = 20;
        };

        struct Particle
        {
            Vector2 position;
            Vector2 velocity;
        };

        std::vector<Particle> particles(10);
        uint32_t particlesBuffer;
        glGenBuffers(1, &particlesBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particlesBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle), &particles[0], GL_STATIC_DRAW);

        glDispatchCompute(10, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        auto data = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        std::vector<Particle> readData(data, data + 10);

        int test = 44;
    }
};