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
static unsigned int GetFrameCount(const AnimData& anim)
{
	unsigned int frameCount = anim.frameEnd - anim.frameBase;
	return ((frameCount > 0) ? frameCount : 1);
}

static void PerformAnimCommands(ItemInfo* item, bool isFrameBased)
{
	const auto& anim = GetAnimData(*item);

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
				TranslateItem(item, item->Pose.Orientation.y, commandDataPtr[2], commandDataPtr[1], commandDataPtr[0]);

				if (item->IsLara())
				{
					auto bounds = GameBoundingBox(item);
					UpdateLaraRoom(item, -bounds.GetHeight() / 2, -commandDataPtr[0], -commandDataPtr[2]);
				}
				else
				{
					UpdateItemRoom(item->Index);
				}
			}

			commandDataPtr += 3;
			break;

		case AnimCommandType::JumpVelocity:
			if (!isFrameBased)
			{
				item->Animation.Velocity.y = commandDataPtr[0];
				item->Animation.Velocity.z = commandDataPtr[1];
				item->Animation.IsAirborne = true;

				if (item->IsLara())
				{
					auto& player = *GetLaraInfo(item);

					if (player.Control.CalculatedJumpVelocity != 0)
					{
						item->Animation.Velocity.y = player.Control.CalculatedJumpVelocity;
						player.Control.CalculatedJumpVelocity = 0;
					}
				}
			}

			commandDataPtr += 2;
			break;

		case AnimCommandType::Deactivate:
			if (!isFrameBased)
			{
				if (Objects[item->ObjectNumber].intelligent && !item->AfterDeath)
					item->AfterDeath = 1;

				item->Status = ITEM_DEACTIVATED;
			}

			break;

		case AnimCommandType::AttackReady:
			if (!isFrameBased && item->IsLara())
			{
				auto& player = *GetLaraInfo(item);

				if (player.Control.HandStatus != HandStatus::Special)
					player.Control.HandStatus = HandStatus::Free;
			}

			break;

		case AnimCommandType::SoundEffect:
			if (isFrameBased && item->Animation.FrameNumber == commandDataPtr[0])
			{
				if (!Objects[item->ObjectNumber].waterCreature)
				{
					bool playInWater = (commandDataPtr[1] & 0x8000) != 0;
					bool playOnLand	 = (commandDataPtr[1] & 0x4000) != 0;
					bool playAlways	 = (playInWater && playOnLand) || (!playInWater && !playOnLand);

					if (item->IsLara())
					{
						auto& player = *GetLaraInfo(item);

						if (playAlways ||
							(playOnLand && (player.WaterSurfaceDist >= -SHALLOW_WATER_DEPTH || player.WaterSurfaceDist == NO_HEIGHT)) ||
							(playInWater && player.WaterSurfaceDist < -SHALLOW_WATER_DEPTH && player.WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
					}
					else
					{
						if (item->RoomNumber == NO_ROOM)
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
						else if (TestEnvironment(ENV_FLAG_WATER, item))
						{
							if (playAlways || (playInWater && TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber)))
								SoundEffect(commandDataPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
						else if (playAlways || (playOnLand && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber) && !TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.RoomNumber)))
						{
							SoundEffect(commandDataPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
					}
				}
				else
				{
					SoundEffect(commandDataPtr[1] & 0x3FFF, &item->Pose, TestEnvironment(ENV_FLAG_WATER, item) ? SoundEnvironment::Water : SoundEnvironment::Land);
				}
			}

			commandDataPtr += 2;
			break;

		case AnimCommandType::Flipeffect:
			if (isFrameBased && item->Animation.FrameNumber == commandDataPtr[0])
				DoFlipEffect((commandDataPtr[1] & 0x3FFF), item);

			commandDataPtr += 2;
			break;

		default:
			break;
		}
	}
}

