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

AnimFrameInterpData AnimData::GetFrameInterpData(int frameNumber) const
{
	// FAILSAFE: Clamp frame number.
	frameNumber = std::clamp(frameNumber, 0, EndFrameNumber);

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

const KeyframeData& AnimData::GetClosestKeyframe(int frameNumber) const
{
	auto frameData = GetFrameInterpData(frameNumber);
	return ((frameData.Alpha <= 0.5f) ? frameData.Keyframe0 : frameData.Keyframe1);
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

		item->Animation.ActiveState = animPtr->State;

		if (!item->IsLara())
		{
			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
	}

	if (item->Animation.FrameNumber > animPtr->EndFrameNumber)
	{
		ExecuteAnimCommands(*item, false);

		item->Animation.AnimNumber = animPtr->NextAnimNumber;
		item->Animation.FrameNumber = animPtr->NextFrameNumber;

		animPtr = &GetAnimData(*item);
		
		if (item->Animation.ActiveState != animPtr->State)
		{
			item->Animation.ActiveState =
			item->Animation.TargetState = animPtr->State;

			// NOTE: Legacy code only set TargetState for the player.
			// Remove this comment if no issues arise with new generic behaviour. -- Sezz 2023.06.07
		}

		if (!item->IsLara())
		{
			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
	}

	// NOTE: Must use non-zero frame count in this edge case.
	unsigned int frameCount = animPtr->EndFrameNumber;
	if (frameCount == 0)
		frameCount = 1;

	int currentFrameNumber = item->Animation.FrameNumber;

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
				item->Animation.Velocity.z = (animPtr->VelocityStart.z / 2) + ((((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrameNumber) / 4);
			else
				item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrameNumber);
		}
		else
		{
			item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrameNumber);
			item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrameNumber);
		}
	}
	
	if (item->IsLara())
	{
		const auto& player = *GetLaraInfo(item);

		item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrameNumber);

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

	// Use entity's target state if no targetState argument passed.
	if (!targetState.has_value())
		targetState = item->Animation.TargetState;

	// Iterate over state dispatches.
	for (const auto& dispatch : anim.Dispatches)
	{
		// State doesn't match; continue.
		if (dispatch.State != targetState.value())
			continue;

		// Test if current frame is within dispatch range.
		if (TestAnimFrameRange(*item, dispatch.FrameNumberRange.first, dispatch.FrameNumberRange.second))
			return true;
	}

	return false;
}

bool TestLastFrame(ItemInfo* item, std::optional<int> animNumber)
{
	if (!animNumber.has_value())
		animNumber = item->Animation.AnimNumber;

	// Animation doesn't match; return early.
	if (item->Animation.AnimNumber != animNumber)
		return false;

	const auto& anim = GetAnimData(item->Animation.AnimObjectID, animNumber.value());
	return (item->Animation.FrameNumber == anim.EndFrameNumber);
}

bool TestAnimFrame(const ItemInfo& item, int frameStart)
{
	return (item.Animation.FrameNumber == frameStart);
}

bool TestAnimFrameRange(const ItemInfo& item, int frameNumber0, int frameNumber1)
{
	return (item.Animation.FrameNumber >= frameNumber0 &&
			item.Animation.FrameNumber <= frameNumber1);
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

	// Frame missing; return early.
	if (frameNumber < 0 || frameNumber > anim.EndFrameNumber)
	{
		TENLog(
			"Attempted to set missing frame " + std::to_string(frameNumber) + " from animation " + std::to_string(animNumber) +
			((animObjectID == item.ObjectNumber) ? "" : (" from object " + GetObjectName(animObjectID))) +
			" for object " + GetObjectName(item.ObjectNumber),
			LogLevel::Warning);

		return;
	}

	item.Animation.AnimObjectID = animObjectID;
	item.Animation.AnimNumber = animNumber;
	item.Animation.FrameNumber = frameNumber;
	item.Animation.ActiveState =
	item.Animation.TargetState = anim.State;
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

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item)
{
	const auto& anim = GetAnimData(item);
	return anim.GetFrameInterpData(item.Animation.FrameNumber);
}

const KeyframeData& GetKeyframe(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	const auto& anim = GetAnimData(objectID, animNumber);
	return anim.GetClosestKeyframe(frameNumber);
}

const KeyframeData& GetKeyframe(const ItemInfo& item, int animNumber, int frameNumber)
{
	return GetKeyframe(item.ObjectNumber, animNumber, frameNumber);
}

const KeyframeData& GetFirstKeyframe(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetKeyframe(objectID, animNumber, 0);
}

const KeyframeData& GetLastKeyframe(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetKeyframe(objectID, animNumber, INT_MAX);
}

const KeyframeData& GetClosestKeyframe(const ItemInfo& item)
{
	const auto& anim = GetAnimData(item);
	return anim.GetClosestKeyframe(item.Animation.FrameNumber);
}

int GetFrameNumber(const ItemInfo& item)
{
	return item.Animation.FrameNumber;
}

int GetFrameIndex(ItemInfo* item, int frameNumber)
{
	int animNumber = item->Animation.AnimNumber;
	return GetFrameIndex(item->Animation.AnimObjectID, animNumber, frameNumber);
}

int GetFrameIndex(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	return frameNumber;
}

int GetFrameCount(GAME_OBJECT_ID objectID, int animNumber)
{
	const auto& anim = GetAnimData(objectID, animNumber);
	return anim.EndFrameNumber;
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

	return nextAnim.State;
}

bool GetStateDispatch(ItemInfo* item, const AnimData& anim)
{
	// Active and target states already match; return early.
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	// No dispatches; return early.
	if (anim.Dispatches.empty())
		return false;

	// Iterate over state dispatches.
	for (const auto& dispatch : anim.Dispatches)
	{
		// State doesn't match; continue.
		if (dispatch.State != item->Animation.TargetState)
			continue;

		// Set new animation if current frame number is within dispatch range.
		if (TestAnimFrameRange(*item, dispatch.FrameNumberRange.first, dispatch.FrameNumberRange.second))
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
