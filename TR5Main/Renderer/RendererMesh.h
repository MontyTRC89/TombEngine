#pragma once
#include "Structs.h"
#include "RendererBucket.h"

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>
#include <memory>

class RendererMesh
{
private:
	shared_ptr<RendererBucket>				m_buckets[NUM_BUCKETS];
	shared_ptr<RendererBucket>				m_animatedBuckets[NUM_BUCKETS];
	LPDIRECT3DDEVICE9			m_device;

public:
	RendererMesh(LPDIRECT3DDEVICE9 device);
	~RendererMesh();

	vector<D3DXVECTOR3>			Positions;
	RendererBucket*				GetBucket(__int32 bucketIndex);
	RendererBucket*				GetAnimatedBucket(__int32 bucketIndex);
};