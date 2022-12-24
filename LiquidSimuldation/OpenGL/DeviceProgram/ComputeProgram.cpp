#include "ComputeProgram.h"

void ComputeProgram::Use() const
{
	DeviceProgram::Use();
}

void ComputeProgram::Wait() const
{
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	auto sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	GLenum waitReturn = GL_UNSIGNALED;

	while (waitReturn != GL_ALREADY_SIGNALED && waitReturn != GL_CONDITION_SATISFIED) {
		waitReturn = glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
	}
}