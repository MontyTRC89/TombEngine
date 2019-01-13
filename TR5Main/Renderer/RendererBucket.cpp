#ifdef OLDCODE

#include "RendererBucket.h"
#include "Structs.h"
      
#include <stdio.h>

struct RendererVertex;
 
RendererBucket::RendererBucket(ID3D11Device* device)
{
	m_device = device;

	StartVertex = 0;
	NumTriangles = 0;
	NumVertices = 0;
	NumIndices = 0;

	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
}

RendererBucket::~RendererBucket()
{
	
}

void RendererBucket::CleanResources()
{
	if (m_vertexBuffer != NULL)
		m_vertexBuffer->Release();
	m_vertexBuffer = NULL;

	if (m_indexBuffer != NULL)
		m_indexBuffer->Release();
	m_indexBuffer = NULL;
}
  
bool RendererBucket::UpdateBuffers()
{
	HRESULT res;
	 
	/*if (m_vertexBuffer != NULL)
		m_vertexBuffer->Release();

	if (NumVertices == 0)
		return true;

	res = m_device->CreateVertexBuffer(Vertices.size() * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
					0, D3DPOOL_MANAGED, &m_vertexBuffer, NULL);
	if (res != S_OK)
		return false;

	void* vertices;

	res = m_vertexBuffer->Lock(0, 0, &vertices, 0);
	if (res != S_OK)
		return false;

	memcpy(vertices, Vertices.data(), Vertices.size() * sizeof(RendererVertex));

	res = m_vertexBuffer->Unlock();
	if (res != S_OK)
		return false;

	if (m_indexBuffer != NULL)
		m_indexBuffer->Release();

	if (NumIndices == 0)
		return true;

	res = m_device->CreateIndexBuffer(Indices.size() * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED,
									  &m_indexBuffer, NULL);
	if (res != S_OK)
		return false;

	void* indices;

	res = m_indexBuffer->Lock(0, 0, &indices, 0);
	if (res != S_OK)
		return false;

	memcpy(indices, Indices.data(), Indices.size() * 4);

	m_indexBuffer->Unlock();
	if (res != S_OK)
		return false;*/

	return true;
}

ID3D11Buffer* RendererBucket::GetVertexBuffer()
{
	return m_vertexBuffer;
}

ID3D11Buffer* RendererBucket::GetIndexBuffer()
{
	return m_indexBuffer;
}

#endif