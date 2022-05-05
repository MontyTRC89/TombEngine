#pragma once
#include "frameworkbase.h"

struct ROOM_VECTOR 
{
	int roomNumber;
	int yNumber;
};

struct POLYGON
{
	int shape;
	int animatedSequence;
	int animatedFrame;
	std::vector<int> indices;
	std::vector<Vector2> textureCoordinates;
	std::vector<Vector3> normals;
	std::vector<Vector3> tangents;
	std::vector<Vector3> bitangents;
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
