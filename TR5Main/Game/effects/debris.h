#pragma once
#include <sphere.h>
#include <Renderer11.h>
#include <newtypes.h>
#include <level.h>
#include "RendererVertex.h"
#define MAX_DEBRIS 256

struct ILIGHT
{
	short x;
	short y;
	short z;
	short pad1;
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char pad;
};

struct ITEM_LIGHT
{
	ILIGHT light[4];
};

struct SHATTER_ITEM
{
	SPHERE sphere;
	ITEM_LIGHT* il;
	MESH* meshp;
	int bit;
	short yRot;
	short flags;
};

struct ShatterImpactInfo
{
	Vector3 impactDirection;
	Vector3 impactLocation;
};

struct DebrisMesh
{
	BLEND_MODES blendMode;
	std::array<TEN::Renderer::RendererVertex, 3> vertices;
	int tex;
};

struct DebrisFragment
{
	DebrisMesh mesh;
	Quaternion rotation;
	Vector3 angularVelocity;
	Vector3 worldPosition;
	Vector3  velocity;
	Vector3 gravity;
	float terminalVelocity;
	float linearDrag;
	float angularDrag;
	float friction;
	float restitution;
	uint32_t roomNumber;
	uint32_t numBounces;
	bool active;
	bool isStatic;
};

struct DEBRIS_STRUCT
{
	void* textInfo;
	int x;
	int y;
	int z;
	short xyzOffsets1[3];
	short dir;
	short xyzOffsets2[3];
	short speed;
	short xyzOffsets3[3];
	short yVel;
	short gravity;
	short roomNumber;
	byte on;
	byte xRot;
	byte yRot;
	byte r;
	byte g;
	byte b;
	byte pad[22];
};

extern SHATTER_ITEM ShatterItem;
extern std::vector<DebrisFragment> DebrisFragments;
extern ShatterImpactInfo ShatterImpactData;
extern short SmashedMeshCount;
extern MESH_INFO* SmashedMesh[32];
extern short SmashedMeshRoom[32];

void ShatterObject(SHATTER_ITEM* item, MESH_INFO* mesh, int num, short roomNumber, int noZXVel);
DebrisFragment* GetFreeDebrisFragment();
Vector3 CalculateFragmentImpactVelocity(Vector3 fragmentWorldPosition, Vector3 impactDirection, Vector3 impactLocation);
void DisableDebris();
void UpdateDebris();