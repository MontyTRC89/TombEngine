#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/rope.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// NOTE: 0 frames counts as 1.
static unsigned int GetNonZeroFrameCount(const AnimData& anim)
{
	unsigned int frameCount = anim.frameEnd - anim.frameBase;
	return ((frameCount > 0) ? frameCount : 1);
}

static void PerformAnimCommands(ItemInfo& item, bool isFrameBased)
{
	const auto& anim = GetAnimData(item);

	// No commands; return early.
	if (anim.NumCommands == 0)
		return;

	// Get command data pointer.
	short* commandDataPtr = &g_Level.Commands[anim.CommandIndex];

	for (int i = anim.NumCommands; i > 0; i--)
	{
		auto animCommand = (AnimCommandType)commandDataPtr[0];
		commandDataPtr++;

		switch (animCommand)
		{
		case AnimCommandType::MoveOrigin:
			if (!isFrameBased)
			{
				TranslateItem(&item, item.Pose.Orientation.y, commandDataPtr[2], commandDataPtr[1], commandDataPtr[0]);

				if (item.IsLara())
				{
					auto bounds = GameBoundingBox(&item);
					UpdateLaraRoom(&item, -bounds.GetHeight() / 2, -commandDataPtr[0], -commandDataPtr[2]);
				}
				else
				{
					UpdateItemRoom(item.Index);
				}
			}

			commandDataPtr += 3;
			break;

		case AnimCommandType::JumpVelocity:
			if (!isFrameBased)
			{
				item.Animation.IsAirborne = true;
				item.Animation.Velocity.y = commandDataPtr[0];
				item.Animation.Velocity.z = commandDataPtr[1];

				if (item.IsLara())
				{
					auto& player = GetLaraInfo(item);

					if (player.Control.CalculatedJumpVelocity != 0)
					{
						item.Animation.Velocity.y = player.Control.CalculatedJumpVelocity;
						player.Control.CalculatedJumpVelocity = 0;
					}
				}
			}

			commandDataPtr += 2;
			break;

		case AnimCommandType::Deactivate:
			if (!isFrameBased)
			{
				if (Objects[item.ObjectNumber].intelligent && !item.AfterDeath)
					item.AfterDeath = 1;

				item.Status = ITEM_DEACTIVATED;
			}

			break;

		case AnimCommandType::AttackReady:
			if (!isFrameBased && item.IsLara())
			{
				auto& player = GetLaraInfo(item);

				if (player.Control.HandStatus != HandStatus::Special)
					player.Control.HandStatus = HandStatus::Free;
			}

			break;

		case AnimCommandType::SoundEffect:
			if (isFrameBased && item.Animation.FrameNumber == commandDataPtr[0])
			{
				if (!Objects[item.ObjectNumber].waterCreature)
				{
					bool playInWater = (commandDataPtr[1] & 0x8000) != 0;
					bool playOnLand	 = (commandDataPtr[1] & 0x4000) != 0;
					bool playAlways	 = (playInWater && playOnLand) || (!playInWater && !playOnLand);

					if (item.IsLara())
					{
						auto& player = GetLaraInfo(item);

						if (playAlways ||
							(playOnLand && (player.WaterSurfaceDist >= -SHALLOW_WATER_DEPTH || player.WaterSurfaceDist == NO_HEIGHT)) ||
							(playInWater && player.WaterSurfaceDist < -SHALLOW_WATER_DEPTH && player.WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, &item)))
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item.Pose, SoundEnvironment::Always);
						}
					}
					else
					{
						if (item.RoomNumber == NO_ROOM)
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item.Pose, SoundEnvironment::Always);
						}
						else if (TestEnvironment(ENV_FLAG_WATER, &item))
						{
							if (playAlways || (playInWater && TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber)))
								SoundEffect(commandDataPtr[1] & 0x3FFF, &item.Pose, SoundEnvironment::Always);
						}
						else if (playAlways || (playOnLand && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber) && !TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.RoomNumber)))
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item.Pose, SoundEnvironment::Always);
						}
					}
				}
				else
				{
					SoundEffect(commandDataPtr[1] & 0x3FFF, &item.Pose, TestEnvironment(ENV_FLAG_WATER, &item) ? SoundEnvironment::Water : SoundEnvironment::Land);
				}
			}

			commandDataPtr += 2;
			break;

		case AnimCommandType::Flipeffect:
			if (isFrameBased && item.Animation.FrameNumber == commandDataPtr[0])
				DoFlipEffect((commandDataPtr[1] & 0x3FFF), &item);

			commandDataPtr += 2;
			break;

		default:
			break;
		}
	}
}

