#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/rope.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// TODO: Arm anim object in samegame.

unsigned int AnimData::GetFrameCount(bool isNonZero) const
{
	unsigned int frameCount = (Keyframes.size() - 1) * Interpolation;
	return ((isNonZero && frameCount <= 0) ? 1 : frameCount);
}

int AnimData::GetLastFrameNumber() const
{
	return (Keyframes.size() - 1); // TODO: What if empty?
}

AnimFrameInterpData AnimData::GetFrameInterpData(int frameNumber) const
{
	// Normalize frame number into keyframe range.
	float keyframeNumber = frameNumber / (float)Interpolation;

	// Determine keyframe numbers defining interpolated frame.
	int keyframeNumber0 = (int)floor(keyframeNumber);
	int keyframeNumber1 = (int)ceil(keyframeNumber);

	// Calculate interpolation alpha between keyframes.
	float alpha = (1.0f / Interpolation) * (frameNumber % Interpolation);

	// Return frame interpolation data.
	return AnimFrameInterpData(Keyframes[keyframeNumber0], Keyframes[keyframeNumber1], alpha);
}

const Keyframe& AnimData::GetKeyframe(int frameNumber) const
{
	static const auto DUMMY_KEYFRAME = Keyframe{};

	if (frameNumber < 0 || frameNumber >= Keyframes.size() || Keyframes.empty())
		return DUMMY_KEYFRAME;

	return Keyframes[frameNumber];
}

const Keyframe& AnimData::GetClosestKeyframe(int frameNumber) const
{
	auto frameData = GetFrameInterpData(frameNumber);
	return ((frameData.Alpha <= 0.5f) ? frameData.Keyframe0 : frameData.Keyframe1);
}

// NOTE: 0 frames counts as 1.
static unsigned int GetNonZeroFrameCount(const AnimData& anim)
{
	unsigned int frameCount = (anim.Keyframes.size() - 1) * anim.Interpolation;
	return ((frameCount > 0) ? frameCount : 1);
}

static void ExecuteAnimCommands(ItemInfo& item, bool isFrameBased)
{
	const auto& anim = GetAnimData(item);
	
	for (const auto& command : anim.Commands)
		command->Execute(item, isFrameBased);
}

