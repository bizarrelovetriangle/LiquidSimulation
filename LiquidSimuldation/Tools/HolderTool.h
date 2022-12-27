#pragma once
#include <Tools/Tool.h>
#include <unordered_map>

class HolderTool : public Tool
{
public:
	HolderTool(Scene& scene);
	virtual void OnClick() override;
	virtual void OnMoved(const vector2& pos) override;
	virtual void OnRelease() override;
	virtual void OnKey(int key, int action) override;
private:
	float range = 100;
	std::unordered_map<int, vector2> _controlled_particles;
};