void AnimateItem(ItemInfo* item)
{
	if (!item->IsLara())
	{
		item->TouchBits.ClearAll();
		item->HitStatus = false;
	}

	PerformAnimCommands(*item, true);
	item->Animation.FrameNumber++;

	const auto* animPtr = &GetAnimData(*item);

	if (animPtr->NumStateDispatches > 0 && GetStateDispatch(item, *animPtr))
	{
		animPtr = &GetAnimData(*item);

		item->Animation.ActiveState = animPtr->ActiveState;

		if (!item->IsLara())
		{
			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
	}

	if (item->Animation.FrameNumber > animPtr->frameEnd)
	{
		PerformAnimCommands(*item, false);

		item->Animation.AnimNumber = animPtr->JumpAnimNum;
		item->Animation.FrameNumber = animPtr->JumpFrameNum;

		animPtr = &GetAnimData(*item);

		if (!item->IsLara())
		{
			if (item->Animation.ActiveState != animPtr->ActiveState)
			{
				item->Animation.ActiveState = animPtr->ActiveState;
				item->Animation.TargetState = animPtr->ActiveState;
			}

			if (item->Animation.RequiredState == item->Animation.ActiveState)
				item->Animation.RequiredState = NO_STATE;
		}
		else
		{
			item->Animation.ActiveState = animPtr->ActiveState;
		}
	}

	unsigned int frameCount = GetNonZeroFrameCount(*animPtr);
	int currentFrame = item->Animation.FrameNumber - animPtr->frameBase;

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

bool HasStateDispatch(ItemInfo* item, int targetState)
{
	const auto& anim = GetAnimData(*item);

	// No dispatches; return early.
	if (anim.NumStateDispatches <= 0)
		return false;

	if (targetState == NO_STATE)
		targetState = item->Animation.TargetState;

	// Iterate over animation's state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];
		
		if (dispatch.TargetState != targetState)
			continue;

		// Iterate over dispatch frame ranges.
		for (int j = 0; j < dispatch.NumberRanges; j++)
		{
			const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];

			// Check if current frame is within dispatch range.
			if (item->Animation.FrameNumber >= range.StartFrame &&
				item->Animation.FrameNumber <= range.EndFrame)
			{
				return true;
			}
		}
	}

	return false;
}

bool TestAnimNumber(const ItemInfo& item, int animNumber)
{
	const auto& object = Objects[item.ObjectNumber];
	return (item.Animation.AnimNumber == (object.animIndex + animNumber));
}

bool TestLastFrame(ItemInfo* item, int animNumber)
{
	if (animNumber == NO_ANIM)
		animNumber = item->Animation.AnimNumber;

	if (item->Animation.AnimNumber != animNumber)
		return false;

	const auto& anim = GetAnimData(*item, animNumber);
	return (item->Animation.FrameNumber >= anim.frameEnd);
}

bool TestAnimFrame(const ItemInfo& item, int frameStart)
{
	const auto& anim = GetAnimData(item);
	return (item.Animation.FrameNumber == (anim.frameBase + frameStart));
}

