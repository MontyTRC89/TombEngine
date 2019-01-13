#pragma once
#ifdef OLDCODE

#include "Enums.h"

#include <stdio.h>
#include <D3D11.h>
#include <DxErr.h>
#include <vector>
#include <memory>
#include <SimpleMath.h>

using namespace std;
using namespace DirectX::SimpleMath;

struct RendererBone;
struct RendererMesh;
struct RendererPolygon;
struct RendererVertex;

class RendererObject
{
private:
	ID3D11Device*				m_device;
	__int32						m_id;

public:
	RendererObject(ID3D11Device* device, __int32 id, __int32 numMeshes);
	//~RendererObject();
	void						AverageNormals();

	vector<shared_ptr<RendererMesh>>		ObjectMeshes;
	shared_ptr<RendererBone>				Skeleton;
	vector<Matrix>							AnimationTransforms;
	vector<Matrix>							BindPoseTransforms;
	vector<shared_ptr<RendererBone>>		LinearizedBones;
	__int32						GetId();
	bool						HasDataInBucket[NUM_BUCKETS];
	bool						HasDataInAnimatedBucket[NUM_BUCKETS];
	bool						DoNotDraw;
	__int32						NumVertices;
	void						CleanResources();

	//bool(Renderer::*DrawRoutine)(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass);
};

#endif