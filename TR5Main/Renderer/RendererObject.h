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
	void						AverageNormals();

	vector<RendererMesh*>		ObjectMeshes;
	RendererBone*				Skeleton;
	D3DXMATRIX*					AnimationTransforms;
	D3DXMATRIX*					BindPoseTransforms;
	vector<RendererBone*>		LinearizedBones;
	__int32						GetId();
	bool						HasDataInBucket[NUM_BUCKETS];
	bool						HasDataInAnimatedBucket[NUM_BUCKETS];
	bool						DoNotDraw;
	__int32						NumVertices;
	//bool(Renderer::*DrawRoutine)(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
};