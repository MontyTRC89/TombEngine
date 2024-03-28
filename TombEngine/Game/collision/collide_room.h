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

enum class CollisionType
{
	None,
	Front,
	Left,
	Right,
	Top,
	TopFront,
	Clamp
};

enum class CollisionProbeMode
{
	None,
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

	Vector3 FloorNormal;
	Vector3 CeilingNormal;
};

struct CollisionSetup
{
	CollisionProbeMode Mode = CollisionProbeMode::None; // Probe mode.

	int   Radius	   = 0; // Collision bounds radius.
	int   Height	   = 0; // Collision bounds height.
	short ForwardAngle = 0; // 2D heading angle.

	int UpperFloorBound	  = 0; // Borderline floor step-down height.
	int LowerFloorBound	  = 0; // Borderline floor step-up height.
	int UpperCeilingBound = 0; // Borderline ceiling step-down height.
	int LowerCeilingBound = 0; // Borderline ceiling step-up height.

	bool BlockFloorSlopeUp	  = false; // Block floor slopes above.
	bool BlockFloorSlopeDown  = false; // Block floor slopes below.
	bool BlockCeilingSlope	  = false; // Block ceiling slopes.
	bool BlockDeathFloorDown  = false; // Block death floors.
	bool BlockMonkeySwingEdge = false; // Block non-monkey sectors.
	
	bool EnableObjectPush = false; // Can be pushed by objects.
	bool EnableSpasm	  = false; // Convulse when pushed.

	// Preserve previous parameters to restore later.
	Vector3i	   PrevPosition		= Vector3i::Zero;
	GAME_OBJECT_ID PrevAnimObjectID = ID_NO_OBJECT;
	int			   PrevAnimNumber	= 0;
	int			   PrevFrameNumber	= 0;
	int			   PrevState		= 0;
};

struct CollisionInfo
{
	CollisionSetup Setup; // In parameters.

	CollisionPosition Middle;       
	CollisionPosition MiddleLeft;   
	CollisionPosition MiddleRight;  
	CollisionPosition Front;        
	CollisionPosition FrontLeft;    
	CollisionPosition FrontRight;   

	Pose Shift = Pose::Zero;
	CollisionType CollisionType;
	Vector3 FloorNormal;
	Vector3 CeilingNormal;
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
CollisionResult GetCollision(const ItemInfo* item);
CollisionResult GetCollision(const ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
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
bool TestEnvironment(RoomEnvFlags environmentType, const ItemInfo* item);
bool TestEnvironment(RoomEnvFlags environmentType, int roomNumber);
bool TestEnvironment(RoomEnvFlags environmentType, ROOM_INFO* room);
bool TestEnvironmentFlags(RoomEnvFlags environmentType, int flags);

