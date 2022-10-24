#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/flipeffect.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/rope.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Generic;
using TEN::Renderer::g_Renderer;

BOUNDING_BOX InterpolatedBounds;

void AnimateLara(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	PerformAnimCommands(item, true);

	item->Animation.FrameNumber++;

	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

	if (anim->NumStateDispatches > 0 && GetStateDispatch(item, *anim))
	{
		anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = anim->ActiveState;
	}

	if (item->Animation.FrameNumber > anim->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = anim->JumpAnimNum;
		item->Animation.FrameNumber = anim->JumpFrameNum;

		anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = anim->ActiveState;
	}

	int frameCount = anim->frameEnd - anim->frameBase;
	frameCount = (frameCount > 0) ? frameCount : 1;
	int currentFrame = item->Animation.FrameNumber - anim->frameBase;

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
			item->Animation.Velocity.z += (anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount;
			item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : GRAVITY;
			item->Pose.Position.y += item->Animation.Velocity.y;
		}
	}
	else
	{
		if (lara->Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
			item->Animation.Velocity.z = (anim->VelocityStart.z / 2) + ((((anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount) * currentFrame) / 4);
		else
			item->Animation.Velocity.z = anim->VelocityStart.z + (((anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount) * currentFrame);
	}

	item->Animation.Velocity.x = anim->VelocityStart.x + (((anim->VelocityEnd.x - anim->VelocityStart.x) / frameCount) * currentFrame);

	if (lara->Control.Rope.Ptr != -1)
		DelAlignLaraToRope(item);

	if (!lara->Control.IsMoving)
		TranslateItem(item, lara->Control.MoveAngle, item->Animation.Velocity.z, 0.0f, item->Animation.Velocity.x);

	// Update matrices
	g_Renderer.UpdateLaraAnimations(true);
}

void PerformAnimCommands(ItemInfo* item, bool isFrameBased)
{
	const auto& anim = g_Level.Anims[item->Animation.AnimNumber];

	if (anim.NumCommands == 0)
		return;

	short* cmd = &g_Level.Commands[anim.CommandIndex];

	for (int i = anim.NumCommands; i > 0; i--)
	{
		auto animCommand = (AnimCommandType)cmd[0];
		cmd++;

		switch (animCommand)
		{
		case AnimCommandType::MoveOrigin:
			if (!isFrameBased)
			{
				TranslateItem(item, item->Pose.Orientation.y, cmd[2], cmd[1], cmd[0]);
				auto* bounds = GetBoundsAccurate(item);
				UpdateItemRoom(item, -bounds->Height() / 2, -cmd[0], -cmd[2]);
			}

			cmd += 3;
			break;

		case AnimCommandType::JumpVelocity:
			if (!isFrameBased)
			{
				item->Animation.Velocity.y = cmd[0];
				item->Animation.Velocity.z = cmd[1];
				item->Animation.IsAirborne = true;

				if (item->IsLara())
				{
					auto* lara = GetLaraInfo(item);

					if (lara->Control.CalculatedJumpVelocity)
					{
						item->Animation.Velocity.y = lara->Control.CalculatedJumpVelocity;
						lara->Control.CalculatedJumpVelocity = 0;
					}
				}
			}

			cmd += 2;
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
				auto* lara = GetLaraInfo(item);
				if (lara->Control.HandStatus != HandStatus::Special)
					lara->Control.HandStatus = HandStatus::Free;
			}

			break;

		case AnimCommandType::SoundEffect:
			if (isFrameBased && item->Animation.FrameNumber == cmd[0])
			{
				if (!Objects[item->ObjectNumber].waterCreature)
				{
					bool inWater = (cmd[1] & 0x8000) != 0;
					bool onDry   = (cmd[1] & 0x4000) != 0;
					bool always  = (inWater && onDry) || (!inWater && !onDry);

					if (item->IsLara())
					{
						auto* lara = GetLaraInfo(item);

						if (always ||
						   (onDry && (lara->WaterSurfaceDist >= -SHALLOW_WATER_START_LEVEL || lara->WaterSurfaceDist == NO_HEIGHT)) ||
						   (inWater && lara->WaterSurfaceDist < -SHALLOW_WATER_START_LEVEL && lara->WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
						{
							SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
					}
					else
					{
						if (item->RoomNumber == NO_ROOM)
							SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						else if (TestEnvironment(ENV_FLAG_WATER, item))
						{
							if (always || (inWater && TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber)))
								SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
						}
						else if (always || (onDry && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber) && !TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.roomNumber)))
							SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
					}
				}
				else
					SoundEffect(cmd[1] & 0x3FFF, &item->Pose, TestEnvironment(ENV_FLAG_WATER, item) ? SoundEnvironment::Water : SoundEnvironment::Land);
			}

			cmd += 2;
			break;

		case AnimCommandType::Flipeffect:
			if (isFrameBased && item->Animation.FrameNumber == cmd[0])
				DoFlipEffect((cmd[1] & 0x3FFF), item);

			cmd += 2;
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

	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

	if (anim->NumStateDispatches > 0 && GetStateDispatch(item, *anim))
	{
		anim = &g_Level.Anims[item->Animation.AnimNumber];

		item->Animation.ActiveState = anim->ActiveState;
		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	if (item->Animation.FrameNumber > anim->frameEnd)
	{
		PerformAnimCommands(item, false);

		item->Animation.AnimNumber = anim->JumpAnimNum;
		item->Animation.FrameNumber = anim->JumpFrameNum;

		anim = &g_Level.Anims[item->Animation.AnimNumber];
		if (item->Animation.ActiveState != anim->ActiveState)
		{
			item->Animation.ActiveState = anim->ActiveState;
			item->Animation.TargetState = anim->ActiveState;
		}

		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	int frameCount = anim->frameEnd - anim->frameBase;
	frameCount = (frameCount > 0) ? frameCount : 1;
	int currentFrame = item->Animation.FrameNumber - anim->frameBase;

	if (item->Animation.IsAirborne)
	{
		item->Animation.Velocity.y += (item->Animation.Velocity.y >= 128.0f) ? 1.0f : 6.0f;
		item->Pose.Position.y += item->Animation.Velocity.y;
	}
	else
	{
		item->Animation.Velocity.z = anim->VelocityStart.z + (((anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount) * currentFrame);
		item->Animation.Velocity.x = anim->VelocityStart.x + (((anim->VelocityEnd.x - anim->VelocityStart.x) / frameCount) * currentFrame);
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

	if (targetState < 0)
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
	if (animNumber < 0)
		animNumber = item->Animation.AnimNumber;

	if (item->Animation.AnimNumber != animNumber)
		return false;

	const auto& anim = g_Level.Anims[animNumber];

	return (item->Animation.FrameNumber >= anim.frameEnd);
}

void TranslateItem(ItemInfo* item, float angle, float forward, float down, float right)
{
	item->Pose.Position = TranslatePoint(item->Pose.Position, angle, forward, down, right);
}

void TranslateItem(ItemInfo* item, EulerAngles orient, float distance)
{
	item->Pose.Position = TranslatePoint(item->Pose.Position, orient, distance);
}

void TranslateItem(ItemInfo* item, Vector3 direction, float distance)
{
	item->Pose.Position = TranslatePoint(item->Pose.Position, direction, distance);
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

BOUNDING_BOX* GetBoundsAccurate(ItemInfo* item)
{
	int rate = 0;
	AnimFrame* framePtr[2];
	
	int frac = GetFrame(item, framePtr, &rate);
	if (frac == 0)
		return &framePtr[0]->boundingBox;
	else
	{
		InterpolatedBounds.X1 = framePtr[0]->boundingBox.X1 + (framePtr[1]->boundingBox.X1 - framePtr[0]->boundingBox.X1) * frac / rate;
		InterpolatedBounds.X2 = framePtr[0]->boundingBox.X2 + (framePtr[1]->boundingBox.X2 - framePtr[0]->boundingBox.X2) * frac / rate;
		InterpolatedBounds.Y1 = framePtr[0]->boundingBox.Y1 + (framePtr[1]->boundingBox.Y1 - framePtr[0]->boundingBox.Y1) * frac / rate;
		InterpolatedBounds.Y2 = framePtr[0]->boundingBox.Y2 + (framePtr[1]->boundingBox.Y2 - framePtr[0]->boundingBox.Y2) * frac / rate;
		InterpolatedBounds.Z1 = framePtr[0]->boundingBox.Z1 + (framePtr[1]->boundingBox.Z1 - framePtr[0]->boundingBox.Z1) * frac / rate;
		InterpolatedBounds.Z2 = framePtr[0]->boundingBox.Z2 + (framePtr[1]->boundingBox.Z2 - framePtr[0]->boundingBox.Z2) * frac / rate;
		return &InterpolatedBounds;
	}
}

AnimFrame* GetBestFrame(ItemInfo* item)
{
	int rate = 0;
	AnimFrame* framePtr[2];

	int frac = GetFrame(item, framePtr, &rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
}

int GetFrame(ItemInfo* item, AnimFrame* framePtr[], int* rate)
{
	int frame = item->Animation.FrameNumber;
	const auto& anim = g_Level.Anims[item->Animation.AnimNumber];

	framePtr[0] = framePtr[1] = &g_Level.Frames[anim.FramePtr];
	int rate2 = *rate = anim.Interpolation & 0x00ff;
	frame -= anim.frameBase; 

	int first = frame / rate2;
	int interpolation = frame % rate2;
	framePtr[0] += first;			// Get frame pointers...
	framePtr[1] = framePtr[0] + 1;	// and store away.

	if (interpolation == 0)
		return 0;

	// Clamp key frame to end if need be.
	int second = first * rate2 + rate2;
	if (second > anim.frameEnd)
		*rate = anim.frameEnd - (second - rate2);

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

	int end  = g_Level.Anims[animNumber].frameEnd;
	int base = g_Level.Anims[animNumber].frameBase;
	return (end - base);
}

int GetNextAnimState(ItemInfo* item)
{
	return GetNextAnimState(item->ObjectNumber, item->Animation.AnimNumber);
}

int GetNextAnimState(int objectID, int animNumber)
{
	int nextAnim = g_Level.Anims[Objects[objectID].animIndex + animNumber].JumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnim].ActiveState;
}

void DrawAnimatingItem(ItemInfo* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is NULL in ObjectInfo
}

void GetLaraJointPosition(Vector3Int* pos, int laraMeshIndex)
{
	if (laraMeshIndex >= NUM_LARA_MESHES)
		laraMeshIndex = LM_HEAD;

	Vector3 p = Vector3(pos->x, pos->y, pos->z);
	g_Renderer.GetLaraAbsBonePosition(&p, laraMeshIndex);

	pos->x = p.x;
	pos->y = p.y;
	pos->z = p.z;
}

void ClampRotation(PHD_3DPOS* pose, float angle, float rotation)
{
	if (angle <= rotation)
	{
		if (angle >= -rotation)
			pose->Orientation.y += angle;
		else
			pose->Orientation.y -= angle;
	}
	else
		pose->Orientation.y -= angle;
}
