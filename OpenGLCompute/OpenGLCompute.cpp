#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <vector>

std::string readFile(const std::string& path) {
    std::stringstream ss;
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path);
    ss << file.rdbuf();
    return ss.str();
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwHideWindow(window);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    std::string computeShaderCode = readFile("Shaders/computeShader.glsl");
    auto computeShader = glCreateShader(GL_COMPUTE_SHADER);
    auto cstr = computeShaderCode.c_str();
    glShaderSource(computeShader, 1, &cstr, NULL);

    int success;
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };

    auto program = glCreateProgram();
    glAttachShader(program, computeShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(computeShader);


    size_t width = 10;
    size_t height = 10;
    glUseProgram(program);

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

    return 0;
}
