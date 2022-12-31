#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <Math/vector2.h>
#include <Math/vector2i.h>
#include <Elements/Wall.h>
#include <FluidProcessor.h>
#include <GLFW/glfw3.h>

class Wall;
class FluidProcessor;
class Tool;

class Scene {
public:
	Scene();
	~Scene();
	void Start();
	std::unique_ptr<FluidProcessor> _fluidProcessor;

	void Update();
	void Draw();
	void createParticles(vector2 position);
	void createWalls(std::vector<Wall>& walls);
	void InitEnvironment(bool is_full_screen);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	static void MouseClickCallback(GLFWwindow* window, int button, int action, int mods);

	std::unique_ptr<Tool> _tool;
	vector2 _mouse_pos;

	vector2i _window_size;
	GLFWwindow* _window = nullptr;
	std::vector<Wall> _walls;

	float _expected_delta_time = 1. / 60;
	float _delta_time = 0;
};