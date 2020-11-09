/////////////////////////////////////////////
//shapesGenerator.cpp
//Copyright(c) 2019 Luta Vlad
//https://github.com/meemknight/OpenGLEngine
/////////////////////////////////////////////
#include <cstdlib>
#include <Windows.h>
#include <iostream>
#include <glm/glm.hpp>

namespace shapeGenerator
{

	auto getVertex(int x, int y, int element, float **vertexes, int size)
	{
		int evaluated = (((x)+(y * (size + 1))) * 9) + (element);
		// std::cout << evaluated << std::endl;

		return &((*vertexes)[(((x)+(y * (size + 1))) * 9) + (element)]);
	}

	auto getVertexS(int x, int y, int element, float **vertexes, int size)
	{
		if (x < 0) { x = 0; }
		if (y < 0) { y = 0; }
		if (x >= size) { x = size - 1;}
		if (y >= size) { y = size - 1;}
		return(getVertex(x, y, element, vertexes, size));
	}

	
	void generatePlane(float **vertexes, unsigned int **indices, int size, int &vSize, int &iSize)
	{

		vSize = (size + 1)*(size + 1) * 9;
		iSize = (size)*(size) * 6;

		*vertexes = new float[vSize];
		*indices = new unsigned int[iSize];

		//int count2 = 0;
		for (int y = 0; y < size + 1; y++)
		{
			for (int x = 0; x < size + 1; x++)
			{

				*(getVertex(x, y, 0, vertexes, size)) = x - (size / 2);  //x component
				*(getVertex(x, y, 1, vertexes, size)) = 0;
				*(getVertex(x, y, 2, vertexes, size)) = y - (size / 2); //z component

				// colors
				*(getVertex(x, y, 3, vertexes, size)) = 0.2;		
				*(getVertex(x, y, 4, vertexes, size)) = 0.2;
				*(getVertex(x, y, 5, vertexes, size)) = 0.2;

				// normals
				*(getVertex(x, y, 6, vertexes, size)) = 0;
				*(getVertex(x, y, 7, vertexes, size)) = 1;
				*(getVertex(x, y, 8, vertexes, size)) = 0;
			

				//count2 += 6;

			}
		}

		int count = 0;

		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				/*
				(*indices)[count] = x + (y * (size + 1));
				count++;
				(*indices)[count] = x + 1 + (y * (size + 1));
				count++;
				(*indices)[count] = x + ((y + 1) * (size + 1));
				count++;

				(*indices)[count] = x + 1 + (y * (size + 1));
				count++;
				(*indices)[count] = x + 1 + ((y + 1) * (size + 1));
				count++;
				(*indices)[count] = x + ((y + 1) * (size + 1));
				count++;
				*/

				(*indices)[count] = x + ((y + 1) * (size + 1));
				count++;
				(*indices)[count] = x + 1 + (y * (size + 1));
				count++;
				(*indices)[count] = x + (y * (size + 1));
				count++;

				(*indices)[count] = x + ((y + 1) * (size + 1));
				count++;
				(*indices)[count] = x + 1 + ((y + 1) * (size + 1));
				count++;
				(*indices)[count] = x + 1 + (y * (size + 1));
				count++;
			}

		}

	}


}