#pragma once
#include <GL/glew.h>

class FrameBufferObject
{
	void initialize();
public:
	FrameBufferObject();

	unsigned int id = 0;

	void cleanup();
};

