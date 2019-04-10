#include "Drawer2D.h"


Drawer2D::Drawer2D(int width, int height)
{
	initialize(width, height);
}

void Drawer2D::initialize(int width, int height)
{
	initialised = true;
	this->width = width;
	this->height = height;
	buffer = new Color[width*height];
	tempbuf = new float[width*height * 5];
	vb.createData(nullptr, width*height * 5 * sizeof(float));
};

void Drawer2D::Draw(int x, int y, Color c)
{
	y = height - y - 1;
	x = width - x - 1;
	buffer[x + (y * width)] = c;
}

void Drawer2D::Clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
	ZeroMemory(buffer, width*height * sizeof(Color));
}

void Drawer2D::Render()
{
	//convert
	int pos = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			tempbuf[pos] = ((((float)x / (float)width) * 2) - 1)*-1;	//x
			pos++;
			tempbuf[pos] = ((((float)y / (float)height) * 2) - 1)*-1; 	//y
			pos++;
			tempbuf[pos] = ((float)buffer[x + (y * width)].r) / (float)UCHAR_MAX;
			pos++;
			tempbuf[pos] = ((float)buffer[x + (y * width)].g) / (float)UCHAR_MAX;
			pos++;
			tempbuf[pos] = ((float)buffer[x + (y * width)].b) / (float)UCHAR_MAX;
			pos++;
		}

	}

	vb.subData(tempbuf, 0, width*height * 5 * sizeof(float));
	va.bind();

	glDrawArrays(GL_POINTS, 0, width*height);
}

void Drawer2D::Cleanup()
{
	if(initialised)
	{
		initialised = false;
		delete[] buffer;
		delete[] tempbuf;
		vb.cleanup();
	}
}
