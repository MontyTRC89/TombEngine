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

namespace TEN::Animation
{
	constexpr auto VERTICAL_VELOCITY_GRAVITY_THRESHOLD = CLICK(0.5f);

	// TODO: Arm anim object in savegame.

	FrameInterpData::FrameInterpData(const KeyframeData& keyframe0, const KeyframeData& keyframe1, float alpha) :
		Keyframe0(keyframe0),
		Keyframe1(keyframe1)
	{
		Alpha = alpha;
	}

	const KeyframeData& AnimData::GetClosestKeyframe(int frameNumber) const
	{
		auto frameInterp = GetFrameInterpolation(frameNumber);
		return ((frameInterp.Alpha <= 0.5f) ? frameInterp.Keyframe0 : frameInterp.Keyframe1);
	}

	FrameInterpData AnimData::GetFrameInterpolation(int frameNumber) const
	{
		// FAILSAFE: Clamp frame number.
		frameNumber = std::clamp(frameNumber, 0, EndFrameNumber);

		// Calculate keyframes defining interpolated frame.
		float keyframeNumber = frameNumber / (float)Interpolation;
		const auto& keyframe0 = Keyframes[(int)floor(keyframeNumber)];
		const auto& keyframe1 = Keyframes[(int)ceil(keyframeNumber)];

		// Calculate interpolation alpha between keyframes.
		float alpha = (1.0f / Interpolation) * (frameNumber % Interpolation);

		// Return frame interpolation.
		return FrameInterpData(keyframe0, keyframe1, alpha);
	}

	FixedMotionData AnimData::GetFixedMotion(int frameNumber) const
	{
		// NOTE: Must use non-zero frame count in this edge case.
		unsigned int frameCount = std::max(1, EndFrameNumber);

		// Calculate relative translation and curve alpha.
		float alpha = (float)frameNumber / (float)frameCount;
		auto translation = Vector3(
			(FixedMotionCurveX != BezierCurve2D::Zero) ? FixedMotionCurveX.GetY(alpha) : 0.0f,
			(FixedMotionCurveY != BezierCurve2D::Zero) ? FixedMotionCurveY.GetY(alpha) : 0.0f,
			(FixedMotionCurveZ != BezierCurve2D::Zero) ? FixedMotionCurveZ.GetY(alpha) : 0.0f);

		// Return fixed motion.
		return FixedMotionData{ translation, alpha };
	}

	RootMotionData AnimData::GetRootMotion(int frameNumber) const
	{
		// Test for root motion flags.
		bool hasTranslation = Flags & ((int)AnimFlags::RootMotionTranslationX | (int)AnimFlags::RootMotionTranslationY | (int)AnimFlags::RootMotionTranslationZ);
		bool hasRot = Flags & ((int)AnimFlags::RootMotionRotationX | (int)AnimFlags::RootMotionRotationY | (int)AnimFlags::RootMotionRotationZ);
		if (!hasTranslation && !hasRot)
			return {};

		// Handle frame 0.
		if (frameNumber == 0)
		{
			if (Flags & (int)AnimFlags::RootMotionCycle)
			{
				if (Keyframes.size() > 1)
				{
					frameNumber = 1;
				}
				else
				{
					TENLog("Attempted to get anim root motion for frame 0 from cycle with 1 frame.", LogLevel::Info);
					return {};
				}
			}
			else
			{
				return {};
			}
		}

		// Get frame interpolation.
		auto frameInterp = GetFrameInterpolation(frameNumber);
		auto prevFrameInterp = GetFrameInterpolation(frameNumber - 1);
		
		// Calculate relative translation.
		auto translation = Vector3::Zero;
		if (hasTranslation)
		{
			auto rootPos = Vector3::Lerp(frameInterp.Keyframe0.RootPosition, frameInterp.Keyframe1.RootPosition, frameInterp.Alpha);
			auto prevRootPos = Vector3::Lerp(prevFrameInterp.Keyframe0.RootPosition, prevFrameInterp.Keyframe1.RootPosition, prevFrameInterp.Alpha);
			auto rootTranslation = rootPos - prevRootPos;

			if (Flags & (int)AnimFlags::RootMotionTranslationX)
				translation.x = rootTranslation.x;
			if (Flags & (int)AnimFlags::RootMotionTranslationY)
				translation.y = rootTranslation.y;
			if (Flags & (int)AnimFlags::RootMotionTranslationZ)
				translation.z = rootTranslation.z;
		}

		// Calculate relative rotation.
		auto rot = EulerAngles::Identity;
		if (hasRot)
		{
			auto rootOrient = EulerAngles(Quaternion::Slerp(frameInterp.Keyframe0.BoneOrientations.front(), frameInterp.Keyframe1.BoneOrientations.front(), frameInterp.Alpha));
			auto prevRootOrient = EulerAngles(Quaternion::Slerp(prevFrameInterp.Keyframe0.BoneOrientations.front(), prevFrameInterp.Keyframe1.BoneOrientations.front(), prevFrameInterp.Alpha));
			auto rootRot = rootOrient - prevRootOrient;

			if (Flags & (int)AnimFlags::RootMotionRotationX)
				rot.x = rootRot.x;
			if (Flags & (int)AnimFlags::RootMotionRotationY)
				rot.y = rootRot.y;
			if (Flags & (int)AnimFlags::RootMotionRotationZ)
				rot.z = rootRot.z;
		}

		// Return root motion.
		return RootMotionData{ translation, rot };
	}

