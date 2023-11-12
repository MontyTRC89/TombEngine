#pragma once

#include "Math/Math.h"
#include "Objects/game_object_ids.h"

struct ItemInfo;
class FloorInfo;
struct ROOM_INFO;
struct MESH_INFO;
enum RoomEnvFlags;

constexpr auto NO_LOWER_BOUND = -NO_HEIGHT;	// Used by coll->Setup.LowerFloorBound.
constexpr auto NO_UPPER_BOUND = NO_HEIGHT;	// Used by coll->Setup.UpperFloorBound.
constexpr auto COLLISION_CHECK_DISTANCE = BLOCK(8);

enum CollisionType
{
	CT_NONE		 = 0,
	CT_FRONT	 = (1 << 0),
	CT_LEFT		 = (1 << 1),
	CT_RIGHT	 = (1 << 2),
	CT_TOP		 = (1 << 3),
	CT_TOP_FRONT = (1 << 4),
	CT_CLAMP	 = (1 << 5)
};

enum class CollisionProbeMode
{
	Quadrants,
	FreeForward,
	FreeFlat
};

enum class CornerType
{
	None,
	Inner,
	Outer
};

struct CollisionPosition
{
	int Floor;
	int Ceiling;
	int Bridge;
	float SplitAngle;
	bool FloorSlope;
	bool CeilingSlope;
	bool DiagonalStep;

	bool HasDiagonalSplit()		   { return ((SplitAngle == (45.0f * RADIAN)) || (SplitAngle == (135.0f * RADIAN))); }
	bool HasFlippedDiagonalSplit() { return (HasDiagonalSplit() && (SplitAngle != (45.0f * RADIAN))); }
};

struct CollisionResult
{
	Vector3i Coordinates;
	int RoomNumber;

	FloorInfo* Block;
	FloorInfo* BottomBlock;

	CollisionPosition Position;
	Vector2 FloorTilt;	 // x = x, y = z
	Vector2 CeilingTilt; // x = x, y = z
};

struct CollisionSetup
{
	CollisionProbeMode Mode;	// Probe rotation mode
	int   Radius;				// Collision bounds horizontal size
	int   Height;				// Collision bounds vertical size
	short ForwardAngle;			// Forward angle direction

	int LowerFloorBound;		// Borderline floor step-up height 
	int UpperFloorBound;		// Borderline floor step-down height
	int LowerCeilingBound;		// Borderline ceiling step-up height
	int UpperCeilingBound;		// Borderline ceiling step-down height

	bool BlockFloorSlopeUp;		// Treat steep slopes as walls
	bool BlockFloorSlopeDown;	// Treat steep slopes as pits
	bool BlockCeilingSlope;		// Treat steep slopes on ceilings as walls
	bool BlockDeathFloorDown;	// Treat death sectors as pits
	bool BlockMonkeySwingEdge;	// Treat non-monkey sectors as walls
	
	bool EnableObjectPush;		// Can be pushed by objects
	bool EnableSpasm;			// Convulse when pushed

	// Preserve previous parameters to restore later.
	Vector3i	   PrevPosition		= Vector3i::Zero;;
	GAME_OBJECT_ID PrevAnimObjectID = ID_NO_OBJECT;
	int			   PrevAnimNumber	= 0;
	int			   PrevFrameNumber	= 0;
	int			   PrevState		= 0;
};

struct CollisionInfo
{
	CollisionSetup    Setup;    // In parameters

	CollisionPosition Middle;       
	CollisionPosition MiddleLeft;   
	CollisionPosition MiddleRight;  
	CollisionPosition Front;        
	CollisionPosition FrontLeft;    
	CollisionPosition FrontRight;   

	Pose Shift = Pose::Zero;
	CollisionType CollisionType;
	Vector2 FloorTilt;	 // x = x, y = z
	Vector2 CeilingTilt; // x = x, y = z
	short NearestLedgeAngle;
	float NearestLedgeDistance;

	int  LastBridgeItemNumber;
	Pose LastBridgeItemPose;

	bool HitStatic;
	bool HitTallObject;

	bool TriangleAtRight()	   { return ((MiddleRight.SplitAngle != 0.0f) && (MiddleRight.SplitAngle == Middle.SplitAngle)); }
	bool TriangleAtLeft()	   { return ((MiddleLeft.SplitAngle != 0.0f) && (MiddleLeft.SplitAngle == Middle.SplitAngle)); }
	bool DiagonalStepAtRight() { return (MiddleRight.DiagonalStep && TriangleAtRight() && (NearestLedgeAngle % ANGLE(90.0f))); }
	bool DiagonalStepAtLeft()  { return (MiddleLeft.DiagonalStep && TriangleAtLeft() && (NearestLedgeAngle % ANGLE(90.0f))); }
};

[[nodiscard]] bool TestItemRoomCollisionAABB(ItemInfo* item);

CollisionResult GetCollision(const ItemInfo& item);
CollisionResult GetCollision(ItemInfo* item);
CollisionResult GetCollision(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
CollisionResult GetCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
CollisionResult GetCollision(const Vector3i& pos, int roomNumber, const EulerAngles& orient, float dist);
CollisionResult GetCollision(const Vector3i& pos, int roomNumber, const Vector3& dir, float dist);
CollisionResult GetCollision(const Vector3i& pos, int roomNumber);
CollisionResult GetCollision(int x, int y, int z, short roomNumber);
CollisionResult GetCollision(const GameVector& pos);
CollisionResult GetCollision(FloorInfo* floor, int x, int y, int z);

void  GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, const Vector3i& offset, bool resetRoom = false);
void  GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, bool resetRoom = false);
int	  GetQuadrant(short angle);
short GetNearestLedgeAngle(ItemInfo* item, CollisionInfo* coll, float& distance);

FloorInfo* GetFloor(int x, int y, int z, short* roomNumber);
int GetFloorHeight(FloorInfo* floor, int x, int y, int z);
int GetCeiling(FloorInfo* floor, int x, int y, int z);
int GetDistanceToFloor(int itemNumber, bool precise = true);

int GetWaterSurface(int x, int y, int z, short roomNumber);
int GetWaterSurface(ItemInfo* item);
int GetWaterDepth(int x, int y, int z, short roomNumber);
int GetWaterDepth(ItemInfo* item);
int GetWaterHeight(int x, int y, int z, short roomNumber);
int GetWaterHeight(ItemInfo* item);

int  FindGridShift(int x, int z);
void ShiftItem(ItemInfo* item, CollisionInfo* coll);
void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, float offsetMultiplier = 0.0f, bool snapToAngle = true);
void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, short angle, float offsetMultiplier = 0.0f);
void SnapItemToGrid(ItemInfo* item, CollisionInfo* coll);

void AlignEntityToSurface(ItemInfo* item, const Vector2& ellipse, float alpha = 0.75f, short constraintAngle = ANGLE(70.0f));

bool TestEnvironment(RoomEnvFlags environmentType, int x, int y, int z, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, Vector3i pos, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ItemInfo* item);
bool TestEnvironment(RoomEnvFlags environmentType, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ROOM_INFO* room);
bool TestEnvironmentFlags(RoomEnvFlags environmentType, int flags);

