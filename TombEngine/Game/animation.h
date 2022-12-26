#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

class EulerAngles;
class Pose;
class Vector3i;
struct ItemInfo;

constexpr auto NO_STATE = -1;
constexpr auto NO_ANIM	= -1;

enum class AnimCommandType
{
	None,
	MoveOrigin,
	JumpVelocity,
	AttackReady,
	Deactivate,
	SoundEffect,
	Flipeffect
};

struct AnimFrame
{
	GameBoundingBox boundingBox = GameBoundingBox::Zero;
	short offsetX = 0;
	short offsetY = 0;
	short offsetZ = 0;
	std::vector<Quaternion> angles = {};
};

struct StateDispatchData
{
	int TargetState	 = NO_STATE;
	int NumberRanges = 0;
	int RangeIndex	 = 0;
};

struct StateDispatchRangeData
{
	int StartFrame	 = 0;
	int EndFrame	 = 0;
	int LinkAnimNum	 = NO_ANIM;
	int LinkFrameNum = NO_ANIM;
};

struct AnimData
{
	int FramePtr	  = 0;
	int Interpolation = 0;
	int ActiveState	  = 0;

	// CONVENTION: +X = Right, +Y = Down, +Z = Forward.
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;

	int frameBase = 0;
	int frameEnd  = 0;

	int JumpAnimNum		   = NO_ANIM;
	int JumpFrameNum	   = 0;
	int NumStateDispatches = 0;
	int StateDispatchIndex = 0;
	int NumCommands		   = 0;
	int CommandIndex	   = 0;
};

struct BoneMutator
{
	Vector3 Offset   = Vector3::Zero;
	Vector3 Rotation = Vector3::Zero;
	Vector3 Scale    = Vector3::One;

	bool IsEmpty() { return ((Offset == Vector3::Zero) && (Rotation == Vector3::Zero) && (Scale == Vector3::One)); };
};

void AnimateLara(ItemInfo* item);
void AnimateItem(ItemInfo* item);

bool HasStateDispatch(ItemInfo* item, int targetState = NO_STATE);
bool TestLastFrame(ItemInfo* item, int animNumber = NO_ANIM);
bool TestCurrentAnimation(ItemInfo* item, int animIndex = NO_ANIM);
bool TestFrameSingle(ItemInfo* item, int frameToStart);
bool TestFrameBetween(ItemInfo* item, int frameStart, int frameEnd);

void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
void TranslateItem(ItemInfo* item, const EulerAngles& orient, float distance);
void TranslateItem(ItemInfo* item, const Vector3& direction, float distance);
void SetAnimation(ItemInfo* item, int animIndex, int frameToStart = 0);

int GetCurrentRelativeFrameNumber(ItemInfo* item);
int GetFrameNumber(ItemInfo* item, int frameToStart);
int GetFrameNumber(int objectID, int animNumber, int frameToStart);
int GetFrameCount(int animNumber);
int GetNextAnimState(ItemInfo* item);
int GetNextAnimState(int objectID, int animNumber);
bool GetStateDispatch(ItemInfo* item, const AnimData& anim);
int GetFrame(ItemInfo* item, AnimFrame* outFramePtr[], int& outRate);
AnimFrame* GetBestFrame(ItemInfo* item);

void ClampRotation(Pose& outPose, short angle, short rotation); 
void DrawAnimatingItem(ItemInfo* item);

Vector3i GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& offset = Vector3i::Zero);

void PerformAnimCommands(ItemInfo* item, bool isFrameBased);
