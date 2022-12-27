#include "EmitterTool.h"
#include "Scene.h"

EmitterTool::EmitterTool(Scene& scene)
	: Tool(scene)
{
}

void EmitterTool::OnClick() {
	clicked = true;
}

void EmitterTool::OnMoved(const vector2& pos) {
	_mouse_pos = pos;
	if (!clicked) return;

	static vector2 prev_pos;

	if ((_mouse_pos - prev_pos).length() > 3) {
		//vector2 deviation = vector2(float(rand()), float(rand())) - vector2(RAND_MAX / 2);
		_scene._fluidProcessor->CreateParticle(_mouse_pos);
	}

	prev_pos = _mouse_pos;
}

void EmitterTool::OnRelease() {
	clicked = false;
}

void EmitterTool::OnKey(int key, int action) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		_scene.createParticles(_mouse_pos);
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
		_scene._fluidProcessor->CreateParticle(_mouse_pos);
}
