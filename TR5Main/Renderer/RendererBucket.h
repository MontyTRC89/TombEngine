#pragma once

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>

using namespace std;

struct RendererVertex;
struct RendererPolygon;

class RendererBucket
{
private:
	LPDIRECT3DDEVICE9			m_device;
	PDIRECT3DVERTEXBUFFER9		m_vertexBuffer;
	PDIRECT3DINDEXBUFFER9		m_indexBuffer;

public:
	RendererBucket(LPDIRECT3DDEVICE9 device);
	~RendererBucket();
	bool						UpdateBuffers();

	vector<RendererVertex>		Vertices;
	vector<__int32>				Indices;
	vector<RendererPolygon>		Polygons;
	__int32						StartVertex;
	__int32						NumTriangles;
	__int32						NumVertices;
	__int32						NumIndices;
	PDIRECT3DVERTEXBUFFER9		GetVertexBuffer();
	PDIRECT3DINDEXBUFFER9		GetIndexBuffer();
};