	RootMotionData AnimData::GetRootMotionCounteraction(int frameNumber) const
	{
		// Test for root motion flags.
		bool hasTranslation = Flags & ((int)AnimFlags::RootMotionTranslationX | (int)AnimFlags::RootMotionTranslationY | (int)AnimFlags::RootMotionTranslationZ);
		bool hasRot = Flags & ((int)AnimFlags::RootMotionRotationX | (int)AnimFlags::RootMotionRotationY | (int)AnimFlags::RootMotionRotationZ);
		if (!hasTranslation && !hasRot)
			return {};

		// Handle frame 0.
		if (frameNumber == 0)
			return {};

		// Get frame interpolation.
		auto frameInterp = GetFrameInterpolation(frameNumber);

		// Get relative translation counteraction.
		auto translation = Vector3::Zero;
		if (hasTranslation)
		{
			auto basePos = Keyframes.front().RootPosition;
			auto rootPos = Vector3::Lerp(frameInterp.Keyframe0.RootPosition, frameInterp.Keyframe1.RootPosition, frameInterp.Alpha);

			if (Flags & (int)AnimFlags::RootMotionTranslationX)
				translation.x = basePos.x - rootPos.x;
			if (Flags & (int)AnimFlags::RootMotionTranslationY)
				translation.y = basePos.y - rootPos.y;
			if (Flags & (int)AnimFlags::RootMotionTranslationZ)
				translation.z = basePos.z - rootPos.z;
		}

		// Get relative rotation counteraction.
		auto rot = EulerAngles::Identity;
		if (hasRot)
		{
			auto baseOrient = EulerAngles(Keyframes.front().BoneOrientations.front());
			auto rootOrient = EulerAngles(Quaternion::Slerp(frameInterp.Keyframe0.BoneOrientations.front(), frameInterp.Keyframe1.BoneOrientations.front(), frameInterp.Alpha));

			if (Flags & (int)AnimFlags::RootMotionRotationX)
				rot.x = baseOrient.x - rootOrient.x;
			if (Flags & (int)AnimFlags::RootMotionRotationY)
				rot.y = baseOrient.y - rootOrient.y;
			if (Flags & (int)AnimFlags::RootMotionRotationZ)
				rot.z = baseOrient.z - rootOrient.z;
		}

		// Return root motion counteraction.
		return RootMotionData{ translation, rot };
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

		// Handle dispatch.
		const auto* dispatch = GetStateDispatch(item);
		if (dispatch != nullptr)
		{
			SetStateDispatch(item, *dispatch);
			anim = &GetAnimData(item);

			item.Animation.ActiveState = anim->StateID;

			if (!item.IsLara())
			{
				// Reset RequiredState if already reached.
				if (item.Animation.RequiredState == item.Animation.ActiveState)
					item.Animation.RequiredState = NO_VALUE;
			}
		}

		// Handle link.
		if (item.Animation.FrameNumber > anim->EndFrameNumber)
		{
			ExecuteAnimCommands(item, false);

			item.SetAnimBlend(anim->BlendFrameCount, anim->BlendCurve);
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
		
		// Update blend.
		if (item.Animation.Blend.IsEnabled())
		{
			item.Animation.Blend.FrameNumber++;
			if (item.Animation.Blend.FrameNumber > item.Animation.Blend.FrameCount)
				item.DisableAnimBlend();
		}

		// Get fixed motion and root motion.
		auto fixedMotion = anim->GetFixedMotion(item.Animation.FrameNumber);
		auto rootMotion = anim->GetRootMotion(item.Animation.FrameNumber);

		// TODO: Better handling.
		// Apply motion translation and gravity.
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
			// TODO: Y velocity.
			if (item.IsLara())
			{
				const auto& player = GetLaraInfo(item);

				bool isInSwamp = (player.Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, &item));
				item.Animation.Velocity.x = (fixedMotion.Translation.x + rootMotion.Translation.x) * (isInSwamp ? 0.5f : 1.0f);
				item.Animation.Velocity.z = (fixedMotion.Translation.z + rootMotion.Translation.z) * (isInSwamp ? 0.5f : 1.0f);
			}
			else
			{
				item.Animation.Velocity.x = fixedMotion.Translation.x + rootMotion.Translation.x;
				item.Animation.Velocity.z = fixedMotion.Translation.z + rootMotion.Translation.z;
			}
		}

