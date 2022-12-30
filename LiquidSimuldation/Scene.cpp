#pragma once
#include <Scene.h>;
#include <iostream>
#include <chrono>
#include <thread>
#include <Elements/Particle.h>
#include <FluidProcessor.h>
#include <Utils/DataFactory.h>
#include <Utils/NeatTimer.h>
#include <DeviceComputation/CommonBuffers.h>
#include <OpenGL/DeviceProgram/DeviceProgram.h>
#include <Tools/HolderTool.h>
#include <Tools/EmitterTool.h>
#include <Tools/KnifeTool.h>

Scene::Scene()
{
	bool is_full_screen = false;
	InitEnvironment(is_full_screen);

	DataFactory<matrix3x3>::GetData([&]() {
		auto matrix = std::make_shared<matrix3x3>();
		*matrix *= vector3(2. / _window_size.x, 2. / _window_size.y, 1.);
		return matrix;
		});

	createWalls(_walls);

	_fluidProcessor = std::make_unique<FluidProcessor>(_window_size);

	_tool = std::make_unique<EmitterTool>(*this);
}

Scene::~Scene() = default;

void Scene::Start() {
	std::chrono::steady_clock clock;
	std::chrono::steady_clock::time_point then;
	glfwSetKeyCallback(_window, KeyCallback);
	glfwSetCursorPosCallback(_window, CursorPositionCallback);
	glfwSetMouseButtonCallback(_window, MouseClickCallback);

	//createParticles(vector2());

	while (!glfwWindowShouldClose(_window))
	{
		NeatTimer::GetInstance().StageBegin("Events");
		glfwPollEvents();

		auto interval = std::chrono::milliseconds(1000) / 100;
		auto dt = clock.now() - then;
		if (dt < interval) std::this_thread::sleep_for(interval - dt);
		_deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(clock.now() - then).count();
		then = clock.now();

		Update();
		NeatTimer::GetInstance().StageBegin("Draw");
		Draw();
		NeatTimer::GetInstance().Refresh(std::chrono::seconds(1));
	}
}

void Scene::Update() {
	_tool->OnMoved(_mouse_pos);

	for (auto& wall : _walls) {
		wall.Update(_expectedDeltaTime);
	}

	_fluidProcessor->Update(_walls, _expectedDeltaTime);
}

void Scene::Draw() {
	glClearColor(0.f, 0.f, 0.f, 1.0f); 
	glClear(GL_COLOR_BUFFER_BIT);

	for (auto& wall : _walls) {
		wall.draw();
	}
	_walls[4].draw();

	{
		size_t threads_count = CommonBuffers::GetInstance().threads_count->Retrive().front();
		DeviceProgram render_program;
 		render_program.InitProgram({
			{ GL_VERTEX_SHADER, "shaders/render/thread.vert" },
			{ GL_FRAGMENT_SHADER, "shaders/render/thread.frag" } });
			
		glUseProgram(render_program.program_id);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CommonBuffers::GetInstance().particles->GetBufferId());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, CommonBuffers::GetInstance().pairs->GetBufferId());
			
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, CommonBuffers::GetInstance().config->GetBufferId());
		auto& view_matrix = *DataFactory<matrix3x3>::GetData();
		glUniformMatrix3fv(0, 1, GL_FALSE, (float*)&view_matrix);
		vector3 color(0.9, 0.6, 0.7);
		glUniform4fv(2, 1, (float*)&color);
			
		glDrawArraysInstanced(GL_LINES, 0, 2, threads_count);
	}

	_fluidProcessor->Draw();

	glfwSwapBuffers(_window);
}

void Scene::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	auto scene = (Scene*)glfwGetWindowUserPointer(window);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_FALSE);
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		scene->_tool = std::make_unique<EmitterTool>(*scene);
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		scene->_tool = std::make_unique<HolderTool>(*scene);
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		scene->_tool = std::make_unique<KnifeTool>(*scene);

	scene->_tool->OnKey(key, action);
}

void Scene::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	auto scene = (Scene*)glfwGetWindowUserPointer(window);
	scene->_mouse_pos = vector2i(xpos, ypos) - scene->_window_size / 2;
	scene->_mouse_pos.y = -scene->_mouse_pos.y;
}

void Scene::MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
{
	auto scene = (Scene*)glfwGetWindowUserPointer(window);
	if (action == GLFW_PRESS)
		scene->_tool->OnClick();
	if (action == GLFW_RELEASE)
		scene->_tool->OnRelease();
}

void Scene::createParticles(vector2 position) {
	int distance = 15;
	int width = 20;
	int height = 20;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			vector2 particlePosition(x - width / 2, y - height / 2);
			_fluidProcessor->CreateParticle(particlePosition * (float)distance + position);
		}
	}
}

void Scene::createWalls(std::vector<Wall>& walls) {
	int wall_width = _window_size.x - 30;
	int wall_height = _window_size.y - 30;

	vector2 point_a(-wall_width / 2, -wall_height / 2);
	vector2 point_b(wall_width / 2, -wall_height / 2);
	vector2 point_c(wall_width / 2, wall_height / 2);
	vector2 point_d(-wall_width / 2, wall_height / 2);

	walls.emplace_back(Wall(point_a, point_b));
	walls.emplace_back(Wall(point_b, point_c));
	walls.emplace_back(Wall(point_c, point_d));
	walls.emplace_back(Wall(point_d, point_a));
	walls.emplace_back(Wall(vector2(100, -200), vector2(500, -200)));
	//walls[4].rotate_speed = 1;
}

void Scene::InitEnvironment(bool is_full_screen) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (is_full_screen) {
		_window_size = vector2i(1920, 1080);
		_window = glfwCreateWindow(_window_size.x, _window_size.y, "LearnOpenGL", glfwGetPrimaryMonitor(), nullptr);
	}
	else {
		_window_size = vector2i(1400, 1000);
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