void AnimateItem(ItemInfo* item)
{
	if (!item->IsLara())
	{
		item->TouchBits.ClearAll();
		item->HitStatus = false;
	}

	ExecuteAnimCommands(*item, true);
	item->Animation.FrameNumber++;

	const auto* animPtr = &GetAnimData(*item);

	if (!animPtr->Dispatches.empty() && GetStateDispatch(item, *animPtr))
	{
		animPtr = &GetAnimData(*item);

		item->Animation.ActiveState = animPtr->ActiveState;

		if (!item->IsLara())
		{
			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
	}

	if (item->Animation.FrameNumber > ((animPtr->Keyframes.size() - 1) * animPtr->Interpolation))
	{
		ExecuteAnimCommands(*item, false);

		item->Animation.AnimNumber = animPtr->NextAnimNumber;
		item->Animation.FrameNumber = animPtr->NextFrameNumber;

		animPtr = &GetAnimData(*item);
		
		if (item->Animation.ActiveState != animPtr->ActiveState)
		{
			item->Animation.ActiveState =
			item->Animation.TargetState = animPtr->ActiveState;

			// NOTE: Legacy code only set TargetState for the player.
			// Remove this comment if no issues arise with new generic behaviour. -- Sezz 2023.06.07
		}

		if (!item->IsLara())
		{
			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
	}

	unsigned int frameCount = GetNonZeroFrameCount(*animPtr);
	int currentFrame = item->Animation.FrameNumber;

	if (item->Animation.IsAirborne)
	{
		if (item->IsLara())
		{
			if (TestEnvironment(ENV_FLAG_SWAMP, item))
			{
				item->Animation.Velocity.z -= item->Animation.Velocity.z / 8;
				if (abs(item->Animation.Velocity.z) < 8.0f)
				{
					item->Animation.IsAirborne = false;
					item->Animation.Velocity.z = 0.0f;
				}

				if (item->Animation.Velocity.y > 128.0f)
					item->Animation.Velocity.y /= 2;
				item->Animation.Velocity.y -= item->Animation.Velocity.y / 4;

				if (item->Animation.Velocity.y < 4.0f)
					item->Animation.Velocity.y = 4.0f;
				item->Pose.Position.y += item->Animation.Velocity.y;
			}
			else
			{
				item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
				item->Animation.Velocity.z += (animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount;

				item->Pose.Position.y += item->Animation.Velocity.y;
			}
		}
		else
		{
			item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
			item->Pose.Position.y += item->Animation.Velocity.y;
		}
	}
	else
	{
		if (item->IsLara())
		{
			const auto& player = *GetLaraInfo(item);

			if (player.Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
				item->Animation.Velocity.z = (animPtr->VelocityStart.z / 2) + ((((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame) / 4);
			else
				item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame);
		}
		else
		{
			item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrame);
			item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame);
		}
	}
	
	if (item->IsLara())
	{
		const auto& player = *GetLaraInfo(item);

		item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrame);

		if (player.Control.Rope.Ptr != -1)
			DelAlignLaraToRope(item);

		if (!player.Control.IsMoving)
			TranslateItem(item, player.Control.MoveAngle, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

		// Update matrices.
		g_Renderer.UpdateLaraAnimations(true);
	}
	else
	{
		TranslateItem(item, item->Pose.Orientation.y, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

		// Update matrices.
		g_Renderer.UpdateItemAnimations(item->Index, true);
	}
}

bool HasStateDispatch(ItemInfo* item, std::optional<int> targetState)
{
	const auto& anim = GetAnimData(*item);

	// No dispatches; return early.
	if (anim.Dispatches.empty())
		return false;

	// If input target state not set, use entity's target state.
	if (!targetState.has_value())
		targetState = item->Animation.TargetState;

	// Iterate over animation's state dispatches.
	for (const auto& dispatch : anim.Dispatches)
	{
		// Target states don't match; continue.
		if (dispatch.TargetState != targetState.value())
			continue;

		// Check if current frame is within dispatch range.
		if (item->Animation.FrameNumber >= dispatch.FrameRange.first &&
			item->Animation.FrameNumber <= dispatch.FrameRange.second)
		{
			return true;
		}
	}

	return false;
}

bool TestLastFrame(ItemInfo* item, std::optional<int> animNumber)
{
	if (!animNumber.has_value())
		animNumber = item->Animation.AnimNumber;

	// Animation to test doesn't match; return early.
	if (item->Animation.AnimNumber != animNumber)
		return false;

	const auto& anim = GetAnimData(item->Animation.AnimObjectID, animNumber.value());
	return (item->Animation.FrameNumber >= ((anim.Keyframes.size() - 1) * anim.Interpolation));
}

// Deprecated.
bool TestAnimFrame(const ItemInfo& item, int frameStart)
{
	return (item.Animation.FrameNumber == frameStart);
}

bool TestAnimFrameRange(const ItemInfo& item, int frameStart, int frameEnd)
{
	return (item.Animation.FrameNumber >= frameStart &&
			item.Animation.FrameNumber <= frameEnd);
}

void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down, float right)
{
	item->Pose.Translate(headingAngle, forward, down, right);
}

void TranslateItem(ItemInfo* item, const EulerAngles& orient, float distance)
{
	item->Pose.Translate(orient, distance);
}

void TranslateItem(ItemInfo* item, const Vector3& direction, float distance)
{
	item->Pose.Translate(direction, distance);
}

void SetAnimation(ItemInfo& item, GAME_OBJECT_ID animObjectID, int animNumber, int frameNumber)
{
	// Animation already set; return early.
	if (item.Animation.AnimObjectID == animObjectID &&
		item.Animation.AnimNumber == animNumber &&
		item.Animation.FrameNumber == frameNumber)
	{
		return;
	}

	const auto& animObject = Objects[animObjectID];
	const auto& anim = GetAnimData(animObject, animNumber);

	// Animation missing; return early.
	if (animNumber < 0 || animNumber >= animObject.Animations.size())
	{
		TENLog(
			"Attempted to set missing animation " + std::to_string(animNumber) +
			((animObjectID == item.ObjectNumber) ? "" : (" from object " + GetObjectName(animObjectID))) +
			" for object " + GetObjectName(item.ObjectNumber),
			LogLevel::Warning);

		return;
	}

	item.Animation.AnimObjectID = animObjectID;
	item.Animation.AnimNumber = animNumber;
	item.Animation.FrameNumber = frameNumber;
	item.Animation.ActiveState =
	item.Animation.TargetState = anim.ActiveState;
}

void SetAnimation(ItemInfo& item, int animNumber, int frameNumber)
{
	SetAnimation(item, item.ObjectNumber, animNumber, frameNumber);
}

void SetAnimation(ItemInfo* item, int animNumber, int frameNumber)
{
	SetAnimation(*item, item->ObjectNumber, animNumber, frameNumber);
}

const AnimData& GetAnimData(const ObjectInfo& object, int animNumber)
{
	if (animNumber < 0 || animNumber >= object.Animations.size())
	{
		TENLog("Attempted to fetch missing animation.", LogLevel::Warning);
		return object.Animations.front();
	}

	return object.Animations[animNumber];
}

const AnimData& GetAnimData(GAME_OBJECT_ID objectID, int animNumber)
{
	const auto& object = Objects[objectID];
	return GetAnimData(object, animNumber);
}

const AnimData& GetAnimData(const ItemInfo& item, std::optional<int> animNumber)
{
	if (!animNumber.has_value())
		animNumber = item.Animation.AnimNumber;

	return GetAnimData(item.Animation.AnimObjectID, animNumber.value());
}

const AnimData& GetAnimData(const ItemInfo* item, std::optional<int> animNumber)
{
	return GetAnimData(*item, animNumber);
}

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item)
{
	const auto& anim = GetAnimData(item);
	int frameNumber = GetFrameNumber(item);
	return anim.GetFrameInterpData(frameNumber);
}

const Keyframe& GetKeyframe(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	const auto& anim = GetAnimData(objectID, animNumber);

	// Get and clamp frame count.
	unsigned int frameCount = anim.Keyframes.size() * anim.Interpolation;
	if (frameNumber > frameCount)
		frameNumber = frameCount;

	return anim.GetClosestKeyframe(frameNumber);
}

const Keyframe& GetKeyframe(const ItemInfo& item, int animNumber, int frameNumber)
{
	return GetKeyframe(item.ObjectNumber, animNumber, frameNumber);
}

const Keyframe& GetFirstKeyframe(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetKeyframe(objectID, animNumber, 0);
}

const Keyframe& GetLastKeyframe(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetKeyframe(objectID, animNumber, INT_MAX);
}

const Keyframe& GetClosestKeyframe(const ItemInfo& item)
{
	const auto& anim = GetAnimData(item);
	int frameNumber = GetFrameNumber(item);
	return anim.GetClosestKeyframe(frameNumber);
}

// Deprecated. Got relative.
int GetFrameNumber(const ItemInfo& item)
{
	return item.Animation.FrameNumber;
}

int GetFrameIndex(ItemInfo* item, int frameNumber)
{
	int animNumber = item->Animation.AnimNumber;
	return GetFrameIndex(item->Animation.AnimObjectID, animNumber, frameNumber);
}

// Deprecated. Got absolute.
int GetFrameIndex(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	return frameNumber;
}

int GetFrameCount(GAME_OBJECT_ID objectID, int animNumber)
{
	const auto& anim = GetAnimData(objectID, animNumber);
	return anim.Keyframes.size() * anim.Interpolation;
}

int GetFrameCount(const ItemInfo& item)
{
	return GetFrameCount(item.Animation.AnimObjectID, item.Animation.AnimNumber);
}

int GetNextAnimState(const ItemInfo& item)
{
	return GetNextAnimState(item.Animation.AnimObjectID, item.Animation.AnimNumber);
}

int GetNextAnimState(GAME_OBJECT_ID objectID, int animNumber)
{
	const auto& anim = GetAnimData(objectID, animNumber);
	const auto& nextAnim = GetAnimData(objectID, anim.NextAnimNumber);

	return nextAnim.ActiveState;
}

bool GetStateDispatch(ItemInfo* item, const AnimData& anim)
{
	// Active and target states already match; return early.
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	// No dispatches; return early.
	if (anim.Dispatches.empty())
		return false;

	// Iterate over animation's state dispatches.
	for (const auto& dispatch : anim.Dispatches)
	{
		// Target states don't match; continue.
		if (dispatch.TargetState != item->Animation.TargetState)
			continue;

		// Set new animation if current frame is within dispatch range.
		if (item->Animation.FrameNumber >= dispatch.FrameRange.first &&
			item->Animation.FrameNumber <= dispatch.FrameRange.second)
		{
			item->Animation.AnimNumber = dispatch.NextAnimNumber;
			item->Animation.FrameNumber = dispatch.NextFrameNumber;
			return true;
		}
	}

	return false;
}

void DrawAnimatingItem(ItemInfo* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is nullptr in ObjectInfo
}

void ClampRotation(Pose& outPose, short angle, short rotation)
{
	if (angle <= rotation)
	{
		outPose.Orientation.y += (angle >= -rotation) ? angle : -rotation;
	}
	else
	{
		outPose.Orientation.y += rotation;
	}
}

Vector3i GetJointPosition(const ItemInfo& item, int jointIndex, const Vector3i& relOffset)
{
	// Use matrices done in renderer to transform relative offset.
	return Vector3i(g_Renderer.GetAbsEntityBonePosition(item.Index, jointIndex, relOffset.ToVector3()));
}

Vector3i GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& relOffset)
{
	return GetJointPosition(*item, jointIndex, relOffset);
}

Vector3i GetJointPosition(ItemInfo* item, const CreatureBiteInfo& bite)
{
	return GetJointPosition(item, bite.BoneID, bite.Position);
}

Vector3i GetJointPosition(const ItemInfo& item, const CreatureBiteInfo& bite)
{
	return GetJointPosition(item, bite.BoneID, bite.Position);
}

Vector3 GetJointOffset(GAME_OBJECT_ID objectID, int jointIndex)
{
	const auto& object = Objects[objectID];

	int* bonePtr = &g_Level.Bones[object.boneIndex + (jointIndex * 4)];
	return Vector3(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3));
}

Quaternion GetBoneOrientation(const ItemInfo& item, int boneID)
{
	static const auto REF_DIRECTION = Vector3::UnitZ;

	auto origin = g_Renderer.GetAbsEntityBonePosition(item.Index, boneID);
	auto target = g_Renderer.GetAbsEntityBonePosition(item.Index, boneID, REF_DIRECTION);

	auto direction = target - origin;
	direction.Normalize();
	return Geometry::ConvertDirectionToQuat(direction);
}

// NOTE: Will not work for bones at ends of hierarchies.
float GetBoneLength(GAME_OBJECT_ID objectID, int boneID)
{
	const auto& object = Objects[objectID];

	if (object.nmeshes == boneID)
		return 0.0f;

	auto nextBoneOffset = GetJointOffset(objectID, boneID + 1);
	return nextBoneOffset.Length();
}
