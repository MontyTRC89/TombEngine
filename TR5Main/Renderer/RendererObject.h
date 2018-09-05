#pragma once
#include "Enums.h"

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>
#include <memory>

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
	//~RendererObject();
	void						AverageNormals();

	vector<shared_ptr<RendererMesh>>		ObjectMeshes;
	shared_ptr<RendererBone>				Skeleton;
	vector<D3DXMATRIX>			AnimationTransforms;
	vector<D3DXMATRIX>			BindPoseTransforms;
	vector<shared_ptr<RendererBone>>		LinearizedBones;
	__int32						GetId();
	bool						HasDataInBucket[NUM_BUCKETS];
	bool						HasDataInAnimatedBucket[NUM_BUCKETS];
	bool						DoNotDraw;
	__int32						NumVertices;
	void						CleanResources();

	//bool(Renderer::*DrawRoutine)(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
};