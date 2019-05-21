#pragma once
#include <GL\glew.h>
#include <iostream>
#include <Windows.h>
#include "vertexAttribute.h"
#include "vertexBuffer.h"
#include <SFML/Graphics.hpp>

class Color
{
public:
	Color() {};
	Color(int c) :color(c) {}
	//Color(unsigned int color) :color(color) {};

	union
	{
		unsigned __int32 color;
		struct
		{
			unsigned char b;
			unsigned char g;
			unsigned char r;
			unsigned char a;
		};
	};

	enum
	{
		RED = 0x11111111 << 8,
		GREEN = 0x11111111 << 4,
		BLUE = 0x11111111,
		WHITE = RED | GREEN | BLUE,
	};
};


class Drawer2D
{
public:
	Drawer2D(int width, int height);
	Drawer2D() {};

	void initialize(int width, int height);

	int width;
	int height;

	int program;
	Color *buffer;
	float *tempbuf;
	vertexAttribute va{ 2u, 3u };
	vertexBuffer vb;
	sf::RenderWindow *window;

	bool initialised = false;

	void Draw(int x, int y, Color c);
	void Clear();
	void Render();

	//this will save to an image the current drawing target
	void SaveImage();

	void Cleanup();
};

