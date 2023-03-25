#pragma once
#include "Math/Math.h"
#include "Objects/game_object_ids.h"

using namespace TEN::Math;

class EulerAngles;
class Pose;
class Vector3i;
struct ItemInfo;

// NOTES:
// animNumber: Relative entity animation ID.
// animIndex:  Index of animation in giant g_Level.Anims vector.

constexpr auto NO_STATE = -1;
constexpr auto NO_ANIM	= -1;

enum class AnimCommandType
{
	None,
	MoveOrigin, // "Post-animation adjustment"
	JumpVelocity,
	AttackReady,
	Deactivate,
	SoundEffect,
	Flipeffect
};

struct AnimFrame
{
	GameBoundingBox			BoundingBox		 = GameBoundingBox::Zero;
	Vector3					Offset			 = Vector3::Zero;
	std::vector<Quaternion> BoneOrientations = {};
};

struct StateDispatchData
{
	int TargetState	 = NO_STATE;
	int NumberRanges = 0;
	int RangeIndex	 = 0;
};

struct StateDispatchRangeData
{
	int StartFrame	 = 0;		// g_Level.Frames base index.
	int EndFrame	 = 0;		// g_Level.Frames end index.
	int LinkAnimNum	 = NO_ANIM; // g_Level.Anims index.
	int LinkFrameNum = NO_ANIM; // g_Level.Frames index.
};

struct AnimData
{
	int FramePtr	  = 0; // g_Level.Frames base index.
	int Interpolation = 0;
	int ActiveState	  = 0;

	// CONVENTION: +X = Right, +Y = Down, +Z = Forward.
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;

	int frameBase = 0; // g_Level.Frames base index.
	int frameEnd  = 0; // g_Level.Frames end index.

	int JumpAnimNum		   = NO_ANIM; // g_Level.Anims index.
	int JumpFrameNum	   = 0;		  // g_Level.Frames index.
	int NumStateDispatches = 0;
	int StateDispatchIndex = 0;
	int NumCommands		   = 0;
	int CommandIndex	   = 0;
};

struct AnimFrameInterpData
{
	AnimFrame* FramePtr0 = nullptr;
	AnimFrame* FramePtr1 = nullptr;
	float	   Alpha	 = 0.0f;
};

struct BoneMutator
{
	Vector3		Offset	 = Vector3::Zero;
	EulerAngles Rotation = EulerAngles::Zero;
	Vector3		Scale	 = Vector3::One;

	bool IsEmpty() const
	{
		return (Offset == Vector3::Zero &&
				Rotation == EulerAngles::Zero &&
				Scale == Vector3::One);
	};
};

// Animation controller
void AnimateItem(ItemInfo* item);

// Inquirers
bool HasStateDispatch(ItemInfo* item, int targetState = NO_STATE);
bool TestAnimNumber(const ItemInfo& item, int animNumber);
bool TestLastFrame(ItemInfo* item, int animNumber = NO_ANIM);
bool TestAnimFrame(const ItemInfo& item, int frameStart);
bool TestAnimFrameRange(const ItemInfo& item, int frameStart, int frameEnd);

// Entity translation
void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
void TranslateItem(ItemInfo* item, const EulerAngles& orient, float distance);
void TranslateItem(ItemInfo* item, const Vector3& direction, float distance);

// Setters
void SetAnimation(ItemInfo* item, int animNumber, int frameNumber = 0);

// Getters
AnimData& GetAnimData(int animIndex);
AnimData& GetAnimData(const ObjectInfo& object, int animNumber);
AnimData& GetAnimData(const ItemInfo& item, int animNumber = NO_ANIM);
int		  GetCurrentRelativeFrameNumber(ItemInfo* item);
int		  GetAnimNumber(ItemInfo& item, int animNumber);
int		  GetFrameNumber(ItemInfo* item, int frameToStart);
int		  GetFrameNumber(int objectID, int animNumber, int frameToStart);
int		  GetFrameCount(int animIndex);
int		  GetNextAnimState(ItemInfo* item);
int		  GetNextAnimState(int objectID, int animNumber);
bool	  GetStateDispatch(ItemInfo* item, const AnimData& anim);

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item);
AnimFrame&			GetAnimFrame(const ItemInfo& item, int animNumber, int frameNumber);
AnimFrame*			GetFrame(GAME_OBJECT_ID objectID, int animNumber, int frameNumber);
AnimFrame*			GetFirstFrame(GAME_OBJECT_ID objectID, int animNumber);
AnimFrame*			GetLastFrame(GAME_OBJECT_ID objectID, int animNumber);
AnimFrame&			GetBestFrame(const ItemInfo& item);

void ClampRotation(Pose& outPose, short angle, short rotation); 
void DrawAnimatingItem(ItemInfo* item);

Vector3i   GetJointPosition(const ItemInfo& item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3i   GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3	   GetJointOffset(GAME_OBJECT_ID objectID, int jointIndex);
Quaternion GetBoneOrientation(const ItemInfo& item, int boneIndex);
float	   GetBoneLength(GAME_OBJECT_ID objectID, int boneIndex);
