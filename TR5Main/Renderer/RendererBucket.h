#pragma once

#ifdef OLDCODE

#include <stdio.h>
#include <D3D11.h>
#include <DxErr.h>
#include <vector>
#include <memory>

using namespace std;

struct RendererVertex;
struct RendererPolygon;

class RendererBucket
{
private:
	ID3D11Device*				m_device;
	ID3D11Buffer*				m_vertexBuffer;
	ID3D11Buffer*				m_indexBuffer;

public:
	RendererBucket(ID3D11Device* device);
	~RendererBucket();
	bool						UpdateBuffers();

	vector<RendererVertex>		Vertices;
	vector<__int32>				Indices;
	vector<RendererPolygon>		Polygons;
	vector<RendererPolygon>		AnimatedPolygons;
	__int32						StartVertex;
	__int32						NumTriangles;
	__int32						NumVertices;
	__int32						NumIndices;
	ID3D11Buffer*				GetVertexBuffer();
	ID3D11Buffer*				GetIndexBuffer();
	void						CleanResources();
};

#endif
