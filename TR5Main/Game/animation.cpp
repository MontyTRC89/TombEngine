#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/control/flipeffect.h"
#include "Game/collision/collide_room.h"
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

void AnimateLara(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	item->FrameNumber++;

	auto* anim = &g_Level.Anims[item->AnimNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->AnimNumber];
		item->ActiveState = anim->ActiveState;
	}

	if (item->FrameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					UpdateItemRoom(item, -LARA_HEIGHT / 2, -cmd[0], -cmd[2]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->VerticalVelocity = *(cmd++);
					item->Velocity = *(cmd++);
					item->Airborne = true;
					if (lara->Control.CalculatedJumpVelocity)
					{
						item->VerticalVelocity = lara->Control.CalculatedJumpVelocity;
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

		item->AnimNumber = anim->jumpAnimNum;
		item->FrameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->AnimNumber];
		item->ActiveState = anim->ActiveState;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];
		int flags;
		int effectID = 0;

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
				if (item->FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;
				if (flags == (int)SOUND_PLAYCONDITION::LandAndWater ||
					(flags == (int)SOUND_PLAYCONDITION::Land && (lara->WaterSurfaceDist >= 0 || lara->WaterSurfaceDist == NO_HEIGHT)) ||
					(flags == (int)SOUND_PLAYCONDITION::Water && lara->WaterSurfaceDist < 0 && lara->WaterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
				{
					SoundEffect(cmd[1] & 0x3FFF, &item->Position, 2);
				}

				cmd += 2;
				break;

			case COMMAND_EFFECT:
				if (item->FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				effectID = cmd[1] & 0x3FFF;
				DoFlipEffect(effectID, item);

				cmd += 2;
				break;

			default:
				break;

			}
		}
	}

	int lateral = anim->Xvelocity;
	if (anim->Xacceleration)
		lateral += anim->Xacceleration * (item->FrameNumber - anim->frameBase);
	item->LateralVelocity = lateral >>= 16;

	if (item->Airborne)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			item->Velocity -= item->Velocity >> 3;
			if (abs(item->Velocity) < 8)
			{
				item->Velocity = 0;
				item->Airborne = false;
			}
			if (item->VerticalVelocity > 128)
				item->VerticalVelocity /= 2;
			item->VerticalVelocity -= item->VerticalVelocity / 4;
			if (item->VerticalVelocity < 4)
				item->VerticalVelocity = 4;
			item->Position.yPos += item->VerticalVelocity;
		}
		else
		{
			int velocity = (anim->velocity + anim->acceleration * (item->FrameNumber - anim->frameBase - 1));
			item->Velocity -= velocity >> 16;
			item->Velocity += (velocity + anim->acceleration) >> 16;
			item->VerticalVelocity += (item->VerticalVelocity >= 128 ? 1 : GRAVITY);
			item->Position.yPos += item->VerticalVelocity;
		}
	}
	else
	{
		int velocity;

		if (lara->Control.WaterStatus == WaterStatus::Wade && TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			velocity = (anim->velocity >> 1);
			if (anim->acceleration)
				velocity += (anim->acceleration * (item->FrameNumber - anim->frameBase)) >> 2;
		}
		else
		{
			velocity = anim->velocity;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->FrameNumber - anim->frameBase);
		}

		item->Velocity = velocity >> 16;
	}

	// TEMP
	item->Velocity += lara->ExtraVelocity.x;
	item->VerticalVelocity += lara->ExtraVelocity.y;
	item->LateralVelocity += lara->ExtraVelocity.z;

	if (lara->Control.RopeControl.Ptr != -1)
		DelAlignLaraToRope(item);

	if (!lara->Control.IsMoving)
		MoveItem(item, lara->Control.MoveAngle, item->Velocity, item->LateralVelocity);

	// TEMP
	lara->ExtraVelocity = PHD_VECTOR();

	// Update matrices
	g_Renderer.updateLaraAnimations(true);
}

