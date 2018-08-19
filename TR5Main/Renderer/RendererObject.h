#pragma once
#include "Enums.h"

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>

using namespace std;

struct RendererBone;
struct RendererMesh;
struct RendererPolygon;
struct RendererVertex;

class RendererObject
{
private:
	LPDIRECT3DDEVICE9			m_device;
	__int32						m_id;

public:
	RendererObject(LPDIRECT3DDEVICE9 device, __int32 id, __int32 numMeshes);
	~RendererObject();

	vector<RendererMesh*>		ObjectMeshes;
	RendererBone*				Skeleton;
	D3DXMATRIX*					AnimationTransforms;
	D3DXMATRIX*					BindPoseTransforms;
	vector<RendererBone*>		LinearizedBones;
	__int32						GetId();
	bool						HasDataInBucket[NUM_BUCKETS];
};