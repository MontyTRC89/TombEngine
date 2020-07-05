#pragma once
#include "framework.h"

struct POLYGON
{
	int shape;
	std::vector<int> indices;
	std::vector<Vector2> textureCoordinates;
};

struct BUCKET
{
	int texture;
	byte blendMode;
	bool animated;
	int numQuads;
	int numTriangles;
	std::vector<POLYGON> polygons;
};
