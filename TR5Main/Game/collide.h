#pragma once
#include "phd_global.h"
#include "level.h"

// used by coll->badPos
#define NO_BAD_POS (-NO_HEIGHT)
// used by coll->badNeg
#define NO_BAD_NEG NO_HEIGHT

enum COLL_TYPE
{
	CT_NONE = 0,			   // 0x00
	CT_FRONT = (1 << 0),  // 0x01
	CT_LEFT = (1 << 1),  // 0x02
	CT_RIGHT = (1 << 2),  // 0x04
	CT_TOP = (1 << 3),  // 0x08
	CT_TOP_FRONT = (1 << 4),  // 0x10
	CT_CLAMP = (1 << 5)   // 0x20
};

enum HEIGHT_TYPES
{
	WALL,
	SMALL_SLOPE,
	BIG_SLOPE,
	DIAGONAL,
	SPLIT_TRI
};

enum HEADINGS
{
	NORTH,
	EAST,
	SOUTH,
	WEST
};

enum OCTANTS
{
	O_NORTH,
	O_NORTHEAST,
	O_EAST,
	O_SOUTHEAST,
	O_SOUTH,
	O_SOUTHWEST,
	O_WEST,
	O_NORTHWEST
}

struct BOUNDING_BOX
{
	short X1;
	short X2;
	short Y1;
	short Y2;
	short Z1;
	short Z2;
};

struct COLL_FLOOR
{
	int floor;
	int ceiling;
	int type;
	int splitFloor;
	int splitCeiling;
};

struct COLL_INFO
{
	/*
	COLL_FLOOR middle;        // mid
	COLL_FLOOR middle_left;   // left
	COLL_FLOOR middle_right;  // right
	COLL_FLOOR front;         // front
	COLL_FLOOR front_left;    // left2
	COLL_FLOOR front_right;   // right2
	*/
	int midFloor;
	int midCeiling;
	int midType;
	int midSplitFloor;
	int midSplitCeil;

	int frontFloor;
	int frontCeiling;
	int frontType;
	int frontSplitFloor;
	int frontSplitCeil;

	int leftFloor;
	int leftCeiling;
	int leftType;
	int leftSplitFloor;
	int leftSplitCeil;

	int rightFloor;
	int rightCeiling;
	int rightType;
	int rightSplitFloor;
	int rightSplitCeil;

	int leftFloor2;
	int leftCeiling2;
	int leftType2;
	int leftSplitFloor2;
	int leftSplitCeil2;

	int rightFloor2;
	int rightCeiling2;
	int rightType2;
	int rightSplitFloor2;
	int rightSplitCeil2;

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
	short octant;
	short collType; // CT_enum
	short* trigger;
	signed char tiltX;
	signed char tiltZ;
	bool hitByBaddie;
	bool hitStatic;
	bool slopesAreWalls;
	bool slopesArePits;
	bool lavaIsPit;
	bool enableBaddiePush;
	bool enableSpaz;
	bool hitCeiling;
};

extern BOUNDING_BOX GlobalCollisionBounds;
constexpr auto MAX_ITEMS = 1024;
extern ITEM_INFO* CollidedItems[MAX_ITEMS];
extern MESH_INFO* CollidedMeshes[MAX_ITEMS];

void GenericSphereBoxCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int CollideStaticObjects(COLL_INFO* coll, int x, int y, int z, short roomNumber, int hite);
int GetCollidedObjects(ITEM_INFO* collidingItem, int radius, int flag1, ITEM_INFO** collidedItems, MESH_INFO** collidedMeshes, int flag2);
int TestWithGlobalCollisionBounds(ITEM_INFO* item, ITEM_INFO* lara, COLL_INFO* coll);
void TrapCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void TestForObjectOnLedge(ITEM_INFO* item, COLL_INFO* coll);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void UpdateLaraRoom(ITEM_INFO* item, int height);
short GetTiltType(FLOOR_INFO* floor, int x, int y, int z);
int FindGridShift(int x, int z);
int TestBoundsCollideStatic(short* bounds, PHD_3DPOS* pos, int radius);
int ItemPushLaraStatic(ITEM_INFO* item, short* bounds, PHD_3DPOS* pos, COLL_INFO* coll);
void AIPickupCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void ObjectCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c);
void AlignLaraPosition(PHD_VECTOR* vec, ITEM_INFO* item, ITEM_INFO* l);
void TriggerLaraBlood();
int ItemPushLara(ITEM_INFO* item, ITEM_INFO* l, COLL_INFO* coll, int spazon, char bigpush);
int TestLaraPosition(short* bounds, ITEM_INFO* item, ITEM_INFO* l);
int Move3DPosTo3DPos(PHD_3DPOS* src, PHD_3DPOS* dest, int velocity, short angAdd);
int MoveLaraPosition(PHD_VECTOR* pos, ITEM_INFO* item, ITEM_INFO* l);
int TestBoundsCollide(ITEM_INFO* item, ITEM_INFO* l, int radius);
void CreatureCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void GetCollisionInfo(COLL_INFO* coll, int xPos, int yPos, int zPos, int roomNumber, int objectHeight);
void LaraBaddieCollision(ITEM_INFO* item, COLL_INFO* coll);
void CalcItemToFloorRotation(ITEM_INFO* item, short rotY, int radiusZ, int radiusX); // new function for rotating item along XZ slopes
bool SnapToQuadrant(short& angle, int interval);
int GetQuadrant(short angle);
bool SnapToDiagonal(short& angle, int interval);
int GetOctant(short angle);
Vector2 GetDiagonalIntersect(int xPos, int zPos, int splitType, int radius, short yRot); // finds xPos, zPos that intersects with diagonal on sector
void RotateBoundingBox(array<float, 4>& box, short angle);
