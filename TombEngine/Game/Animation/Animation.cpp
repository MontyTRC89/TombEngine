#include "framework.h"
#include "Game/Animation/Animation.h"

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
#include "Renderer/Renderer.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

//namespace TEN::Animation
//{
	constexpr auto VERTICAL_VELOCITY_GRAVITY_THRESHOLD = CLICK(0.5f);

	// TODO: Arm anim object in savegame.

	KeyframeInterpData::KeyframeInterpData(const KeyframeData& keyframe0, const KeyframeData& keyframe1, float alpha) :
		Keyframe0(keyframe0),
		Keyframe1(keyframe1)
	{
		Alpha = alpha;
	}

	KeyframeInterpData AnimData::GetKeyframeInterpData(int frameNumber) const
	{
		// FAILSAFE: Clamp frame number.
		frameNumber = std::clamp(frameNumber, 0, EndFrameNumber);

		// Calculate keyframes defining interpolated frame.
		float keyframeNumber = frameNumber / (float)Interpolation;
		const auto& keyframe0 = Keyframes[(int)floor(keyframeNumber)];
		const auto& keyframe1 = Keyframes[(int)ceil(keyframeNumber)];

		// Calculate interpolation alpha between keyframes.
		float alpha = (1.0f / Interpolation) * (frameNumber % Interpolation);

		// Return keyframe interpolation data.
		return KeyframeInterpData(keyframe0, keyframe1, alpha);
	}

	const KeyframeData& AnimData::GetClosestKeyframe(int frameNumber) const
	{
		auto interpData = GetKeyframeInterpData(frameNumber);
		return ((interpData.Alpha <= 0.5f) ? interpData.Keyframe0 : interpData.Keyframe1);
	}

	bool BoneMutator::IsEmpty() const
	{
		return (Offset == Vector3::Zero &&
				Rotation == EulerAngles::Identity &&
				Scale == Vector3::One);
	};

	static void ExecuteAnimCommands(ItemInfo& item, bool isFrameBased)
	{
		const auto& anim = GetAnimData(item);
		for (const auto& command : anim.Commands)
			command->Execute(item, isFrameBased);
	}

	void AnimateItem(ItemInfo& item)
	{
		if (!item.IsLara())
		{
			item.TouchBits.ClearAll();
			item.HitStatus = false;
		}

		ExecuteAnimCommands(item, true);
		item.Animation.FrameNumber++;

		const auto* anim = &GetAnimData(item);

		if (SetStateDispatch(item))
		{
			anim = &GetAnimData(item);

			item.Animation.ActiveState = anim->StateID;

			if (!item.IsLara())
			{
				// Reset RequiredState if already reached.
				if (item.Animation.RequiredState == item.Animation.ActiveState)
					item.Animation.RequiredState = NO_VALUE;
			}
		}

		if (item.Animation.FrameNumber > anim->EndFrameNumber)
		{
			ExecuteAnimCommands(item, false);

			item.Animation.AnimNumber = anim->NextAnimNumber;
			item.Animation.FrameNumber = anim->NextFrameNumber;

			anim = &GetAnimData(item);

			if (item.Animation.ActiveState != anim->StateID)
			{
				item.Animation.ActiveState =
					item.Animation.TargetState = anim->StateID;
			}

			if (!item.IsLara())
			{
				// Reset RequiredState if already reached.
				if (item.Animation.RequiredState == item.Animation.ActiveState)
					item.Animation.RequiredState = NO_VALUE;
			}
		}

		// NOTE: Must use non-zero frame count in this edge case.
		unsigned int frameCount = anim->EndFrameNumber;
		if (frameCount == 0)
			frameCount = 1;

		int currentFrameNumber = item.Animation.FrameNumber;

		auto animAccel = (anim->VelocityEnd - anim->VelocityStart) / frameCount;
		auto animVel = anim->VelocityStart + (animAccel * currentFrameNumber);

		if (item.Animation.IsAirborne)
		{
			if (item.IsLara())
			{
				if (TestEnvironment(ENV_FLAG_SWAMP, &item))
				{
					item.Animation.Velocity.z -= item.Animation.Velocity.z / 8;
					if (abs(item.Animation.Velocity.z) < 8.0f)
					{
						item.Animation.IsAirborne = false;
						item.Animation.Velocity.z = 0.0f;
					}

					if (item.Animation.Velocity.y > VERTICAL_VELOCITY_GRAVITY_THRESHOLD)
						item.Animation.Velocity.y /= 2;
					item.Animation.Velocity.y -= item.Animation.Velocity.y / 4;

					if (item.Animation.Velocity.y < 4.0f)
						item.Animation.Velocity.y = 4.0f;
					item.Pose.Position.y += item.Animation.Velocity.y;
				}
				else
				{
					item.Animation.Velocity.y += GetEffectiveGravity(item.Animation.Velocity.y);
					item.Animation.Velocity.z += animAccel.z;

					item.Pose.Position.y += item.Animation.Velocity.y;
				}
			}
			else
			{
				item.Animation.Velocity.y += GetEffectiveGravity(item.Animation.Velocity.y);
				item.Pose.Position.y += item.Animation.Velocity.y;
			}
		}
		else
		{
			if (item.IsLara())
			{
				const auto& player = GetLaraInfo(item);

				bool isInSwamp = (player.Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, &item));
				item.Animation.Velocity.z = isInSwamp ? (animVel.z / 2) : animVel.z;
			}
			else
			{
				item.Animation.Velocity.x = animVel.x;
				item.Animation.Velocity.z = animVel.z;
			}
		}

		if (item.IsLara())
		{
			const auto& player = GetLaraInfo(item);

			item.Animation.Velocity.x = animVel.x;

			if (player.Control.Rope.Ptr != NO_VALUE)
				DelAlignLaraToRope(&item);

			if (!player.Control.IsMoving)
				TranslateItem(&item, player.Control.MoveAngle, item.Animation.Velocity.z, 0.0f, item.Animation.Velocity.x);

			// Update matrices.
			g_Renderer.UpdateLaraAnimations(true);
		}
		else
		{
			TranslateItem(&item, item.Pose.Orientation.y, item.Animation.Velocity.z, 0.0f, item.Animation.Velocity.x);

			// Update matrices.
			g_Renderer.UpdateItemAnimations(item.Index, true);
		}
	}

	bool TestStateDispatch(const ItemInfo& item, int targetStateID)
	{
		const auto& anim = GetAnimData(item);

		// Iterate over state dispatches.
		for (const auto& dispatch : anim.Dispatches)
		{
			// State ID mismatch; continue.
			if (dispatch.StateID != ((targetStateID == NO_VALUE) ? item.Animation.TargetState : targetStateID))
				continue;

			// Test if current frame is within dispatch range.
			if (TestAnimFrameRange(item, dispatch.FrameNumberRange.first, dispatch.FrameNumberRange.second))
				return true;
		}

		return false;
	}

	bool TestLastFrame(const ItemInfo& item, int animNumber)
	{
		if (animNumber == NO_VALUE)
			animNumber = item.Animation.AnimNumber;

		// Animation number mismatch; return early.
		if (item.Animation.AnimNumber != animNumber)
			return false;

		// FAILSAFE: Frames beyond real end frame also count.
		const auto& anim = GetAnimData(item.Animation.AnimObjectID, animNumber);
		return (item.Animation.FrameNumber >= anim.EndFrameNumber);
	}

	bool TestAnimFrameRange(const ItemInfo& item, int lowFrameNumber, int highFrameNumber)
	{
		return (item.Animation.FrameNumber >= lowFrameNumber &&
				item.Animation.FrameNumber <= highFrameNumber);
	}

	const AnimData& GetAnimData(const ObjectInfo& object, int animNumber)
	{
		return object.Animations[animNumber];
	}

	const AnimData& GetAnimData(GAME_OBJECT_ID objectID, int animNumber)
	{
		const auto& object = Objects[objectID];
		return GetAnimData(object, animNumber);
	}

	const AnimData& GetAnimData(const ItemInfo& item, int animNumber)
	{
		if (animNumber == NO_VALUE)
			animNumber = item.Animation.AnimNumber;

		return GetAnimData(item.Animation.AnimObjectID, animNumber);
	}

	KeyframeInterpData GetFrameInterpData(const ItemInfo& item)
	{
		const auto& anim = GetAnimData(item);
		return anim.GetKeyframeInterpData(item.Animation.FrameNumber);
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

	int GetNextAnimState(const ItemInfo& item)
	{
		return GetNextAnimState(item.Animation.AnimObjectID, item.Animation.AnimNumber);
	}

	int GetNextAnimState(GAME_OBJECT_ID objectID, int animNumber)
	{
		const auto& anim = GetAnimData(objectID, animNumber);
		const auto& nextAnim = GetAnimData(objectID, anim.NextAnimNumber);

		return nextAnim.StateID;
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

	float GetEffectiveGravity(float verticalVel)
	{
		return ((verticalVel >= VERTICAL_VELOCITY_GRAVITY_THRESHOLD) ? 1.0f : GRAVITY);
	}

	Vector3i GetJointPosition(const ItemInfo& item, int boneID, const Vector3i& relOffset)
	{
		// Use matrices done in renderer to transform relative offset.
		return Vector3i(g_Renderer.GetAbsEntityBonePosition(item.Index, boneID, relOffset.ToVector3()));
	}

	Vector3i GetJointPosition(ItemInfo* item, int boneID, const Vector3i& relOffset)
	{
		return GetJointPosition(*item, boneID, relOffset);
	}

	Vector3i GetJointPosition(ItemInfo* item, const CreatureBiteInfo& bite)
	{
		return GetJointPosition(item, bite.BoneID, bite.Position);
	}

	Vector3i GetJointPosition(const ItemInfo& item, const CreatureBiteInfo& bite)
	{
		return GetJointPosition(item, bite.BoneID, bite.Position);
	}

	Vector3 GetJointOffset(GAME_OBJECT_ID objectID, int boneID)
	{
		const auto& object = Objects[objectID];
		const int* bonePtr = &g_Level.Bones[object.boneIndex + (boneID * 4)];

		return Vector3(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3));
	}

	Quaternion GetBoneOrientation(const ItemInfo& item, int boneID)
	{
		static const auto REF_DIR = Vector3::UnitZ;

		auto origin = g_Renderer.GetAbsEntityBonePosition(item.Index, boneID);
		auto target = g_Renderer.GetAbsEntityBonePosition(item.Index, boneID, REF_DIR);

		auto dir = target - origin;
		dir.Normalize();
		return Geometry::ConvertDirectionToQuat(dir);
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

		// Animation missing; return early.
		if (animNumber < 0 || animNumber >= animObject.Animations.size())
		{
			TENLog(
				"Attempted to set missing animation " + std::to_string(animNumber) +
				((animObjectID == item.ObjectNumber) ? "" : (" from moveable " + GetObjectName(animObjectID))) +
				" for object " + GetObjectName(item.ObjectNumber),
				LogLevel::Warning);

			return;
		}

		const auto& anim = GetAnimData(animObject, animNumber);

		// Frame missing; return early.
		if (frameNumber < 0 || frameNumber > anim.EndFrameNumber)
		{
			TENLog(
				"Attempted to set missing frame " + std::to_string(frameNumber) +
				" from animation " + std::to_string(animNumber) +
				((animObjectID == item.ObjectNumber) ? "" : (" from moveable " + GetObjectName(animObjectID))) +
				" for object " + GetObjectName(item.ObjectNumber),
				LogLevel::Warning);

			return;
		}

		item.Animation.AnimObjectID = animObjectID;
		item.Animation.AnimNumber = animNumber;
		item.Animation.FrameNumber = frameNumber;
		item.Animation.ActiveState =
			item.Animation.TargetState = anim.StateID;
	}

	void SetAnimation(ItemInfo& item, int animNumber, int frameNumber)
	{
		SetAnimation(item, item.ObjectNumber, animNumber, frameNumber);
	}

	bool SetStateDispatch(ItemInfo& item, int targetStateID)
	{
		// Set target state ID.
		if (targetStateID != NO_VALUE)
			item.Animation.TargetState = targetStateID;

		const auto& anim = GetAnimData(item);

		// No dispatches; return early.
		if (anim.Dispatches.empty())
			return false;

		// Active and target state IDs already match; return early.
		if (item.Animation.ActiveState == item.Animation.TargetState)
			return false;

		// Iterate over state dispatches.
		for (const auto& dispatch : anim.Dispatches)
		{
			// State ID mismatch; continue.
			if (dispatch.StateID != item.Animation.TargetState)
				continue;

			// Set new animation and frame numbers if current frame number is within dispatch range.
			if (TestAnimFrameRange(item, dispatch.FrameNumberRange.first, dispatch.FrameNumberRange.second))
			{
				item.Animation.AnimNumber = dispatch.NextAnimNumber;
				item.Animation.FrameNumber = dispatch.NextFrameNumber;
				return true;
			}
		}

		return false;
	}

	void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down, float right)
	{
		item->Pose.Translate(headingAngle, forward, down, right);
	}

	void TranslateItem(ItemInfo* item, const EulerAngles& orient, float dist)
	{
		item->Pose.Translate(orient, dist);
	}

	void TranslateItem(ItemInfo* item, const Vector3& dir, float dist)
	{
		item->Pose.Translate(dir, dist);
	}

	void ClampRotation(Pose& outPose, short angle, short rot)
	{
		if (angle <= rot)
		{
			outPose.Orientation.y += (angle >= -rot) ? angle : -rot;
		}
		else
		{
			outPose.Orientation.y += rot;
		}
	}

	// TODO: Refactor. Empty stub because moveable drawing is disabled when DrawRoutine pointer is nullptr in ObjectInfo.
	void DrawAnimatingItem(ItemInfo* item)
	{
	}
//}
