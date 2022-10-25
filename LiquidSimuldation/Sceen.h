#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include "Particle.h"
#include "Wall.h"
#include "FluidProcessor.h"
#include "DataFactory.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Sceen {
public:
	Sceen() = delete;

	Sceen(bool is_full_screen)
	{
		InitEnvironment(is_full_screen);

		DataFactory<matrix3x3>::GetData([&]() {
			auto matrix = std::make_shared<matrix3x3>();
			matrix->scale(vector3(2. / _window_size.x, 2. / _window_size.y, 1.));
			return matrix;
		 });

		createWalls(_walls);

		_fluidProcessor = std::make_unique<FluidProcessor>(_window_size);
	}

	void Start() {
		sf::Clock clock;
		glfwSetKeyCallback(_window, KeyCallback);
		glfwSetCursorPosCallback(_window, CursorPositionCallback);

		while (!glfwWindowShouldClose(_window))
		{
			glfwPollEvents();

			float interval = 1. / 100;
			double time = clock.getElapsedTime().asSeconds();
			if (time < interval) sf::sleep(sf::seconds(interval - time));
			_deltaTime = clock.restart().asSeconds();

			Update();
			Draw();
		}
	}

private:
	void Update() {
		_fluidProcessor->Update(_walls, _expectedDeltaTime);
	}

	void Draw() {
		glClearColor(0.f, 0.f, 0.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		_fluidProcessor->Draw();

		for (auto& wall : _walls) {
			wall.draw();
		}

		glfwSwapBuffers(_window);
	}

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto sceen = (Sceen*)glfwGetWindowUserPointer(window);
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_FALSE);
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
			sceen->_fluidProcessor->createParticle(sceen->_mouse_position);
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			sceen->createParticles(sceen->_mouse_position);
	}

	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
	{
		auto sceen = (Sceen*)glfwGetWindowUserPointer(window);
		sceen->_mouse_position = sf::Vector2f(sf::Vector2i(xpos, ypos) - sceen->_window_size / 2);
		sceen->_mouse_position.y = -sceen->_mouse_position.y;
	}

	void createParticles(vector2 position) {
		int distance = 10;
		int width = 20;
		int height = 20;

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				vector2 particlePosition(x - width / 2, y - height / 2);
				_fluidProcessor->createParticle(particlePosition * (double)distance + position);
			}
		}
	}

	void createWalls(std::vector<Wall>& walls) {
		int wall_width = _window_size.x - 100;
		int wall_height = _window_size.y - 100;

		sf::Vector2f point_a(-wall_width / 2, -wall_height / 2);
		sf::Vector2f point_b(wall_width / 2, -wall_height / 2);
		sf::Vector2f point_c(wall_width / 2, wall_height / 2);
		sf::Vector2f point_d(-wall_width / 2, wall_height / 2);

		walls.emplace_back(Wall(point_a, point_b));
		walls.emplace_back(Wall(point_b, point_c));
		walls.emplace_back(Wall(point_c, point_d));
		walls.emplace_back(Wall(point_d, point_a));
		//walls.emplace_back(Wall(window, sf::Vector2f(-100, -100), sf::Vector2f(100, 100)));
	}

	void InitEnvironment(bool is_full_screen) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		if (is_full_screen) {
			_window_size = sf::Vector2i(1920, 1080);
			_window = glfwCreateWindow(_window_size.x, _window_size.y, "LearnOpenGL", glfwGetPrimaryMonitor(), nullptr);
		}
		else {
			_window_size = sf::Vector2i(1000, 800);
			_window = glfwCreateWindow(_window_size.x, _window_size.y, "LearnOpenGL", nullptr, nullptr);
		}

		if (!_window)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
		}
		glfwSetWindowUserPointer(_window, this);
		glfwMakeContextCurrent(_window);
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}
	}

	vector2 _mouse_position;
	sf::Vector2i _window_size;
	GLFWwindow* _window = nullptr;
	std::unique_ptr<FluidProcessor> _fluidProcessor;
	std::vector<Wall> _walls;

	float _expectedDeltaTime = 1. / 60;
	float _deltaTime = 0;
};