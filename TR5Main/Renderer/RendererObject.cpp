#include "RendererObject.h"
#include "Enums.h"

#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <vector>

class RendererBucket;

RendererObject::RendererObject(LPDIRECT3DDEVICE9 device, __int32 id, __int32 numMeshes)
{
	m_device = device;
	m_id = id;

	ObjectMeshes.reserve(numMeshes);
	AnimationTransforms.reserve(numMeshes);
	BindPoseTransforms.reserve(numMeshes);

	for (__int32 i = 0; i < NUM_BUCKETS; i++)
		HasDataInBucket[i] = false;
}

RendererObject::~RendererObject()
{
	for (vector<RendererMesh*>::iterator it = ObjectMeshes.begin(); it != ObjectMeshes.end(); ++it)
		delete (*it);
	ObjectMeshes.clear();

	for (vector<RendererBone*>::iterator it = LinearizedBones.begin(); it != LinearizedBones.end(); ++it)
		delete (*it);
	LinearizedBones.clear();

	Skeleton = NULL;
}

__int32 RendererObject::GetId()
{
	return m_id;
}

void RendererObject::AverageNormals()
{

}