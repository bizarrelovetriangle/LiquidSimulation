#pragma once
#include "Tools/Tool.h"

class EmitterTool : public Tool
{
public:
	EmitterTool(Scene& scene);
	virtual void OnClick() override;
	virtual void OnMoved(const vector2& pos) override;
	virtual void OnRelease() override;
	virtual void OnKey(int key, int action) override;
};

