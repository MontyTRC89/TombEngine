#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

struct ITEM_INFO;
struct COLL_INFO;
struct FLOOR_INFO;
struct ROOM_INFO;
struct MESH_INFO;
enum RoomEnvFlags;

constexpr auto NO_LOWER_BOUND = -NO_HEIGHT;	// used by coll->Setup.LowerFloorBound
constexpr auto NO_UPPER_BOUND = NO_HEIGHT;	// used by coll->Setup.UpperFloorBound
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

enum class CollProbeMode
{
	Quadrants,
	FreeForward,
	FreeFlat
};

enum class CornerResult
{
	None,
	Inner,
	Outer
};

struct COLL_POSITION
{
	int Floor;
	int Ceiling;
	int Bridge;
	float SplitAngle;
	bool FloorSlope;
	bool CeilingSlope;
	bool DiagonalStep;

	bool HasDiagonalSplit() { return SplitAngle == 45.0f * RADIAN || SplitAngle == 135.0f * RADIAN; }
	bool HasFlippedDiagonalSplit() { return HasDiagonalSplit() && SplitAngle != 45.0f * RADIAN; }
};

struct COLL_RESULT
{
	PHD_VECTOR Coordinates;
	int RoomNumber;

	FLOOR_INFO* Block;
	FLOOR_INFO* BottomBlock;

	COLL_POSITION Position;
	Vector2 FloorTilt;
	Vector2 CeilingTilt;
};

struct COLL_SETUP
{
	CollProbeMode Mode;			// Probe rotation mode

	bool CeilingSlopeIsWall;	// Treat steep slopes on ceilings as walls
	bool FloorSlopeIsWall;		// Treat steep slopes as walls
	bool FloorSlopeIsPit;		// Treat steep slopes as pits
	bool DeathFlagIsPit;		// Treat death sectors as pits
	bool NoMonkeyFlagIsWall;	// Treat non-monkey sectors as walls
	bool EnableObjectPush;		// Can be pushed by objects
	bool EnableSpasm;			// Convulse when pushed
						    
	int   Radius;				// Collision bounds horizontal size
	int   Height;				// Collision bounds vertical size
	short ForwardAngle;			// Forward angle direction
	int   LowerFloorBound;		// Borderline floor step-up height 
	int   UpperFloorBound;		// Borderline floor step-down height
	int   LowerCeilingBound;	// Borderline ceiling step-up height
	int   UpperCeilingBound;	// Borderline ceiling step-down height

	PHD_VECTOR OldPosition;		// Preserve old parameters to restore later
	int OldState;
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
	int FloorTiltX;
	int FloorTiltZ;
	int CeilingTiltX;
	int CeilingTiltZ;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight() { return MiddleRight.SplitAngle != 0.0f && MiddleRight.SplitAngle == Middle.SplitAngle; }
	bool TriangleAtLeft() { return MiddleLeft.SplitAngle != 0.0f && MiddleLeft.SplitAngle == Middle.SplitAngle; }
	bool DiagonalStepAtRight() { return MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90)); }
	bool DiagonalStepAtLeft()  { return MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90)); }
};

COLL_RESULT GetCollisionResult(ITEM_INFO* item, short angle, int distance, int height = 0, int side = 0);
COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z);
COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber);
COLL_RESULT GetCollisionResult(ITEM_INFO* item);

void  GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, bool resetRoom = false);
void  GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, bool resetRoom = false);
int   GetQuadrant(short angle);
short GetNearestLedgeAngle(ITEM_INFO* item, COLL_INFO* coll, float& distance);

int  FindGridShift(int x, int z);
void ShiftItem(ITEM_INFO* item, COLL_INFO* coll);
void MoveItem(ITEM_INFO* item, short angle, int x, int y = 0);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, float offsetMultiplier = 0.0f, bool snapYRot = true);
void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, short angle, float offsetMultiplier = 0.0f);
void SnapItemToGrid(ITEM_INFO* item, COLL_INFO* coll);

void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide = 1);

bool TestEnvironment(RoomEnvFlags envType, ROOM_INFO* room);
bool TestEnvironment(RoomEnvFlags envType, int roomNumber);
bool TestEnvironment(RoomEnvFlags envType, ITEM_INFO* item);
bool TestEnvironment(RoomEnvFlags envType, int x, int y, int z, int roomNumber);
