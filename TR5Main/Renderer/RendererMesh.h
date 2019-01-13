#pragma once
#include "Structs.h"
#include "RendererBucket.h"

#include <stdio.h>
#include <d3d9.h>
#ifdef OLDCODE


#include <d3dx9.h>
#include <DxErr.h>
#include <vector>
#include <memory>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class RendererMesh
{
private:
	//shared_ptr<RendererBucket>				m_buckets[NUM_BUCKETS];
	//shared_ptr<RendererBucket>				m_animatedBuckets[NUM_BUCKETS];
	ID3D11Device*							m_device;

public:
	RendererMesh(ID3D11Device* device);
	~RendererMesh();

	vector<Vector3>				Positions;
	RendererBucket*				GetBucket(__int32 bucketIndex);
	RendererBucket*				GetAnimatedBucket(__int32 bucketIndex);
};

#endif