void AnimateLara(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	PerformAnimCommands(item, true);

	item->Animation.FrameNumber++;

	auto* animPtr = &GetAnimData(*item);

	if (animPtr->NumStateDispatches > 0 && GetStateDispatch(item, *animPtr))
	{
		animPtr = &GetAnimData(*item);
		item->Animation.ActiveState = animPtr->ActiveState;
	}

	if (item->Animation.FrameNumber > animPtr->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = animPtr->JumpAnimNum;
		item->Animation.FrameNumber = animPtr->JumpFrameNum;

		animPtr = &GetAnimData(*item);
		item->Animation.ActiveState = animPtr->ActiveState;
	}

	unsigned int frameCount = GetFrameCount(*animPtr);
	int currentFrame = item->Animation.FrameNumber - animPtr->frameBase;

	if (item->Animation.IsAirborne)
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
		if (lara->Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
			item->Animation.Velocity.z = (animPtr->VelocityStart.z / 2) + ((((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame) / 4);
		else
			item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame);
	}

	item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrame);

	if (lara->Control.Rope.Ptr != -1)
		DelAlignLaraToRope(item);

	if (!lara->Control.IsMoving)
		TranslateItem(item, lara->Control.MoveAngle, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

	// Update matrices.
	g_Renderer.UpdateLaraAnimations(true);
}

void AnimateItem(ItemInfo* item)
{
	item->TouchBits.ClearAll();
	item->HitStatus = false;

	PerformAnimCommands(item, true);

	item->Animation.FrameNumber++;

	auto* animPtr = &GetAnimData(*item);

	if (animPtr->NumStateDispatches > 0 && GetStateDispatch(item, *animPtr))
	{
		animPtr = &GetAnimData(*item);

		item->Animation.ActiveState = animPtr->ActiveState;
		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = NO_STATE;
	}

	if (item->Animation.FrameNumber > animPtr->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = animPtr->JumpAnimNum;
		item->Animation.FrameNumber = animPtr->JumpFrameNum;

		animPtr = &GetAnimData(*item);
		if (item->Animation.ActiveState != animPtr->ActiveState)
		{
			item->Animation.ActiveState = animPtr->ActiveState;
			item->Animation.TargetState = animPtr->ActiveState;
		}

		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = NO_STATE;
	}

	unsigned int frameCount = GetFrameCount(*animPtr);
	int currentFrame = item->Animation.FrameNumber - animPtr->frameBase;

	if (item->Animation.IsAirborne)
	{
		item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
		item->Pose.Position.y += item->Animation.Velocity.y;
	}
	else
	{
		item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrame);
		item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame);
	}
	
	TranslateItem(item, item->Pose.Orientation.y, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

	// Update matrices.
	g_Renderer.UpdateItemAnimations(item->Index, true);
}

bool HasStateDispatch(ItemInfo* item, int targetState)
{
	const auto& anim = GetAnimData(*item);

	// No dispatches; return early.
	if (anim.NumStateDispatches <= 0)
		return false;

	if (targetState == NO_STATE)
		targetState = item->Animation.TargetState;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];
		
		if (dispatch.TargetState != targetState)
			continue;

		// Iterate over frame range of state dispatch.
		for (int j = 0; j < dispatch.NumberRanges; j++)
		{
			const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];

			// Check if frame is within range.
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

	const auto& anim = GetAnimData(animNumber);
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

void SetAnimation(ItemInfo* item, int animNumber, int frameToStart)
{
	if (item->Animation.AnimNumber == animNumber)
		return;

	const auto& object = Objects[item->ObjectNumber];

	int index = object.animIndex + animNumber;
	if (index < 0 || index >= g_Level.Anims.size())
	{
		TENLog(
			std::string("Attempted to set nonexistent animation ") + std::to_string(animNumber) +
			std::string(" for object ") + std::to_string(item->ObjectNumber),
			LogLevel::Warning);

		return;
	}

	const auto& anim = GetAnimData(index);

	item->Animation.AnimNumber = index;
	item->Animation.FrameNumber = anim.frameBase + frameToStart;
	item->Animation.ActiveState = anim.ActiveState;
	item->Animation.TargetState = anim.ActiveState;
}

