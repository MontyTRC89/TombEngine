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

void AnimateLara(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	PerformAnimCommands(item, true);

	item->Animation.FrameNumber++;

	auto* animPtr = &g_Level.Anims[item->Animation.AnimNumber];

	if (animPtr->NumStateDispatches > 0 && GetStateDispatch(item, *animPtr))
	{
		animPtr = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = animPtr->ActiveState;
	}

	if (item->Animation.FrameNumber > animPtr->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = animPtr->JumpAnimNum;
		item->Animation.FrameNumber = animPtr->JumpFrameNum;

		animPtr = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = animPtr->ActiveState;
	}

	int frameCount = animPtr->frameEnd - animPtr->frameBase;
	frameCount = (frameCount > 0) ? frameCount : 1;
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
			item->Animation.Velocity.z += (animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount;
			item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
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

void PerformAnimCommands(ItemInfo* item, bool isFrameBased)
{
	const auto& anim = g_Level.Anims[item->Animation.AnimNumber];

	if (anim.NumCommands == 0)
		return;

	short* commandPtr = &g_Level.Commands[anim.CommandIndex];

	for (int i = anim.NumCommands; i > 0; i--)
	{
		auto animCommand = (AnimCommandType)commandPtr[0];
		commandPtr++;

		switch (animCommand)
		{
		case AnimCommandType::MoveOrigin:
			if (!isFrameBased)
			{
				TranslateItem(item, item->Pose.Orientation.y, commandPtr[2], commandPtr[1], commandPtr[0]);

				if (item->IsLara())
				{
					auto bounds = GameBoundingBox(item);
					UpdateLaraRoom(item, -bounds.GetHeight() / 2, -commandPtr[0], -commandPtr[2]);
				}
				else
				{
					UpdateItemRoom(item->Index);
				}
			}

			commandPtr += 3;
			break;

		case AnimCommandType::JumpVelocity:
			if (!isFrameBased)
			{
				item->Animation.Velocity.y = commandPtr[0];
				item->Animation.Velocity.z = commandPtr[1];
				item->Animation.IsAirborne = true;

				if (item->IsLara())
				{
					auto& lara = *GetLaraInfo(item);

					if (lara.Control.CalculatedJumpVelocity)
					{
						item->Animation.Velocity.y = lara.Control.CalculatedJumpVelocity;
						lara.Control.CalculatedJumpVelocity = 0;
					}
				}
			}

			commandPtr += 2;
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
				auto& lara = *GetLaraInfo(item);

				if (lara.Control.HandStatus != HandStatus::Special)
					lara.Control.HandStatus = HandStatus::Free;
			}

			break;

		case AnimCommandType::SoundEffect:
			if (isFrameBased && item->Animation.FrameNumber == commandPtr[0])
			{
				if (!Objects[item->ObjectNumber].waterCreature)
				{
					bool inWater = (commandPtr[1] & 0x8000) != 0;
					bool onDry   = (commandPtr[1] & 0x4000) != 0;
					bool always  = (inWater && onDry) || (!inWater && !onDry);

					if (item->IsLara())
					{
						auto* lara = GetLaraInfo(item);

						if (always ||
						   (onDry && (lara->WaterSurfaceDist >= -SHALLOW_WATER_DEPTH || lara->WaterSurfaceDist == NO_HEIGHT)) ||
						   (inWater && lara->WaterSurfaceDist < -SHALLOW_WATER_DEPTH && lara->WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
						{
							SoundEffect(commandPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
					}
					else
					{
						if (item->RoomNumber == NO_ROOM)
						{
							SoundEffect(commandPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
						else if (TestEnvironment(ENV_FLAG_WATER, item))
						{
							if (always || (inWater && TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber)))
								SoundEffect(commandPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
						else if (always || (onDry && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.RoomNumber) && !TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.RoomNumber)))
							SoundEffect(commandPtr[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
					}
				}
				else
					SoundEffect(commandPtr[1] & 0x3FFF, &item->Pose, TestEnvironment(ENV_FLAG_WATER, item) ? SoundEnvironment::Water : SoundEnvironment::Land);
			}

			commandPtr += 2;
			break;

		case AnimCommandType::Flipeffect:
			if (isFrameBased && item->Animation.FrameNumber == commandPtr[0])
				DoFlipEffect((commandPtr[1] & 0x3FFF), item);

			commandPtr += 2;
			break;

		default:
			break;
		}
	}
}

void AnimateItem(ItemInfo* item)
{
	item->TouchBits = NO_JOINT_BITS;
	item->HitStatus = false;

	PerformAnimCommands(item, true);

	item->Animation.FrameNumber++;

	auto* animPtr = &g_Level.Anims[item->Animation.AnimNumber];

	if (animPtr->NumStateDispatches > 0 && GetStateDispatch(item, *animPtr))
	{
		animPtr = &g_Level.Anims[item->Animation.AnimNumber];

		item->Animation.ActiveState = animPtr->ActiveState;
		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	if (item->Animation.FrameNumber > animPtr->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = animPtr->JumpAnimNum;
		item->Animation.FrameNumber = animPtr->JumpFrameNum;

		animPtr = &g_Level.Anims[item->Animation.AnimNumber];
		if (item->Animation.ActiveState != animPtr->ActiveState)
		{
			item->Animation.ActiveState = animPtr->ActiveState;
			item->Animation.TargetState = animPtr->ActiveState;
		}

		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	int frameCount = animPtr->frameEnd - animPtr->frameBase;
	frameCount = (frameCount > 0) ? frameCount : 1;
	int currentFrame = item->Animation.FrameNumber - animPtr->frameBase;

	if (item->Animation.IsAirborne)
	{
		item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
		item->Pose.Position.y += item->Animation.Velocity.y;
	}
	else
	{
		item->Animation.Velocity.z = animPtr->VelocityStart.z + (((animPtr->VelocityEnd.z - animPtr->VelocityStart.z) / frameCount) * currentFrame);
		item->Animation.Velocity.x = animPtr->VelocityStart.x + (((animPtr->VelocityEnd.x - animPtr->VelocityStart.x) / frameCount) * currentFrame);
	}
	
	TranslateItem(item, item->Pose.Orientation.y, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

	// Update matrices.
	short itemNumber = item - g_Level.Items.data();
	g_Renderer.UpdateItemAnimations(itemNumber, true);
}

bool HasStateDispatch(ItemInfo* item, int targetState)
{
	const auto& anim = g_Level.Anims[item->Animation.AnimNumber];

	if (anim.NumStateDispatches <= 0)
		return false;

	if (targetState == NO_STATE)
		targetState = item->Animation.TargetState;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];
		if (dispatch.TargetState == targetState)
		{
			// Iterate over frame range of state dispatch.
			for (int j = 0; j < dispatch.NumberRanges; j++)
			{
				const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];
				if (item->Animation.FrameNumber >= range.StartFrame && item->Animation.FrameNumber <= range.EndFrame)
					return true;
			}
		}
	}

	return false;
}

bool TestLastFrame(ItemInfo* item, int animNumber)
{
	if (animNumber == NO_ANIM)
		animNumber = item->Animation.AnimNumber;

	if (item->Animation.AnimNumber != animNumber)
		return false;

	const auto& anim = g_Level.Anims[animNumber];

	return (item->Animation.FrameNumber >= anim.frameEnd);
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

void SetAnimation(ItemInfo* item, int animIndex, int frameToStart)
{
	if (item->Animation.AnimNumber == animIndex)
		return;

	int index = Objects[item->ObjectNumber].animIndex + animIndex;
	if (index < 0 || index >= g_Level.Anims.size())
	{
		TENLog(std::string("Attempted to set nonexistent animation ") + std::to_string(animIndex) + std::string(" for object ") + std::to_string(item->ObjectNumber), LogLevel::Warning);
		return;
	}

	item->Animation.AnimNumber = index;
	item->Animation.FrameNumber = g_Level.Anims[index].frameBase + frameToStart;
	item->Animation.ActiveState = g_Level.Anims[index].ActiveState;
	item->Animation.TargetState = item->Animation.ActiveState;
}

bool GetStateDispatch(ItemInfo* item, const AnimData& anim)
{
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	if (anim.NumStateDispatches <= 0)
		return false;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim.NumStateDispatches; i++)
	{
		const auto& dispatch = g_Level.Changes[anim.StateDispatchIndex + i];

		if (dispatch.TargetState == item->Animation.TargetState)
		{
			// Iterate over frame range of state dispatch.
			for (int j = 0; j < dispatch.NumberRanges; j++)
			{
				const auto& range = g_Level.Ranges[dispatch.RangeIndex + j];

				if (item->Animation.FrameNumber >= range.StartFrame && item->Animation.FrameNumber <= range.EndFrame)
				{
					item->Animation.AnimNumber = range.LinkAnimNum;
					item->Animation.FrameNumber = range.LinkFrameNum;
					return true;
				}
			}
		}
	}

	return false;
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

int GetFrame(ItemInfo* item, AnimFrame* outFramePtr[], int& outRate)
{
	int frame = item->Animation.FrameNumber;
	const auto& anim = g_Level.Anims[item->Animation.AnimNumber];

	outFramePtr[0] = outFramePtr[1] = &g_Level.Frames[anim.FramePtr];
	int rate2 = outRate = anim.Interpolation & 0x00ff;
	frame -= anim.frameBase; 

	int first = frame / rate2;
	int interpolation = frame % rate2;
	outFramePtr[0] += first;			 // Get frame pointers...
	outFramePtr[1] = outFramePtr[0] + 1; // and store away.

	if (interpolation == 0)
		return 0;

	// Clamp key frame to end if need be.
	int second = first * rate2 + rate2;
	if (second > anim.frameEnd)
		outRate = anim.frameEnd - (second - rate2);

	return interpolation;
}

int GetCurrentRelativeFrameNumber(ItemInfo* item)
{
	return item->Animation.FrameNumber - GetFrameNumber(item, 0);
}

int GetFrameNumber(ItemInfo* item, int frameToStart)
{
	int animNumber = item->Animation.AnimNumber - Objects[item->ObjectNumber].animIndex;
	return GetFrameNumber(item->ObjectNumber, animNumber, frameToStart);
}

int GetFrameNumber(int objectID, int animNumber, int frameToStart)
{
	return (g_Level.Anims[Objects[objectID].animIndex + animNumber].frameBase + frameToStart);
}

int GetFrameCount(int animNumber)
{
	if (animNumber < 0 || g_Level.Anims.size() <= animNumber)
		return 0;

	int end = g_Level.Anims[animNumber].frameEnd;
	int base = g_Level.Anims[animNumber].frameBase;
	return (end - base);
}

int GetNextAnimState(ItemInfo* item)
{
	return GetNextAnimState(item->ObjectNumber, item->Animation.AnimNumber);
}

int GetNextAnimState(int objectID, int animNumber)
{
	int nextAnimNumber = g_Level.Anims[Objects[objectID].animIndex + animNumber].JumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnimNumber].ActiveState;
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
		outPose.Orientation.y += rotation;
}

Vector3i GetJointPosition(ItemInfo* item, int jointIndex, const Vector3i& offset)
{
	// Get real item number.
	short itemNumber = item - g_Level.Items.data();

	// Use matrices done in the renderer to transform the offset vector.
	auto pos = offset.ToVector3();
	g_Renderer.GetItemAbsBonePosition(itemNumber, pos, jointIndex);
	return Vector3i(pos);
}