void AnimateItem(ITEM_INFO* item)
{
	item->TouchBits = 0;
	item->HitStatus = false;

	item->FrameNumber++;

	auto* anim = &g_Level.Anims[item->AnimNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->AnimNumber];

		item->ActiveState = anim->ActiveState;
		if (item->RequiredState == item->ActiveState)
			item->RequiredState = 0;
	}

	if (item->FrameNumber > anim->frameEnd)
	{
		if (anim->numberCommands > 0)
		{
			short* cmd = &g_Level.Commands[anim->commandIndex];
			for (int i = anim->numberCommands; i > 0; i--)
			{
				switch (*(cmd++))
				{
				case COMMAND_MOVE_ORIGIN:
					TranslateItem(item, cmd[0], cmd[1], cmd[2]);
					cmd += 3;
					break;

				case COMMAND_JUMP_VELOCITY:
					item->VerticalVelocity = *(cmd++);
					item->Velocity = *(cmd++);
					item->Airborne = true;
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

		item->AnimNumber = anim->jumpAnimNum;
		item->FrameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->AnimNumber];
		if (item->ActiveState != anim->ActiveState)
		{
			item->ActiveState = anim->ActiveState;
			item->TargetState = anim->ActiveState;
		}

		if (item->RequiredState == item->ActiveState)
			item->RequiredState = 0;
	}

	if (anim->numberCommands > 0)
	{
		short* cmd = &g_Level.Commands[anim->commandIndex];
		int flags;
		int effectID = 0;

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
				if (item->FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;

				if (!Objects[item->ObjectNumber].waterCreature)
				{
					if (item->RoomNumber == NO_ROOM)
					{
						item->Position.xPos = LaraItem->Position.xPos;
						item->Position.yPos = LaraItem->Position.yPos - 762;
						item->Position.zPos = LaraItem->Position.zPos;

						SoundEffect(cmd[1] & 0x3FFF, &item->Position, 2);
					}
					else if (TestEnvironment(ENV_FLAG_WATER, item))
					{
						if (!flags || flags == (int)SOUND_PLAYCONDITION::Water && (TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber)  || Objects[item->ObjectNumber].intelligent))
							SoundEffect(cmd[1] & 0x3FFF, &item->Position, 2);
					}
					else if (!flags || flags == (int)SOUND_PLAYCONDITION::Land && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber))
						SoundEffect(cmd[1] & 0x3FFF, &item->Position, 2);
				}
				else
				{
					if (TestEnvironment(ENV_FLAG_WATER, item))
						SoundEffect(cmd[1] & 0x3FFF, &item->Position, 1);
					else
						SoundEffect(cmd[1] & 0x3FFF, &item->Position, 0);
				}

				break;

			case COMMAND_EFFECT:
				if (item->FrameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				effectID = cmd[1] & 0x3FFF;
				DoFlipEffect(effectID, item);

				cmd += 2;
				break;

			default:
				break;
			}
		}
	}

	int lateral = 0;

	if (item->Airborne)
	{
		item->VerticalVelocity += (item->VerticalVelocity >= 128 ? 1 : 6);
		item->Position.yPos += item->VerticalVelocity;
	}
	else
	{
		int velocity = anim->velocity;
		if (anim->acceleration)
			velocity += anim->acceleration * (item->FrameNumber - anim->frameBase);

		item->Velocity = velocity >> 16;

		lateral = anim->Xvelocity;
		if (anim->Xacceleration)
			lateral += anim->Xacceleration * (item->FrameNumber - anim->frameBase);

		lateral >>= 16;
	}

	MoveItem(item, item->Position.yRot, item->Velocity, lateral);

	// Update matrices.
	short itemNumber = item - g_Level.Items.data();
	g_Renderer.updateItemAnimations(itemNumber, true);
}