bool TestAnimFrameRange(const ItemInfo& item, int frameStart, int frameEnd)
{
	const auto& anim = GetAnimData(item);
	return (item.Animation.FrameNumber >= (anim.frameBase + frameStart) &&
			item.Animation.FrameNumber <= (anim.frameBase + frameEnd));
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

void SetAnimation(ItemInfo* item, int animNumber, int frameNumber)
{
	const auto& object = Objects[item->ObjectNumber];
	int animIndex = object.animIndex + animNumber;

	// Animation already set; return early.
	if (item->Animation.AnimNumber == animIndex)
		return;

	// Animation doesn't exist; return early.
	if (animIndex < 0 || animIndex >= g_Level.Anims.size())
	{
		TENLog(
			std::string("Attempted to set nonexistent animation ") + std::to_string(animNumber) +
			std::string(" for object ") + std::to_string(item->ObjectNumber),
			LogLevel::Warning);

		return;
	}

	const auto& anim = GetAnimData(*item, animNumber);
	
	item->Animation.AnimNumber = animIndex;
	item->Animation.FrameNumber = anim.frameBase + frameNumber;
	item->Animation.ActiveState =
	item->Animation.TargetState = anim.ActiveState;
}

AnimData& GetAnimData(int animIndex)
{
	return g_Level.Anims[animIndex];
}

AnimData& GetAnimData(const ObjectInfo& object, int animNumber)
{
	return g_Level.Anims[object.animIndex + animNumber];
}

AnimData& GetAnimData(const ItemInfo& item, int animNumber)
{
	if (animNumber == NO_ANIM)
		return g_Level.Anims[item.Animation.AnimNumber];

	const auto& object = Objects[item.ObjectNumber];
	return GetAnimData(object, animNumber);
}

bool GetStateDispatch(ItemInfo* item, const AnimData& anim)
{
	// Active and target states already match; return early.
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	// No dispatches; return early.
	if (anim.NumStateDispatches <= 0)
		return false;

	// Iterate over animation's state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];

		if (dispatch.TargetState != item->Animation.TargetState)
			continue;

		// Iterate over dispatch frame ranges.
		for (int j = 0; j < dispatch.NumberRanges; j++)
		{
			const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];

			// Set new animation if current frame is within dispatch range.
			if (item->Animation.FrameNumber >= range.StartFrame &&
				item->Animation.FrameNumber <= range.EndFrame)
			{
				item->Animation.AnimNumber = range.LinkAnimNum;
				item->Animation.FrameNumber = range.LinkFrameNum;
				return true;
			}
		}
	}

	return false;
}

AnimFrameInterpData GetFrameInterpData(const ItemInfo& item)
{
	const auto& anim = GetAnimData(item);

	// Normalize animation's current frame number into keyframe range.
	int frameNumber = item.Animation.FrameNumber - anim.frameBase;
	float frameNumberNorm = frameNumber / (float)anim.Interpolation;

	// Calculate keyframe numbers defining interpolated frame and get pointers to them.
	int frame0 = (int)floor(frameNumberNorm);
	int frame1 = (int)ceil(frameNumberNorm);
	auto* framePtr0 = &g_Level.Frames[anim.FramePtr + frame0];
	auto* framePtr1 = &g_Level.Frames[anim.FramePtr + frame1];

	// Calculate interpolation alpha between keyframes.
	float alpha = (1.0f / anim.Interpolation) * (frameNumber % anim.Interpolation);

	// Return frame interpolation data.
	return AnimFrameInterpData{ framePtr0, framePtr1, alpha };
}

AnimFrame& GetAnimFrame(const ItemInfo& item, int animNumber, int frameNumber)
{
	return *GetFrame(item.ObjectNumber, animNumber, frameNumber);
}

