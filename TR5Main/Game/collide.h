#pragma once
#include "phd_global.h"
#include "level.h"

// used by coll->badPos
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->badNeg
#define NO_BAD_NEG NO_HEIGHT

#define MAX_COLLIDED_OBJECTS 1024

#define COLLISION_CHECK_DISTANCE 6144

struct COLL_RESULT
{
	FLOOR_INFO* Block;
	FLOOR_INFO* BottomBlock;
	int RoomNumber;

	int FloorHeight;
	int HeightType;

	int TiltX;
	int TiltZ;
	int SplitFloor;

	int CeilingHeight;
	int SplitCeiling;
};

struct COLL_POSITION
{
	int Floor;
	int Ceiling;
	int Type;
	int SplitFloor;
	int SplitCeiling;
};

struct COLL_INFO
{
	COLL_POSITION middle;       // mid
	COLL_POSITION middleLeft;   // left
	COLL_POSITION middleRight;  // right
	COLL_POSITION front;        // front
	COLL_POSITION frontLeft;    // left2
	COLL_POSITION frontRight;   // right2

	int radius;
	int badPos;
	int badNeg;
	int badCeiling;
	PHD_VECTOR shift;
	PHD_VECTOR old;
	short oldAnimState;
	short oldAnimNumber;
	short oldFrameNumber;
	short facing;
	short quadrant;
	short collType; // CT_enum
	signed char tiltX;
	signed char tiltZ;
	bool hitStatic;
	bool splat;
	bool slopesAreWalls;
	bool slopesArePits;
	bool lavaIsPit;
	bool enableBaddiePush;
	bool enableSpaz;
};

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

constexpr auto MAX_ITEMS = 1024;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);

extern BOUNDING_BOX GlobalCollisionBounds;
extern ITEM_INFO* CollidedItems[MAX_ITEMS];
extern MESH_INFO* CollidedMeshes[MAX_ITEMS];

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void UpdateLaraRoom(ITEM_INFO* item, int height);
COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z);
COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber);
COLL_RESULT GetCollisionResult(ITEM_INFO* item);
int FindGridShift(int x, int z); 
int TestBoundsCollideStatic(ITEM_INFO* item, MESH_INFO* mesh, int radius);
int ItemPushItem(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush);
int ItemPushStatic(ITEM_INFO* l, MESH_INFO* mesh, COLL_INFO* coll);
void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
int TestLaraPosition(OBJECT_COLLISION_BOUNDS* bounds, ITEM_INFO* item, ITEM_INFO* l);
int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
int MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight);
void GetObjectCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight);
void DoProjectileDynamics(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
void DoObjectCollision(ITEM_INFO* item, COLL_INFO* coll);
bool ItemNearLara(PHD_3DPOS* pos, int radius);
bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius);
bool SnapToQuadrant(short& angle, int interval);
int GetQuadrant(short angle);
bool SnapToDiagonal(short& angle, int interval);
void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide = 1);
Vector2 GetDiagonalIntersect(int xPos, int zPos, int splitType, int radius, short yRot); // find xPos, zPos that intersects with diagonal on sector
Vector2 GetOrthogonalIntersect(int xPos, int zPos, int radius, short yRot); // find xPos, zPos near sector bound, offset by radius;
bool CollideSolidBounds(ITEM_INFO* item, BOUNDING_BOX box, PHD_3DPOS pos, COLL_INFO* coll);
void CollideSolidStatics(ITEM_INFO* item, COLL_INFO* coll);