void TranslateItem(ITEM_INFO* item, int x, int y, int z)
{
	float c = phd_cos(item->Position.yRot);
	float s = phd_sin(item->Position.yRot);

	item->Position.xPos += roundf(c * x + s * z);
	item->Position.yPos += y;
	item->Position.zPos += roundf(-s * x + c * z);
}

bool GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	if (item->ActiveState == item->TargetState)
		return false;

	if (anim->numberChanges <= 0)
		return false;

	for (int i = 0; i < anim->numberChanges; i++)
	{
		auto* change = &g_Level.Changes[anim->changeIndex + i];
		if (change->TargetState == item->TargetState)
		{
			for (int j = 0; j < change->numberRanges; j++)
			{
				auto* range = &g_Level.Ranges[change->rangeIndex + j];
				if (item->FrameNumber >= range->startFrame && item->FrameNumber <= range->endFrame)
				{
					item->AnimNumber = range->linkAnimNum;
					item->FrameNumber = range->linkFrameNum;
					return true;
				}
			}
		}
	}

	return false;
}

BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item)
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

ANIM_FRAME* GetBestFrame(ITEM_INFO* item)
{
	int rate = 0;
	ANIM_FRAME* framePtr[2];

	int frac = GetFrame(item, framePtr, &rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
}

int GetFrame(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate)
{
	int frame = item->FrameNumber;
	auto* anim = &g_Level.Anims[item->AnimNumber];
	framePtr[0] = framePtr[1] = &g_Level.Frames[anim->framePtr];
	int rate2 = *rate = anim->interpolation & 0x00ff;
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

int GetCurrentRelativeFrameNumber(ITEM_INFO* item)
{
	return item->FrameNumber - GetFrameNumber(item, 0);
}

int GetFrameNumber(ITEM_INFO* item, int frameToStart)
{
	return GetFrameNumber(item->ObjectNumber, item->AnimNumber, frameToStart);
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

int GetNextAnimState(ITEM_INFO* item)
{
	return GetNextAnimState(item->ObjectNumber, item->AnimNumber);
}

int GetNextAnimState(int objectID, int animNumber)
{
	auto nextAnim = g_Level.Anims[Objects[objectID].animIndex + animNumber].jumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnim].ActiveState;
}

void SetAnimation(ITEM_INFO* item, int animIndex, int frameToStart)
{
	auto index = Objects[item->ObjectNumber].animIndex + animIndex;

	if (index < 0 || index >= g_Level.Anims.size())
	{
		TENLog(std::string("Attempt to set nonexistent animation ") + std::to_string(animIndex) + std::string(" for object ") + std::to_string(item->ObjectNumber), LogLevel::Warning);
		return;
	}

	if (item->AnimNumber == animIndex)
		return;

	item->AnimNumber = index;
	item->FrameNumber = g_Level.Anims[index].frameBase + frameToStart;
	item->ActiveState = g_Level.Anims[index].ActiveState;
	item->TargetState = item->ActiveState;
}

bool TestLastFrame(ITEM_INFO* item, int animNumber)
{
	if (animNumber < 0)
		animNumber = item->AnimNumber;

	if (item->AnimNumber != animNumber)
		return false;

	auto* anim = &g_Level.Anims[animNumber];
	return (item->FrameNumber >= anim->frameEnd);
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is NULL in OBJECT_INFO
}

void GetLaraJointPosition(PHD_VECTOR* pos, int laraMeshIndex)
{
	if (laraMeshIndex >= NUM_LARA_MESHES)
		laraMeshIndex = LM_HEAD;

	auto pos2 = Vector3(pos->x, pos->y, pos->z);
	g_Renderer.getLaraAbsBonePosition(&pos2, laraMeshIndex);

	pos->x = pos2.x;
	pos->y = pos2.y;
	pos->z = pos2.z;
}

void ClampRotation(PHD_3DPOS* pos, short angle, short rot)
{
	if (angle <= rot)
	{
		if (angle >= -rot)
			pos->yRot += angle;
		else
			pos->yRot -= rot;
	}
	else
		pos->yRot += rot;
}
