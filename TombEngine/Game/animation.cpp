#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
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

BOUNDING_BOX InterpolatedBounds;

void AnimateLara(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.FrameNumber++;

	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = anim->ActiveState;
	}

	if (item->Animation.FrameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, item->Pose.Orientation.y, cmd[2], cmd[1], cmd[0]);
					UpdateItemRoom(item, -LARA_HEIGHT / 2, -cmd[0], -cmd[2]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->Animation.Velocity.y = *(cmd++);
					item->Animation.Velocity.z = *(cmd++);
					item->Animation.IsAirborne = true;
					if (lara->Control.CalculatedJumpVelocity)
					{
						item->Animation.Velocity.y = lara->Control.CalculatedJumpVelocity;
						lara->Control.CalculatedJumpVelocity = 0;
					}

					break;

				case COMMAND_ATTACK_READY:
					if (lara->Control.HandStatus != HandStatus::Special)
						lara->Control.HandStatus = HandStatus::Free;

					break;

				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					cmd += 2;
					break;

				default:
					break;
				}
			}
		}

		item->Animation.AnimNumber = anim->jumpAnimNum;
		item->Animation.FrameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.ActiveState = anim->ActiveState;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];

		for (int i = anim->numberCommands; i > 0; i--)
		{
			switch (*(cmd++))
			{
			case COMMAND_MOVE_ORIGIN:
				cmd += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				cmd += 2;
				break;

			case COMMAND_SOUND_FX:
				if (item->Animation.FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}
				else
				{
					bool inWater = (cmd[1] & 0x8000) != 0;
					bool inDry   = (cmd[1] & 0x4000) != 0;
					bool always  = (inWater && inDry) || (!inWater && !inDry);

					SoundEnvironment condition = SoundEnvironment::Always;
					if (inWater) condition = SoundEnvironment::Water;
					if (inDry)   condition = SoundEnvironment::Land;
					if (always)  condition = SoundEnvironment::Always;

					if (condition == SoundEnvironment::Always ||
						(condition == SoundEnvironment::Land && (lara->WaterSurfaceDist >= -SHALLOW_WATER_START_LEVEL || lara->WaterSurfaceDist == NO_HEIGHT)) ||
						(condition == SoundEnvironment::Water && lara->WaterSurfaceDist <  -SHALLOW_WATER_START_LEVEL && lara->WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
					{
						SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
					}
				}

				cmd += 2;
				break;

			case COMMAND_EFFECT:
				if (item->Animation.FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				DoFlipEffect((cmd[1] & 0x3FFF), item);

				cmd += 2;
				break;

			default:
				break;

			}
		}
	}

	int frameCount = (anim->frameEnd - anim->frameBase) + 1;
	int currentFrame = item->Animation.FrameNumber - anim->frameBase;

	if (item->Animation.IsAirborne)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			item->Animation.Velocity.z -= item->Animation.Velocity.z / 8.0f;
			if (abs(item->Animation.Velocity.z) < 8.0f)
			{
				item->Animation.IsAirborne = false;
				item->Animation.Velocity.z = 0.0f;
			}

			if (item->Animation.Velocity.y > 128.0f)
				item->Animation.Velocity.y /= 2.0f;
			item->Animation.Velocity.y -= item->Animation.Velocity.y / 4.0f;

			if (item->Animation.Velocity.y < 4.0f)
				item->Animation.Velocity.y = 4.0f;
			item->Pose.Position.y += item->Animation.Velocity.y;
		}
		else
		{
			item->Animation.Velocity.z += (anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount;
			item->Animation.Velocity.y += item->Animation.Velocity.y >= 128.0f ? 1.0f : GRAVITY;
			item->Pose.Position.y += item->Animation.Velocity.y;
		}
	}
	else
	{
		if (lara->Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
			item->Animation.Velocity.z = (anim->VelocityStart.z / 2.0f) + ((((anim->VelocityEnd.z - anim->VelocityStart.z) / frameCount) * currentFrame) / 4.0f);
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

void AnimateItem(ItemInfo* item)
{
	item->TouchBits = NO_JOINT_BITS;
	item->HitStatus = false;

	item->Animation.FrameNumber++;

	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->Animation.AnimNumber];

		item->Animation.ActiveState = anim->ActiveState;
		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	if (item->Animation.FrameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, item->Pose.Orientation.y, cmd[2], cmd[1], cmd[0]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->Animation.Velocity.y = *(cmd++);
					item->Animation.Velocity.z = *(cmd++);
					item->Animation.IsAirborne = true;
					break;

				case COMMAND_DEACTIVATE:
					if (Objects[item->ObjectNumber].intelligent && !item->AfterDeath)
						item->AfterDeath = 1;

					item->Status = ITEM_DEACTIVATED;
					break;

				case COMMAND_SOUND_FX:
				case COMMAND_EFFECT:
					cmd += 2;
					break;

				default:
					break;
				}
			}
		}

		item->Animation.AnimNumber = anim->jumpAnimNum;
		item->Animation.FrameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->Animation.AnimNumber];
		if (item->Animation.ActiveState != anim->ActiveState)
		{
			item->Animation.ActiveState = anim->ActiveState;
			item->Animation.TargetState = anim->ActiveState;
		}

		if (item->Animation.RequiredState == item->Animation.ActiveState)
			item->Animation.RequiredState = 0;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];

		for (int i = anim->numberCommands; i > 0; i--)
		{
			switch (*(cmd++))
			{
			case COMMAND_MOVE_ORIGIN:
				cmd += 3;
				break;

			case COMMAND_JUMP_VELOCITY:
				cmd += 2;
				break;

			case COMMAND_SOUND_FX:
				if (item->Animation.FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				if (!Objects[item->ObjectNumber].waterCreature)
				{
					bool inWater = (cmd[1] & 0x8000) != 0;
					bool inDry   = (cmd[1] & 0x4000) != 0;
					bool always  = (inWater && inDry) || (!inWater && !inDry);

					if (item->RoomNumber == NO_ROOM)
					{
						item->Pose.Position.x = LaraItem->Pose.Position.x;
						item->Pose.Position.y = LaraItem->Pose.Position.y - 762;
						item->Pose.Position.z = LaraItem->Pose.Position.z;

						SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
					}
					else if (TestEnvironment(ENV_FLAG_WATER, item))
					{
						if (always || (inWater && TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber)))
							SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
					}
					else if (always || (inDry && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber) && !TestEnvironment(ENV_FLAG_SWAMP, Camera.pos.roomNumber)))
						SoundEffect(cmd[1] & 0x3FFF, &item->Pose, SoundEnvironment::Always);
				}
				else
				{
					SoundEffect(cmd[1] & 0x3FFF, &item->Pose, TestEnvironment(ENV_FLAG_WATER, item) ? SoundEnvironment::Water : SoundEnvironment::Land);
				}

				break;

			case COMMAND_EFFECT:
				if (item->Animation.FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				DoFlipEffect((cmd[1] & 0x3FFF), item);

				cmd += 2;
				break;

			default:
				break;
			}
		}
	}

	int frameCount = (anim->frameEnd - anim->frameBase) + 1;
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
	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

	if (anim->numberChanges <= 0)
		return false;

	if (targetState < 0)
		targetState = item->Animation.TargetState;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim->numberChanges; i++)
	{
		auto* dispatch = &g_Level.Changes[anim->changeIndex + i];
		if (dispatch->TargetState == targetState)
		{
			// Iterate over frame range of state dispatch.
			for (int j = 0; j < dispatch->numberRanges; j++)
			{
				auto* range = &g_Level.Ranges[dispatch->rangeIndex + j];
				if (item->Animation.FrameNumber >= range->startFrame && item->Animation.FrameNumber <= range->endFrame)
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

	auto* anim = &g_Level.Anims[animNumber];
	return (item->Animation.FrameNumber >= anim->frameEnd);
}

void TranslateItem(ItemInfo* item, short headingAngle, float forward, float down, float right)
{
	item->Pose.Position = Geometry::TranslatePoint(item->Pose.Position, headingAngle, forward, down, right);
}

void TranslateItem(ItemInfo* item, EulerAngles orient, float distance)
{
	item->Pose.Position = Geometry::TranslatePoint(item->Pose.Position, orient, distance);
}

void TranslateItem(ItemInfo* item, Vector3 direction, float distance)
{
	item->Pose.Position = Geometry::TranslatePoint(item->Pose.Position, direction, distance);
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

bool GetChange(ItemInfo* item, ANIM_STRUCT* anim)
{
	if (item->Animation.ActiveState == item->Animation.TargetState)
		return false;

	if (anim->numberChanges <= 0)
		return false;

	// Iterate over possible state dispatches.
	for (int i = 0; i < anim->numberChanges; i++)
	{
		auto* dispatch = &g_Level.Changes[anim->changeIndex + i];
		if (dispatch->TargetState == item->Animation.TargetState)
		{
			// Iterate over frame range of state dispatch.
			for (int j = 0; j < dispatch->numberRanges; j++)
			{
				auto* range = &g_Level.Ranges[dispatch->rangeIndex + j];
				if (item->Animation.FrameNumber >= range->startFrame && item->Animation.FrameNumber <= range->endFrame)
				{
					item->Animation.AnimNumber = range->linkAnimNum;
					item->Animation.FrameNumber = range->linkFrameNum;
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
	ANIM_FRAME* framePtr[2];
	
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

ANIM_FRAME* GetBestFrame(ItemInfo* item)
{
	int rate = 0;
	ANIM_FRAME* framePtr[2];

	int frac = GetFrame(item, framePtr, &rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
}

int GetFrame(ItemInfo* item, ANIM_FRAME* framePtr[], int* rate)
{
	int frame = item->Animation.FrameNumber;
	auto* anim = &g_Level.Anims[item->Animation.AnimNumber];
	framePtr[0] = framePtr[1] = &g_Level.Frames[anim->framePtr];
	int rate2 = *rate = anim->Interpolation & 0x00ff;
	frame -= anim->frameBase; 

	int first = frame / rate2;
	int interpolation = frame % rate2;
	framePtr[0] += first;			// Get frame pointers...
	framePtr[1] = framePtr[0] + 1;	// and store away.

	if (interpolation == 0)
		return 0;

	// Clamp key frame to end if need be.
	int second = first * rate2 + rate2;
	if (second > anim->frameEnd)
		*rate = anim->frameEnd - (second - rate2);

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
	int nextAnim = g_Level.Anims[Objects[objectID].animIndex + animNumber].jumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnim].ActiveState;
}

void DrawAnimatingItem(ItemInfo* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is nullptr in ObjectInfo
}

void ClampRotation(PoseData* pose, short angle, short rotation)
{
	if (angle <= rotation)
	{
		if (angle >= -rotation)
			pose->Orientation.y += angle;
		else
			pose->Orientation.y -= rotation;
	}
	else
		pose->Orientation.y += rotation;
}

Vector3i GetLaraJointPosition(int jointIndex, const Vector3i& offset)
{
	if (jointIndex >= NUM_LARA_MESHES)
		jointIndex = LM_HEAD;

	// Use matrices done in the renderer to transform the offset vector.
	auto pos = offset.ToVector3();
	g_Renderer.GetLaraAbsBonePosition(pos, jointIndex);
	return Vector3i(pos);
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

// TODO: Adopt above version and remove this one.
void GetJointPosition(ItemInfo* item, Vector3i* offset, int jointIndex)
{
	short itemNumber = item - g_Level.Items.data();
	auto pos = offset->ToVector3();
	g_Renderer.GetItemAbsBonePosition(itemNumber, pos, jointIndex);
	*offset = Vector3i(pos);
}
