#pragma once
#include <OpenGL/DeviceProgram/DeviceProgram.h>

class ComputeProgram : public DeviceProgram
{
public:
	void Wait() const;
};