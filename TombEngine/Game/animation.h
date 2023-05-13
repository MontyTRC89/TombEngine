#pragma once
#include "Math/Math.h"
#include "Objects/game_object_ids.h"

using namespace TEN::Math;

enum GAME_OBJECT_ID : short;
class EulerAngles;
class Pose;
class Vector3i;
struct CreatureBiteInfo;
struct ItemInfo;
struct ObjectInfo;

// NOTES:
// animNumber: Relative animation number.
// animIndex:  Index of animation in giant g_Level.Anims vector. Temporary.

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

struct StateDispatchRangeData
{
	int LinkAnimNumber	= 0;
	int LinkFrameNumber = 0; // g_Level.Frames base index.
	int StartFrame		= 0; // g_Level.Frames start index.
	int EndFrame		= 0; // g_Level.Frames end index.
};

struct StateDispatchData
{
	int TargetState = 0;
	std::vector<StateDispatchRangeData> Ranges = {};
};

struct AnimData
{
	int ActiveState	  = 0;
	int Interpolation = 0;

	int FramePtr  = 0; // g_Level.Frames base index.
	int frameBase = 0; // g_Level.Frames start index.
	int frameEnd  = 0; // g_Level.Frames end index.

	int LinkAnimNumber	= 0;
	int LinkFrameNumber = 0; // g_Level.Frames index.
	int NumCommands		= 0;
	int CommandIndex	= 0;

	std::vector<StateDispatchData> Dispatches = {};
	
	// CONVENTION: +X = Right, +Y = Down, +Z = Forward.
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;
};

struct AnimFrameInterpData
{
	const AnimFrame* FramePtr0 = nullptr;
	const AnimFrame* FramePtr1 = nullptr;
	float Alpha = 0.0f;
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
bool HasStateDispatch(ItemInfo* item, std::optional<int> targetState = std::nullopt);
bool TestAnimNumber(const ItemInfo& item, int animNumber);
bool TestLastFrame(ItemInfo* item, std::optional<int> animNumber = std::nullopt);
bool TestAnimFrame(const ItemInfo& item, int frameStart);
bool TestAnimFrameRange(const ItemInfo& item, int frameStart, int frameEnd);

// Entity translation
void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
void TranslateItem(ItemInfo* item, const EulerAngles& orient, float distance);
void TranslateItem(ItemInfo* item, const Vector3& direction, float distance);

// Setters
void SetAnimation(ItemInfo& item, GAME_OBJECT_ID animObjectID, int animNumber, int frameNumber = 0);
void SetAnimation(ItemInfo& item, int animNumber, int frameNumber = 0);
void SetAnimation(ItemInfo* item, int animNumber, int frameNumber = 0); // Deprecated.

// Getters
const AnimData& GetAnimData(GAME_OBJECT_ID objectID, int animNumber);
const AnimData& GetAnimData(const ObjectInfo& object, int animNumber);
const AnimData& GetAnimData(const ItemInfo& item, std::optional<int> animNumber = std::nullopt);
const AnimData& GetAnimData(const ItemInfo* item, std::optional<int> animNumber = std::nullopt); // Deprecated.

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item);
const AnimFrame&	GetAnimFrame(const ItemInfo& item, int animNumber, int frameNumber);
const AnimFrame*	GetFrame(GAME_OBJECT_ID objectID, int animNumber, int frameNumber);
const AnimFrame*	GetFirstFrame(GAME_OBJECT_ID objectID, int animNumber);
const AnimFrame*	GetLastFrame(GAME_OBJECT_ID objectID, int animNumber);
const AnimFrame&	GetBestFrame(const ItemInfo& item);

int GetFrameNumber(const ItemInfo& item);
int GetFrameNumber(ItemInfo* item); // Deprecated.
int GetFrameIndex(ItemInfo* item, int frameNumber);
int GetFrameIndex(GAME_OBJECT_ID objectID, int animNumber, int frameNumber);

int GetFrameCount(int animIndex);

int	 GetNextAnimState(ItemInfo* item);
int	 GetNextAnimState(int objectID, int animNumber);
bool GetStateDispatch(ItemInfo* item, const AnimData& anim);

void ClampRotation(Pose& outPose, short angle, short rotation); 
void DrawAnimatingItem(ItemInfo* item);

Vector3i   GetJointPosition(const ItemInfo& item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3i   GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3i   GetJointPosition(ItemInfo* item, const CreatureBiteInfo& bite);
Vector3i   GetJointPosition(const ItemInfo& item, const CreatureBiteInfo& bite);

Vector3	   GetJointOffset(GAME_OBJECT_ID objectID, int jointIndex);
Quaternion GetBoneOrientation(const ItemInfo& item, int boneIndex);
float	   GetBoneLength(GAME_OBJECT_ID objectID, int boneIndex);
