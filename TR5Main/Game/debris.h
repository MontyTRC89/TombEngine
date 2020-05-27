#pragma once

#include "global.h"

struct ShatterImpactInfo {
	Vector3 impactDirection;
	Vector3 impactLocation;
};

struct DebrisMesh {
	RENDERER_BUCKETS bucket;
	array<RendererVertex, 3> vertices;
};

struct DebrisFragment {
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
};

extern SHATTER_ITEM ShatterItem;
extern vector<DebrisFragment> DebrisFragments;
extern ShatterImpactInfo ShatterImpactData;
extern short SmashedMeshCount;
extern MESH_INFO* SmashedMesh[32];
extern short SmashedMeshRoom[32];

void ShatterObject(SHATTER_ITEM* item, MESH_INFO* mesh, int num, short roomNumber, int noZXVel);
DebrisFragment* GetFreeDebrisFragment();
Vector3 CalculateFragmentImpactVelocity(Vector3 fragmentWorldPosition, Vector3 impactDirection, Vector3 impactLocation);
void UpdateDebris();