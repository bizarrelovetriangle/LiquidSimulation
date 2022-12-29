#include "KnifeTool.h"
#include "Scene.h"

KnifeTool::KnifeTool(Scene& scene)
	: Tool(scene)
{
	knife_program.InitProgram({ { GL_COMPUTE_SHADER, "shaders/compute/tools/knife.comp" } });
}

void KnifeTool::OnClick() {
	clicked = true;
}

void KnifeTool::OnMoved(const vector2& pos) {
	static vector2 prev_pos;
	prev_pos = _mouse_pos;
	_mouse_pos = pos;
	if (!clicked) return;

	knife_program.Use();
	int parallel = 200;
	glUniform2f(0, prev_pos.x, prev_pos.y);
	glUniform2f(1, _mouse_pos.x, _mouse_pos.y);
	glDispatchCompute(parallel, 1, 1);
	knife_program.Wait();
}

void KnifeTool::OnRelease() {
	clicked = false;
}

void KnifeTool::OnKey(int key, int action) {

}
