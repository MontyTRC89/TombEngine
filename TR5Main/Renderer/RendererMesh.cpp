#ifdef OLDCODE


#include "RendererMesh.h"
#include "Enums.h"

#include <stdio.h>
#include <D3D11.h>
#include <DxErr.h>
#include <vector>

struct RendererVertex;
struct RendererPolygon;

RendererMesh::RendererMesh(ID3D11Device* device)
{
	m_device = device;

	/*for (__int32 i = 0; i < NUM_BUCKETS; i++)
		m_buckets[i] = make_shared<RendererBucket>(device);
	 
	for (__int32 i = 0; i < NUM_BUCKETS; i++)
		m_animatedBuckets[i] = make_shared<RendererBucket>(device);*/
}

RendererMesh::~RendererMesh()
{
	/*for (__int32 i = 0; i < NUM_BUCKETS; i++)
		delete m_buckets[i];

	for (__int32 i = 0; i < NUM_BUCKETS; i++)
		delete m_animatedBuckets[i];*/
}

RendererBucket* RendererMesh::GetBucket(__int32 bucketIndex)
{
	return NULL;
	//return m_buckets[bucketIndex].get();
}

RendererBucket* RendererMesh::GetAnimatedBucket(__int32 bucketIndex)
{
	return NULL;
	//return m_animatedBuckets[bucketIndex].get();
}

#endif