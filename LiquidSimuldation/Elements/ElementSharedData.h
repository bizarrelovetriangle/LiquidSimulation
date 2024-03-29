#pragma once
#include <vector>
#include <Math/vector2.h>
#include <OpenGL/DeviceProgram/RenderProgram.h>

template <typename T>
class ElementSharedData {
public:
	std::vector<vector2> initial_points;
	std::vector<uint32_t> indexes;
	RenderProgram<T> render_program;
};
