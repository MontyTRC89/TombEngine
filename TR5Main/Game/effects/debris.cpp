#include "framework.h"
#include "Game/effects/debris.h"

#include "Game/control/control.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using std::vector;
using namespace TEN::Renderer;
using namespace TEN::Math::Random;

ShatterImpactInfo ShatterImpactData;
SHATTER_ITEM ShatterItem;
short SmashedMeshCount;
MESH_INFO* SmashedMesh[32];
short SmashedMeshRoom[32];
vector<DebrisFragment> DebrisFragments = vector<DebrisFragment>(MAX_DEBRIS);

DebrisFragment* GetFreeDebrisFragment()
{
	for (auto& frag : DebrisFragments) 
	{
		if (!frag.active)
		{
			return &frag;
		}
	}
	return nullptr;
}

void ShatterObject(SHATTER_ITEM* item, MESH_INFO* mesh, int num, short roomNumber, int noZXVel)
{
	int meshIndex = 0;
	RendererMesh* fragmentsMesh;
	short yRot = 0;
	Vector3 pos;
	bool isStatic;

	if (mesh) 
	{
		isStatic = false;
		meshIndex = StaticObjects[mesh->staticNumber].meshNumber;
		yRot = mesh->pos.yRot;
		pos = Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
	}
	else 
	{
		isStatic = true;
		meshIndex = item->meshIndex;
		yRot = item->yRot;
		pos = Vector3(item->sphere.x, item->sphere.y, item->sphere.z);
	}

	fragmentsMesh = g_Renderer.getMesh(meshIndex);
	
	for (auto& renderBucket : fragmentsMesh->buckets) 
	{
		vector<RendererVertex>& meshVertices = renderBucket.Vertices;

		for (int i = 0; i < renderBucket.Indices.size(); i += 3)
		{
			
		}
	}
}

Vector3 CalculateFragmentImpactVelocity(Vector3 fragmentWorldPosition, Vector3 impactDirection, Vector3 impactLocation)
{
	Vector3 radiusVector = (fragmentWorldPosition - impactLocation);
	Vector3 radiusNormVec = radiusVector;
	radiusNormVec.Normalize();
	float radiusStrenght =  1-((fragmentWorldPosition - impactLocation).Length() / 1024);
	radiusStrenght = fmax(radiusStrenght, 0);
	Vector3 radiusRandomVector = Vector3(GenerateFloat(-0.2, 0.2f), GenerateFloat(-0.2, 0.2f), GenerateFloat(-0.2, 0.2f)) + radiusNormVec;
	radiusRandomVector.Normalize();
	Vector3 radiusVelocity = radiusRandomVector * radiusStrenght*40;
	Vector3 impactDirectionVelocity = (impactDirection + Vector3(GenerateFloat(-0.2, 0.2f), GenerateFloat(-0.2, 0.2f), GenerateFloat(-0.2, 0.2f))) * 80 ;
	return radiusVelocity + impactDirectionVelocity;
}

void DisableDebris()
{
	for (auto& deb : DebrisFragments)
	{
		deb.active = false;
	}
}

void UpdateDebris()
{
	for (auto& deb : DebrisFragments)
	{
		if (deb.active)
		{
			FLOOR_INFO* floor;
			short roomNumber;

			deb.velocity *= deb.linearDrag;
			deb.velocity += deb.gravity;
			deb.velocity = XMVector3ClampLength(deb.velocity, 0, deb.terminalVelocity);
			deb.rotation *= Quaternion::CreateFromYawPitchRoll(deb.angularVelocity.x, deb.angularVelocity.y, deb.angularVelocity.z);
			deb.worldPosition += deb.velocity;
			deb.angularVelocity *= deb.angularDrag;

			roomNumber = deb.roomNumber;
			floor = GetFloor(deb.worldPosition.x, deb.worldPosition.y, deb.worldPosition.z, &roomNumber);

			if (deb.worldPosition.y < floor->CeilingHeight(deb.worldPosition.x, deb.worldPosition.z))
			{
				auto roomNumber = floor->RoomAbove(deb.worldPosition.x, deb.worldPosition.y, deb.worldPosition.z).value_or(NO_ROOM);
				if (roomNumber != NO_ROOM)
					deb.roomNumber = roomNumber;
			}

			if (deb.worldPosition.y > floor->FloorHeight(deb.worldPosition.x, deb.worldPosition.z))
			{
				auto roomNumber = floor->RoomBelow(deb.worldPosition.x, deb.worldPosition.y, deb.worldPosition.z).value_or(NO_ROOM);
				if (roomNumber != NO_ROOM)
					deb.roomNumber = roomNumber;

				if (deb.numBounces > 3)
				{
					deb.active = false;
					continue;
				}
				
				deb.velocity.y *= -deb.restitution;
				deb.velocity.x *= deb.friction;
				deb.velocity.z *= deb.friction;
				deb.numBounces++;
			}
		}
	}
}