		// Update animation.
		if (item.IsLara())
		{
			const auto& player = GetLaraInfo(item);

			if (player.Control.Rope.Ptr != NO_VALUE)
				DelAlignLaraToRope(&item);

			if (!player.Control.IsMoving)
			{
				item.Pose.Translate(player.Control.MoveAngle, item.Animation.Velocity.z, 0.0f, item.Animation.Velocity.x);
				item.Pose.Orientation += rootMotion.Rotation;
			}

			g_Renderer.UpdateLaraAnimations(true);
		}
		else
		{
			item.Pose.Translate(item.Pose.Orientation.y, item.Animation.Velocity.z, 0.0f, item.Animation.Velocity.x);
			item.Pose.Orientation += rootMotion.Rotation;
			g_Renderer.UpdateItemAnimations(item.Index, true);
		}

		// Debug
		if (item.IsLara())
		{
			PrintDebugMessage(std::string(std::string("Fixed motion: ") + ((fixedMotion.Translation != Vector3::Zero) ? "Yes" : "No")).c_str());
			PrintDebugMessage(std::string(std::string("Root motion: ") + ((rootMotion.Translation != Vector3::Zero && rootMotion.Rotation != EulerAngles::Identity) ? "Yes" : "No")).c_str());
			PrintDebugMessage(std::string(std::string("Blend enabled: ") + (item.Animation.Blend.IsEnabled() ? "Yes" : "No")).c_str());
			PrintDebugMessage("Frame number: %d", item.Animation.Blend.FrameNumber);
			PrintDebugMessage("Frame count: %d", item.Animation.Blend.FrameCount);
		}
	}

	bool TestStateDispatch(const ItemInfo& item, int targetStateID)
	{
		const auto* dispatch = GetStateDispatch(item, targetStateID);
		return (dispatch != nullptr);
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

	bool TestAnimFrameRange(const ItemInfo& item, int frameNumberLow, int frameNumberHigh)
	{
		return (item.Animation.FrameNumber >= frameNumberLow &&
				item.Animation.FrameNumber <= frameNumberHigh);
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

	FrameInterpData GetFrameInterpolation(const ItemInfo& item)
	{
		const auto& anim = GetAnimData(item);
		return anim.GetFrameInterpolation(item.Animation.FrameNumber);
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

	const StateDispatchData* GetStateDispatch(const ItemInfo& item, int targetStateID)
	{
		const auto& anim = GetAnimData(item);

		// Run through state dispatches.
		for (const auto& dispatch : anim.Dispatches)
		{
			// State ID mismatch; continue.
			if (dispatch.StateID != ((targetStateID == NO_VALUE) ? item.Animation.TargetState : targetStateID))
				continue;

			// Test if current frame is within dispatch range.
			if (TestAnimFrameRange(item, dispatch.FrameNumberLow, dispatch.FrameNumberHigh))
				return &dispatch;
		}

		return nullptr;
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
		return g_Renderer.GetMoveableBoneOrientation(item.Index, boneID);
	}

	void SetAnimation(ItemInfo& item, GAME_OBJECT_ID animObjectID, int animNumber, int frameNumber, int blendFrameCount, const BezierCurve2D& blendCurve)
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

		item.SetAnimBlend(blendFrameCount, blendCurve);
		item.Animation.AnimObjectID = animObjectID;
		item.Animation.AnimNumber = animNumber;
		item.Animation.FrameNumber = frameNumber;
		item.Animation.ActiveState =
		item.Animation.TargetState = anim.StateID;
	}

	void SetAnimation(ItemInfo& item, int animNumber, int frameNumber, int blendFrameCount, const BezierCurve2D& blendCurve)
	{
		SetAnimation(item, item.ObjectNumber, animNumber, frameNumber, blendFrameCount, blendCurve);
	}

	void SetStateDispatch(ItemInfo& item, const StateDispatchData& dispatch)
	{
		float frameNumber = Remap(
			item.Animation.FrameNumber,
			dispatch.FrameNumberLow, dispatch.FrameNumberHigh,
			dispatch.NextFrameNumberLow, dispatch.NextFrameNumberHigh);

		item.SetAnimBlend(dispatch.BlendFrameCount, dispatch.BlendCurve);
		item.Animation.AnimNumber = dispatch.NextAnimNumber;
		item.Animation.FrameNumber = (int)round(frameNumber);
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
}
