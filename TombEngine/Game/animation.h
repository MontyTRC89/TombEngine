#pragma once
#include "Game/Animation/Commands.h"
#include "Math/Math.h"

using namespace TEN::Animation;
using namespace TEN::Math;

enum GAME_OBJECT_ID : short;
class EulerAngles;
class Pose;
class Vector3i;
struct AnimFrameInterpData;
struct CreatureBiteInfo;
struct ItemInfo;
struct ObjectInfo;

constexpr auto NO_STATE = -1;

struct Keyframe
{
	GameBoundingBox			BoundingBox		 = GameBoundingBox::Zero;
	Vector3					Offset			 = Vector3::Zero;
	std::vector<Quaternion> BoneOrientations = {};
};

struct StateDispatchData
{
	int TargetState		= 0;
	int NextAnimNumber	= 0;
	int NextFrameNumber = 0; // g_Level.Frames index. TODO: Use relative frame number in animation refactors tier 5.
	std::pair<int, int> FrameRange = {};
};

struct AnimData
{
	using AnimCommandPtr = std::unique_ptr<AnimCommand>;

	int State			= 0;
	int EndFrameNumber	= 0;
	int NextAnimNumber	= 0;
	int NextFrameNumber = 0;
	int Interpolation	= 0;

	// CONVENTION: +X = Right, +Y = Down, +Z = Forward.
	Vector3 VelocityStart = Vector3::Zero;
	Vector3 VelocityEnd	  = Vector3::Zero;

	// TODO: Make private.
	std::vector<Keyframe>		   Keyframes  = {};
	std::vector<StateDispatchData> Dispatches = {};
	std::vector<AnimCommandPtr>	   Commands	  = {};
	
	unsigned int		GetFrameCount() const;
	int					GetLastFrameNumber() const;
	AnimFrameInterpData GetFrameInterpData(int frameNumber) const;
	const Keyframe&		GetKeyframe(int frameNumber) const; // TODO: Must adopt.
	const Keyframe&		GetClosestKeyframe(int frameNumber) const;
};

struct AnimFrameInterpData
{
	const Keyframe& Keyframe0;
	const Keyframe& Keyframe1;
	float Alpha = 0.0f;

	AnimFrameInterpData(const Keyframe& keyframe0, const Keyframe& keyframe1, float alpha) : Keyframe0(keyframe0), Keyframe1(keyframe1)
	{
		Alpha = alpha;
	}
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
bool TestLastFrame(ItemInfo* item, std::optional<int> animNumber = std::nullopt); // Maybe replace with GetLastFrame() and do comparisons?
bool TestAnimFrame(const ItemInfo& item, int frameStart);
bool TestAnimFrameRange(const ItemInfo& item, int frameNumber0, int frameNumber1);

// Entity translation
void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
void TranslateItem(ItemInfo* item, const EulerAngles& orient, float distance);
void TranslateItem(ItemInfo* item, const Vector3& direction, float distance);

// Setters
void SetAnimation(ItemInfo& item, GAME_OBJECT_ID animObjectID, int animNumber, int frameNumber = 0);
void SetAnimation(ItemInfo& item, int animNumber, int frameNumber = 0);
void SetAnimation(ItemInfo* item, int animNumber, int frameNumber = 0); // Deprecated.

// Getters
const AnimData& GetAnimData(const ObjectInfo& object, int animNumber);
const AnimData& GetAnimData(GAME_OBJECT_ID objectID, int animNumber);
const AnimData& GetAnimData(const ItemInfo& item, std::optional<int> animNumber = std::nullopt);
const AnimData& GetAnimData(const ItemInfo* item, std::optional<int> animNumber = std::nullopt); // Deprecated.

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item);
const Keyframe&		GetKeyframe(GAME_OBJECT_ID objectID, int animNumber, int frameNumber = 0);
const Keyframe&		GetKeyframe(const ItemInfo& item, int animNumber, int frameNumber = 0);
const Keyframe&		GetFirstKeyframe(GAME_OBJECT_ID objectID, int animNumber);
const Keyframe&		GetLastKeyframe(GAME_OBJECT_ID objectID, int animNumber);
const Keyframe&		GetClosestKeyframe(const ItemInfo& item);

int GetFrameNumber(const ItemInfo& item);
int GetFrameIndex(ItemInfo* item, int frameNumber);
int GetFrameIndex(GAME_OBJECT_ID objectID, int animNumber, int frameNumber);
int GetFrameCount(GAME_OBJECT_ID objectID, int animNumber);
int GetFrameCount(const ItemInfo& item);

int	 GetNextAnimState(const ItemInfo& item);
int	 GetNextAnimState(GAME_OBJECT_ID objectID, int animNumber);
bool GetStateDispatch(ItemInfo* item, const AnimData& anim);

void ClampRotation(Pose& outPose, short angle, short rotation); 
void DrawAnimatingItem(ItemInfo* item);

Vector3i GetJointPosition(const ItemInfo& item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3i GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& relOffset = Vector3i::Zero);
Vector3i GetJointPosition(ItemInfo* item, const CreatureBiteInfo& bite);
Vector3i GetJointPosition(const ItemInfo& item, const CreatureBiteInfo& bite);

Vector3	   GetJointOffset(GAME_OBJECT_ID objectID, int jointIndex);
Quaternion GetBoneOrientation(const ItemInfo& item, int boneID);
float	   GetBoneLength(GAME_OBJECT_ID objectID, int boneID);
