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
	MESH* fragmentsMesh;
	short yRot = 0;
	Vector3 pos;
	bool isStatic;

	if (mesh)
	{
		isStatic = true;
		meshIndex = StaticObjects[mesh->staticNumber].meshNumber;
		yRot = mesh->pos.yRot;
		pos = Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
	}
	else
	{
		isStatic = false;
		meshIndex = item->meshIndex;
		yRot = item->yRot;
		pos = Vector3(item->sphere.x, item->sphere.y, item->sphere.z);
	}

	fragmentsMesh = &g_Level.Meshes[meshIndex];

	for (auto& renderBucket : fragmentsMesh->buckets)
	{
		for (int i = 0; i < renderBucket.polygons.size(); i++)
		{
			POLYGON* poly = &renderBucket.polygons[i];
			int indices[6];

			if (poly->shape == SHAPE_RECTANGLE)
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 3;
				indices[3] = 2;
				indices[4] = 3;
				indices[5] = 1;
			}
			else
			{
				indices[0] = 0;
				indices[1] = 1;
				indices[2] = 2;
				indices[3] = -1;
				indices[4] = -1;
				indices[5] = -1;
			}

			for (int j = 0; j < (poly->shape == SHAPE_RECTANGLE ? 2 : 1); j++)
			{
				DebrisFragment* fragment = GetFreeDebrisFragment();

				if (!fragment->active)
				{
					Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(TO_RAD(yRot), 0, 0);

					Vector3 pos1 = fragmentsMesh->positions[poly->indices[indices[j * 3 + 0]]];
					Vector3 pos2 = fragmentsMesh->positions[poly->indices[indices[j * 3 + 1]]];
					Vector3 pos3 = fragmentsMesh->positions[poly->indices[indices[j * 3 + 2]]];

					Vector2 uv1 = poly->textureCoordinates[indices[j * 3 + 0]];
					Vector2 uv2 = poly->textureCoordinates[indices[j * 3 + 1]];
					Vector2 uv3 = poly->textureCoordinates[indices[j * 3 + 2]];

					Vector3 normal1 = poly->normals[indices[j * 3 + 0]];
					Vector3 normal2 = poly->normals[indices[j * 3 + 1]];
					Vector3 normal3 = poly->normals[indices[j * 3 + 2]];

					//Take the average of all 3 local positions
					Vector3 localPos = (pos1 + pos2 + pos3) / 3;
					Vector3 worldPos = Vector3::Transform(localPos, rotationMatrix);

					fragment->worldPosition = worldPos + pos;

					fragment->mesh.Positions[0] = pos1 - localPos;
					fragment->mesh.Positions[1] = pos2 - localPos;
					fragment->mesh.Positions[2] = pos3 - localPos;

					fragment->mesh.TextureCoordinates[0] = uv1;
					fragment->mesh.TextureCoordinates[1] = uv2;
					fragment->mesh.TextureCoordinates[2] = uv3;

					fragment->mesh.Normals[0] = normal1;
					fragment->mesh.Normals[1] = normal2;
					fragment->mesh.Normals[2] = normal3;

					fragment->mesh.blendMode = BLENDMODE_OPAQUE;
					fragment->mesh.tex = renderBucket.texture;

					fragment->isStatic = isStatic;
					fragment->active = true;
					fragment->terminalVelocity = 1024;
					fragment->gravity = Vector3(0, 7, 0);
					fragment->restitution = 0.6f;
					fragment->friction = 0.6f;
					fragment->linearDrag = .99f;
					fragment->angularVelocity = Vector3(GenerateFloat(-1, 1) * 0.39, GenerateFloat(-1, 1) * 0.39, GenerateFloat(-1, 1) * 0.39);
					fragment->angularDrag = GenerateFloat(0.9f, 0.999f);
					fragment->velocity = CalculateFragmentImpactVelocity(fragment->worldPosition, ShatterImpactData.impactDirection, ShatterImpactData.impactLocation);
					fragment->roomNumber = roomNumber;
					fragment->numBounces = 0;
				}
			}
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
