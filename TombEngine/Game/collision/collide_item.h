#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ItemInfo;
struct CollisionInfo;
class FloorInfo;
struct MESH_INFO;

constexpr auto MAX_COLLIDED_OBJECTS = 1024;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

constexpr auto VEHICLE_COLLISION_TERMINAL_VELOCITY = 30;

extern BOUNDING_BOX GlobalCollisionBounds;
extern ItemInfo* CollidedItems[MAX_COLLIDED_OBJECTS];
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

void GenericSphereBoxCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool GetCollidedObjects(ItemInfo* collidingItem, int radius, bool onlyVisible, ItemInfo** collidedItems, MESH_INFO** collidedMeshes, bool ignoreLara);
bool TestWithGlobalCollisionBounds(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll);
void TestForObjectOnLedge(ItemInfo* item, CollisionInfo* coll);

bool TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ItemInfo* item, ItemInfo* laraItem);
bool AlignLaraPosition(Vector3Int* vec, ItemInfo* item, ItemInfo* laraItem);
bool MoveLaraPosition(Vector3Int* pos, ItemInfo* item, ItemInfo* laraItem);

bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ItemInfo* target, int radius);

bool Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angleAdd);

bool TestBoundsCollide(ItemInfo* item, ItemInfo* laraItem, int radius);
bool TestBoundsCollideStatic(ItemInfo* item, MESH_INFO* mesh, int radius);
bool ItemPushItem(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll, bool spasmEnabled, char bigPush);
bool ItemPushStatic(ItemInfo* laraItem, MESH_INFO* mesh, CollisionInfo* coll);

bool CollideSolidBounds(ItemInfo* item, BOUNDING_BOX box, PHD_3DPOS pos, CollisionInfo* coll);
void CollideSolidStatics(ItemInfo* item, CollisionInfo* coll);

void AIPickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ObjectCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CreatureCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void TrapCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ItemInfo* item, CollisionInfo* coll);
