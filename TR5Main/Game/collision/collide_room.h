#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ITEM_INFO;
struct COLL_INFO;
struct FLOOR_INFO;
struct MESH_INFO;

constexpr auto NO_BAD_POS = (-NO_HEIGHT); // used by coll->Setup.BadHeightDown
constexpr auto NO_BAD_NEG = NO_HEIGHT;    // used by coll->Setup.BadHeightUp
constexpr auto COLLISION_CHECK_DISTANCE = WALL_SIZE * 8;

enum COLL_TYPE
{
	CT_NONE = 0,				// 0x00
	CT_FRONT = (1 << 0),		// 0x01
	CT_LEFT = (1 << 1),			// 0x02
	CT_RIGHT = (1 << 2),		// 0x04
	CT_TOP = (1 << 3),			// 0x08
	CT_TOP_FRONT = (1 << 4),	// 0x10
	CT_CLAMP = (1 << 5)			// 0x20
};

enum class COLL_PROBE_MODE
{
	QUADRANTS,
	FREE_FORWARD,
	FREE_FLAT
};

enum class SPLAT_COLL
{
	NONE,
	WALL,
	STEP
};

enum class CORNER_RESULT
{
	NONE,
	INNER,
	OUTER
};

struct COLL_POSITION
{
	int Floor;
	int Ceiling;
	int Bridge;
	float SplitAngle;
	bool Slope;
	bool DiagonalStep;

	bool HasDiagonalSplit() { return SplitAngle == 45.0f * RADIAN || SplitAngle == 135.0f * RADIAN; }
	bool HasFlippedDiagonalSplit() { return HasDiagonalSplit() && SplitAngle != 45.0f * RADIAN; }
};

struct COLL_RESULT
{
	int RoomNumber;

	FLOOR_INFO* Block;
	FLOOR_INFO* BottomBlock;

	COLL_POSITION Position;

	int TiltX;
	int TiltZ;
};

struct COLL_SETUP
{
	COLL_PROBE_MODE Mode;   // Probe rotation mode

	bool SlopesAreWalls;    // Treat steep slopes as walls
	bool SlopesArePits;     // Treat steep slopes as pits
	bool DeathFlagIsPit;    // Treat death sectors as pits
	bool EnableObjectPush;  // Can be pushed by objects
	bool EnableSpaz;        // Push is treated as hurt
						    
	int   Radius;           // Collision bounds horizontal size
	int   Height;			// Collision bounds vertical size
	short ForwardAngle;     // Forward angle direction
	int   BadHeightDown;    // Borderline step-up height 
	int   BadHeightUp;      // Borderline step-down height
	int   BadCeilingHeight; // Borderline ceiling height

	PHD_VECTOR OldPosition; // Preserve old parameters to restore later
	int OldAnimState;
	int OldAnimNumber;
	int OldFrameNumber;
};

struct COLL_INFO
{
	COLL_SETUP    Setup;    // In parameters

	COLL_POSITION Middle;       
	COLL_POSITION MiddleLeft;   
	COLL_POSITION MiddleRight;  
	COLL_POSITION Front;        
	COLL_POSITION FrontLeft;    
	COLL_POSITION FrontRight;   

	PHD_VECTOR Shift;
	COLL_TYPE CollisionType;
	short NearestLedgeAngle;
	float NearestLedgeDistance;
	int TiltX;
	int TiltZ;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight() { return MiddleRight.SplitAngle != 0.0f && MiddleRight.SplitAngle == Middle.SplitAngle; }
	bool TriangleAtLeft() { return MiddleLeft.SplitAngle != 0.0f && MiddleLeft.SplitAngle == Middle.SplitAngle; }
	bool DiagonalStepAtRight() { return MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90)); }
	bool DiagonalStepAtLeft()  { return MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90)); }
};

COLL_RESULT GetCollisionResult(ITEM_INFO* item, short angle, int dist, int height);
COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z);
COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber);
COLL_RESULT GetCollisionResult(ITEM_INFO* item);

void  GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, bool resetRoom = false);
void  GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, bool resetRoom = false);
int   GetQuadrant(short angle);
short GetNearestLedgeAngle(ITEM_INFO* item, COLL_INFO* coll, float& dist);

int  FindGridShift(int x, int z);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void MoveItem(ITEM_INFO* item, short angle, int x, int y = 0);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, float offsetMultiplier = 0.0f);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, short angle, float offsetMultiplier = 0.0f);
void SnapItemToGrid(ITEM_INFO* item, COLL_INFO* coll);

void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide = 1);
