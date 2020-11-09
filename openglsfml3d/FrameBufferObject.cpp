#include "FrameBufferObject.h"



void FrameBufferObject::initialize()
{
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

}

FrameBufferObject::FrameBufferObject()
{

}

void FrameBufferObject::cleanup()
{
}
