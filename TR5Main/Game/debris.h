#pragma once

#include "..\Global\global.h"
//#define ShatterObject ((void (__cdecl*)(SHATTER_ITEM*, MESH_INFO*, short, short, int)) 0x0041D6B0)
void Inject_Debris();
extern struct ShatterImpactInfo {
	Vector3 impactDirection;
	Vector3 impactLocation;
};
extern ShatterImpactInfo shatterImpactInfo;
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

void shatterObject(SHATTER_ITEM* item, MESH_INFO* mesh, int num, short roomNumber, int noZXVel);
DebrisFragment* getFreeDebrisFragment();
extern vector<DebrisFragment> debrisFragments;
Vector3 calculateFragmentImpactVelocity(Vector3 fragmentWorldPosition, Vector3 impactDirection, Vector3 impactLocation);
void updateDebris();