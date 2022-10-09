#pragma once
#include "Math/Math.h" // TODO: Including this here shouldn't be necessary.

class EulerAngles;
struct ItemInfo;
class Pose;
class Vector3i;

struct AnimFrame
{
	BOUNDING_BOX boundingBox;
	short offsetX;
	short offsetY;
	short offsetZ;
	std::vector<Quaternion> angles;
};

struct StateDispatchData
{
	int TargetState	 = -1;
	int NumberRanges = 0;
	int RangeIndex	 = -1;
};

struct StateDispatchRangeData
{
	int StartFrame	 = -1;
	int EndFrame	 = -1;
	int LinkAnimNum	 = -1;
	int LinkFrameNum = -1;
};

struct AnimData
{
	int FramePtr;
	int Interpolation;
	int ActiveState;

	// CONVENTION: +X = right, +Y = down, +Z = forward
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;

	int frameBase;
	int frameEnd;

	int JumpAnimNum;
	int JumpFrameNum;
	int NumStateDispatches;
	int StateDispatchIndex;
	int NumCommands;
	int CommandIndex;
};

enum class AnimCommandType
{
	None = 0,
	MoveOrigin,
	JumpVelocity,
	AttackReady,
	Deactivate,
	SoundEffect,
	Flipeffect
};

struct BoneMutator
{
	Vector3 Offset   = Vector3::Zero;
	Vector3 Rotation = Vector3::Zero;
	Vector3 Scale    = Vector3::One;

	bool IsEmpty() { return (Offset == Vector3::Zero) && (Rotation == Vector3::Zero) && (Scale == Vector3::One); };
};

void AnimateLara(ItemInfo* item);
void AnimateItem(ItemInfo* item);

bool HasStateDispatch(ItemInfo* item, int targetState = -1);
bool TestLastFrame(ItemInfo* item, int animNumber = -1);

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
