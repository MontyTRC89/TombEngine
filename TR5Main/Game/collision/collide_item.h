#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ITEM_INFO;
struct COLL_INFO;
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

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
bool GetCollidedObjects(ITEM_INFO* collidingItem, int radius, bool onlyVisible, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
bool TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
bool MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);

bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);

bool TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
bool TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius);
bool ItemPushItem(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, bool spazon, char bigpush);
bool ItemPushStatic(ITEM_INFO* l, MESH_INFO* mesh, COLL_INFO* coll);

bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll);
void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll);

void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ITEM_INFO* item, COLL_INFO* coll);
