#ifdef OLDCODE

#include "RendererObject.h"
#include "RendererMesh.h"

#include "Enums.h"

#include <stdio.h>
#include <D3D11.h>
#include <DxErr.h>
#include <vector>

class RendererBucket;

RendererObject::RendererObject(ID3D11Device* device, __int32 id, __int32 numMeshes)
{
	m_device = device;
	m_id = id;

	ObjectMeshes.reserve(numMeshes);
	AnimationTransforms.reserve(numMeshes);
	BindPoseTransforms.reserve(numMeshes);

	for (__int32 i = 0; i < NUM_BUCKETS; i++)
		HasDataInBucket[i] = false;
}

/*RendererObject::~RendererObject()
{
	for (__int32 i = 0; i < ObjectMeshes.size(); i++)
		delete ObjectMeshes[i];

//	for (vector<RendererMesh*>::iterator it = ObjectMeshes.begin(); it != ObjectMeshes.end(); ++it)
//		delete (*it);
	//ObjectMeshes.clear();

	/*for (__int32 i = 0; i < LinearizedBones.size(); i++)
		delete LinearizedBones[i];

	//for (vector<RendererBone*>::iterator it = LinearizedBones.begin(); it != LinearizedBones.end(); ++it)
	//	delete (*it);
	//LinearizedBones.clear();
}*/

void RendererObject::CleanResources()
{
	for (__int32 i = 0; i < ObjectMeshes.size(); i++)
		for (__int32 j = 0; j < NUM_BUCKETS; j++)
		{
			//ObjectMeshes[i]->GetBucket(j)->CleanResources();
		}
}

__int32 RendererObject::GetId()
{
	return m_id;
}

void RendererObject::AverageNormals()
{

}

#endif