AnimData& GetAnimData(int animNumber)
{
	return g_Level.Anims[animNumber];
}

AnimData& GetAnimData(const ItemInfo& item)
{
	return g_Level.Anims[item.Animation.AnimNumber];
}

bool GetStateDispatch(ItemInfo* item, const AnimData& anim)
{
	// Active and target states match; return early.
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	// No dispatches; return early.
	if (anim.NumStateDispatches <= 0)
		return false;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];

		if (dispatch.TargetState != item->Animation.TargetState)
			continue;

		// Iterate over frame range of state dispatch.
		for (int j = 0; j < dispatch.NumberRanges; j++)
		{
			const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];

			// Check if frame is within range.
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

int GetFrame(ItemInfo* item, AnimFrame* outFramePtr[], int& outRate)
{
	const auto& anim = GetAnimData(*item);
	int frameNumber = item->Animation.FrameNumber;

	outFramePtr[0] =
	outFramePtr[1] = &g_Level.Frames[anim.FramePtr];
	int rate =
	outRate = anim.Interpolation & 0x00FF;
	frameNumber -= anim.frameBase;

	int first = frameNumber / rate;
	int interpolation = frameNumber % rate;
	outFramePtr[0] += first;			 // Get frame pointers...
	outFramePtr[1] = outFramePtr[0] + 1; // and store away.

	// No interpolation; return early.
	if (interpolation == 0)
		return 0;

	// Clamp key frame to end if necessary.
	int second = (first * rate) + rate;
	if (second > anim.frameEnd)
		outRate = anim.frameEnd - (second - rate);

	return interpolation;
}

AnimFrame* GetFrame(GAME_OBJECT_ID objectID, int animNumber, int frameNumber)
{
	const auto& object = Objects[objectID];

	int animIndex = object.animIndex + animNumber;
	assertion(animIndex < g_Level.Anims.size(), "GetFrame() attempted to access nonexistent animation.");

	const auto& anim = GetAnimData(animIndex);

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

AnimFrame* GetBestFrame(ItemInfo* item)
{
	int rate = 0;
	AnimFrame* framePtr[2];
	int frac = GetFrame(item, framePtr, rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
}

int GetCurrentRelativeFrameNumber(ItemInfo* item)
{
	return item->Animation.FrameNumber - GetFrameNumber(item, 0);
}

int GetAnimNumber(ItemInfo& item, int animID)
{
	return Objects[item.ObjectNumber].animIndex + animID;
}

int GetFrameNumber(ItemInfo* item, int frameToStart)
{
	int animNumber = item->Animation.AnimNumber - Objects[item->ObjectNumber].animIndex;
	return GetFrameNumber(item->ObjectNumber, animNumber, frameToStart);
}

int GetFrameNumber(int objectID, int animNumber, int frameToStart)
{
	const auto& object = Objects[objectID];
	const auto& anim = GetAnimData(object.animIndex + animNumber);

	return (anim.frameBase + frameToStart);
}

int GetFrameCount(int animNumber)
{
	if (animNumber < 0 || g_Level.Anims.size() <= animNumber)
		return 0;

	const auto& anim = GetAnimData(animNumber);

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
	const auto& anim = GetAnimData(object.animIndex + animNumber);

	int nextAnimNumber = anim.JumpAnimNum;
	const auto& nextAnim = GetAnimData(object.animIndex + nextAnimNumber);
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

Vector3i GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& relOffset)
{
	// Use matrices done in renderer to transform relative offset.
	return Vector3i(g_Renderer.GetAbsEntityBonePosition(item->Index, jointIndex, relOffset.ToVector3()));
}
