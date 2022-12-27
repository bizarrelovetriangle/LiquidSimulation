#pragma once
#include "Math/vector2.h"

class Scene;

class Tool
{
public:
	Tool(Scene& scene)
		: _scene(scene)
	{

	}

	virtual void OnClick() = 0;
	virtual void OnMoved(const vector2& pos) = 0;
	virtual void OnRelease() = 0;
	virtual void OnKey(int key, int action) = 0;
protected:
	Scene& _scene;
	vector2 _mouse_pos;
	bool clicked = false;
};
