#pragma once
#include <Tools/Tool.h>
#include <OpenGL/DeviceProgram/ComputeProgram.h>

class KnifeTool : public Tool
{
public:
	KnifeTool(Scene& scene);
	virtual void OnClick() override;
	virtual void OnMoved(const vector2& pos) override;
	virtual void OnRelease() override;
	virtual void OnKey(int key, int action) override;
private:
	ComputeProgram knife_program;
};

