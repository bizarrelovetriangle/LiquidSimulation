#pragma once
#include "GPUProgramBase.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

class ComputeProgram : public GPUProgramBase {
public:
    void Compute(std::vector<Particle>& particles2, float dt) {
        if (particles2.empty()) return;

        size_t width = particles2.size();
        size_t height = 1;
        glUseProgram(program_id);

        uint32_t particlesBuffer;
        glGenBuffers(1, &particlesBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particlesBuffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, particles2.size() * sizeof(Particle), &particles2[0], GL_STATIC_DRAW);
        glUniform1f(0, dt);

        glDispatchCompute(10, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        auto data = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        std::vector<Particle> readData(data, data + particles2.size());
        particles2 = readData;
        int test = 44;
    }
};