#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ITEM_INFO;
struct CollisionInfo;
struct FLOOR_INFO;
struct MESH_INFO;

constexpr auto MAX_COLLIDED_OBJECTS = 1024;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

extern BOUNDING_BOX GlobalCollisionBounds;
extern ITEM_INFO* CollidedItems[MAX_COLLIDED_OBJECTS];
extern MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

struct OBJECT_COLLISION_BOUNDS
{
	BOUNDING_BOX boundingBox;
	short rotX1;
	short rotX2;
	short rotY1;
	short rotY2;
	short rotZ1;
	short rotZ2;
};

void GenericSphereBoxCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, bool onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* laraItem, CollisionInfo* coll);
void TestForObjectOnLedge(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* laraItem);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* laraItem);
bool MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* laraItem);

bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angleAdd);

bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* laraItem, int radius);
bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius);
bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* laraItem, CollisionInfo* coll, bool spasmEnabled, char bigPush);
bool ItemPushStatic(ITEM_INFO* laraItem, MESH_INFO* mesh, CollisionInfo* coll);

bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, CollisionInfo* coll);
void CollideSolidStatics(ITEM_INFO* item, CollisionInfo* coll);

void AIPickupCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void ObjectCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void CreatureCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void TrapCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ITEM_INFO* item, CollisionInfo* coll);