AnimFrame* GetFrame(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	const auto& object = Objects[objectID];

	int animIndex = object.animIndex + animNumber;
	assertion(animIndex < g_Level.Anims.size(), "GetFrame() attempted to access nonexistent animation.");

	const auto& anim = GetAnimData(object, animNumber);

	// Get and clamp frame count.
	unsigned int frameCount = anim.frameEnd - anim.frameBase;
	if (frameNumber > frameCount)
		frameNumber = frameCount;

	// Interpolate and return frame pointer.
	auto* framePtr = &g_Level.Frames[anim.FramePtr];
	framePtr += frameNumber / anim.Interpolation;
	return framePtr;
}

AnimFrame* GetFirstFrame(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetFrame(objectID, animNumber, 0);
}

AnimFrame* GetLastFrame(GAME_OBJECT_ID objectID, int animNumber)
{
	return GetFrame(objectID, animNumber, INT_MAX);
}

AnimFrame& GetBestFrame(const ItemInfo& item)
{
	auto frameData = GetFrameInterpData(item);
	if (frameData.Alpha <= 0.5f)
		return *frameData.FramePtr0;
	else
		return *frameData.FramePtr1;
}

int GetCurrentRelativeFrameNumber(ItemInfo* item)
{
	return (item->Animation.FrameNumber - GetFrameNumber(item, 0));
}

// NOTE: Returns g_Level.Anims index.
int GetAnimNumber(ItemInfo& item, int animNumber)
{
	const auto& object = Objects[item.ObjectNumber];
	return (object.animIndex + animNumber);
}

int GetFrameNumber(ItemInfo* item, int frameToStart)
{
	int animNumber = item->Animation.AnimNumber - Objects[item->ObjectNumber].animIndex;
	return GetFrameNumber(item->ObjectNumber, animNumber, frameToStart);
}

int GetFrameNumber(int objectID, int animNumber, int frameToStart)
{
	const auto& object = Objects[objectID];
	const auto& anim = GetAnimData(object, animNumber);

	return (anim.frameBase + frameToStart);
}

int GetFrameCount(int animIndex)
{
	if (animIndex < 0 || g_Level.Anims.size() <= animIndex)
		return 0;

	const auto& anim = GetAnimData(animIndex);

	int end = anim.frameEnd;
	int base = anim.frameBase;
	return (end - base);
}

int GetNextAnimState(ItemInfo* item)
{
	return GetNextAnimState(item->ObjectNumber, item->Animation.AnimNumber);
}

int GetNextAnimState(int objectID, int animNumber)
{
	const auto& object = Objects[objectID];
	const auto& anim = GetAnimData(object, animNumber);

	const auto& nextAnim = GetAnimData(anim.JumpAnimNum);
	return nextAnim.ActiveState;
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
		if (angle >= -rotation)
			outPose.Orientation.y += angle;
		else
			outPose.Orientation.y -= rotation;
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

Vector3 GetJointOffset(GAME_OBJECT_ID objectID, int jointIndex)
{
	const auto& object = Objects[objectID];

	int* bonePtr = &g_Level.Bones[object.boneIndex + (jointIndex * 4)];
	return Vector3(*(bonePtr + 1), *(bonePtr + 2), *(bonePtr + 3));
}

Quaternion GetBoneOrientation(const ItemInfo& item, int boneIndex)
{
	static const auto REF_DIRECTION = Vector3::UnitZ;

	auto origin = g_Renderer.GetAbsEntityBonePosition(item.Index, boneIndex);
	auto target = g_Renderer.GetAbsEntityBonePosition(item.Index, boneIndex, REF_DIRECTION);

	auto direction = target - origin;
	direction.Normalize();
	return Geometry::ConvertDirectionToQuat(direction);
}

// NOTE: Will not work for bones at ends of hierarchies.
float GetBoneLength(GAME_OBJECT_ID objectID, int boneIndex)
{
	const auto& object = Objects[objectID];

	if (object.nmeshes == boneIndex)
		return 0.0f;

	auto nextBoneOffset = GetJointOffset(objectID, boneIndex + 1);
	return nextBoneOffset.Length();
}
