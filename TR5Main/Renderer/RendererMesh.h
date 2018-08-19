#pragma once
#include "Structs.h"
#include "RendererBucket.h"

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>

class RendererMesh
{
private:
	RendererBucket**			m_buckets;
	LPDIRECT3DDEVICE9			m_device;

public:
	RendererMesh(LPDIRECT3DDEVICE9 device);
	~RendererMesh();

	vector<D3DXVECTOR3>			Positions;
	RendererBucket*				GetBucket(__int32 bucketIndex);
};