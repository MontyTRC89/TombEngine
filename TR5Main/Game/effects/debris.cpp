#include "framework.h"
#include "effects\debris.h"
#include "level.h"
#include "setup.h"
#include "control.h"
#include "trmath.h"
#include "prng.h"

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
	for (auto& frag : DebrisFragments) {
		if (!frag.active) {
			return &frag;
		}
	}
	return nullptr;
}

void ShatterObject(SHATTER_ITEM* item, MESH_INFO* mesh, int num,short roomNumber,int noZXVel)
{
	MESH* meshPtr = nullptr;
	RendererMesh* fragmentsMesh;
	short yRot = 0;
	Vector3 pos;
	bool isStatic;
	if (mesh) {
		isStatic = false;
		meshPtr = &g_Level.Meshes[StaticObjects[mesh->staticNumber].meshNumber];
		yRot = mesh->yRot;
		pos = Vector3(mesh->x, mesh->y, mesh->z);
	}
	else {
		isStatic = true;
		meshPtr = item->meshp;
		yRot = item->yRot;
		pos = Vector3(item->sphere.x, item->sphere.y, item->sphere.z);
	}
	fragmentsMesh = g_Renderer.getRendererMeshFromTrMesh(nullptr, meshPtr, num, 0, 0);
	for (auto& renderBucket : fragmentsMesh->buckets) {
		vector<RendererVertex>& meshVertices = renderBucket.Vertices;
		for (int i = 0; i < renderBucket.Indices.size(); i += 3)
		{
			DebrisFragment* fragment = GetFreeDebrisFragment();
			if (!fragment) {
				break;
			}
			if (!fragment->active) {
				Matrix rotationMatrix = Matrix::CreateFromYawPitchRoll(TO_RAD(yRot), 0, 0);
				RendererVertex vtx0 = meshVertices.at(renderBucket.Indices[i]);
				RendererVertex vtx1 = meshVertices.at(renderBucket.Indices[i + 1]);
				RendererVertex vtx2 = meshVertices.at(renderBucket.Indices[i + 2]);
				//Take the average of all 3 local positions
				Vector3 localPos = (vtx0.Position + vtx1.Position + vtx2.Position) / 3;
				vtx0.Position -= localPos;
				vtx1.Position -= localPos;
				vtx2.Position -= localPos;
				Vector3 worldPos = Vector3::Transform(localPos, rotationMatrix);
				fragment->worldPosition = worldPos + pos;
				fragment->mesh.vertices[0] = vtx0;
				fragment->mesh.vertices[1] = vtx1;
				fragment->mesh.vertices[2] = vtx2;
				fragment->mesh.blendMode = renderBucket.blendMode;
				fragment->mesh.tex = renderBucket.texture;
				fragment->isStatic = isStatic;
				fragment->active = true;
				fragment->terminalVelocity = 1024;
				fragment->gravity = Vector3(0, 7, 0);
				fragment->restitution = 0.6f;
				fragment->friction = 0.6f;
				fragment->linearDrag = .99f;
				fragment->angularVelocity = Vector3(generateFloat(-1, 1) * 0.39, generateFloat(-1, 1) * 0.39, generateFloat(-1, 1) * 0.39);
				fragment->angularDrag = generateFloat(0.9f, 0.999f);
				fragment->velocity = CalculateFragmentImpactVelocity(fragment->worldPosition, ShatterImpactData.impactDirection, ShatterImpactData.impactLocation);
				fragment->roomNumber = roomNumber;
				fragment->numBounces = 0;
			}
		}
	}
	delete fragmentsMesh;
}

Vector3 CalculateFragmentImpactVelocity(Vector3 fragmentWorldPosition, Vector3 impactDirection, Vector3 impactLocation)
{
	Vector3 radiusVector = (fragmentWorldPosition - impactLocation);
	Vector3 radiusNormVec = radiusVector;
	radiusNormVec.Normalize();
	float radiusStrenght =  1-((fragmentWorldPosition - impactLocation).Length() / 1024);
	radiusStrenght = fmax(radiusStrenght, 0);
	Vector3 radiusRandomVector = Vector3(generateFloat(-0.2, 0.2f), generateFloat(-0.2, 0.2f), generateFloat(-0.2, 0.2f)) + radiusNormVec;
	radiusRandomVector.Normalize();
	Vector3 radiusVelocity = radiusRandomVector * radiusStrenght*40;
	Vector3 impactDirectionVelocity = (impactDirection + Vector3(generateFloat(-0.2, 0.2f), generateFloat(-0.2, 0.2f), generateFloat(-0.2, 0.2f))) * 80 ;
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
			deb.rotation *= Quaternion::CreateFromYawPitchRoll(deb.angularVelocity.x,deb.angularVelocity.y,deb.angularVelocity.z);
			deb.worldPosition += deb.velocity;
			deb.angularVelocity *= deb.angularDrag;

			roomNumber = deb.roomNumber;
			floor = GetFloor(deb.worldPosition.x, deb.worldPosition.y, deb.worldPosition.z,&roomNumber);

			if (deb.worldPosition.y < floor->ceiling*256)
			{
				if (floor->skyRoom != NO_ROOM)
					deb.roomNumber = floor->skyRoom;
			}

			if (deb.worldPosition.y > floor->floor*256)
			{
				if (floor->pitRoom != NO_ROOM)
					deb.roomNumber = floor->pitRoom;

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
