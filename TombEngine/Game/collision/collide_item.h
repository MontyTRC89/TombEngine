#pragma once
#include "Math/Math.h"

using std::pair;

class FloorInfo;
struct CollisionInfo;
struct ItemInfo;
struct MESH_INFO;

constexpr auto MAX_COLLIDED_OBJECTS = 1024;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

constexpr auto VEHICLE_COLLISION_TERMINAL_VELOCITY = 30.0f;

extern GameBoundingBox GlobalCollisionBounds;
extern ItemInfo* CollidedItems[MAX_COLLIDED_OBJECTS];
extern MESH_INFO* CollidedMeshes[MAX_COLLIDED_OBJECTS];

struct ObjectCollisionBounds
{
	GameBoundingBox				   BoundingBox		= GameBoundingBox::Zero;
	pair<EulerAngles, EulerAngles> OrientConstraint = {};
};

void GenericSphereBoxCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool GetCollidedObjects(ItemInfo* collidingItem, int radius, bool onlyVisible, ItemInfo** collidedItems, MESH_INFO** collidedMeshes, bool ignoreLara);
bool TestWithGlobalCollisionBounds(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll);
void TestForObjectOnLedge(ItemInfo* item, CollisionInfo* coll);

bool TestLaraPosition(const ObjectCollisionBounds& bounds, ItemInfo* item, ItemInfo* laraItem);
bool AlignLaraPosition(const Vector3i& offset, ItemInfo* item, ItemInfo* laraItem);
bool MoveLaraPosition(const Vector3i& offset, ItemInfo* item, ItemInfo* laraItem);

bool ItemNearLara(const Vector3i& origin, int radius);
bool ItemNearTarget(const Vector3i& origin, ItemInfo* targetEntity, int radius);

bool Move3DPosTo3DPos(ItemInfo* item, Pose& fromPose, const Pose& toPose, int velocity, short turnRate);

bool TestBoundsCollide(ItemInfo* item, ItemInfo* laraItem, int radius);
bool TestBoundsCollideStatic(ItemInfo* item, MESH_INFO* mesh, int radius);
bool ItemPushItem(ItemInfo* item, ItemInfo* laraItem, CollisionInfo* coll, bool spasmEnabled, char bigPush);
bool ItemPushStatic(ItemInfo* laraItem, MESH_INFO* mesh, CollisionInfo* coll);

bool CollideSolidBounds(ItemInfo* item, GameBoundingBox* box, Pose pose, CollisionInfo* coll);
void CollideSolidStatics(ItemInfo* item, CollisionInfo* coll);

void AIPickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ObjectCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CreatureCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void TrapCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ItemInfo* item, CollisionInfo* coll);
