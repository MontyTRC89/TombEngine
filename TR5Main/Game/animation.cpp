#include "framework.h"
#include "Game/animation.h"

#include "Game/camera.h"
#include "Game/control/flipeffect.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"

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
	LaraInfo*& info = item->data;

	item->frameNumber++;

	auto* anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];
		item->activeState = anim->activeState;
	}

	if (item->frameNumber > anim->frameEnd)
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
					if (info->calcJumpVelocity)
					{
						item->VerticalVelocity = info->calcJumpVelocity;
						info->calcJumpVelocity = 0;
					}
					break;

				case COMMAND_ATTACK_READY:
					if (info->gunStatus != LG_SPECIAL)
						info->gunStatus = LG_HANDS_FREE;
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

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->animNumber];
		item->activeState = anim->activeState;
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
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;
				if (flags == (int)SOUND_PLAYCONDITION::LandAndWater ||
					(flags == (int)SOUND_PLAYCONDITION::Land && (info->waterSurfaceDist >= 0 || info->waterSurfaceDist == NO_HEIGHT)) ||
					(flags == (int)SOUND_PLAYCONDITION::Water && info->waterSurfaceDist < 0 && info->waterSurfaceDist != NO_HEIGHT && !TestEnvironment(ENV_FLAG_SWAMP, item)))
				{
					SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
				}

				cmd += 2;
				break;

			case COMMAND_EFFECT:
				if (item->frameNumber != *cmd)
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
		lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);
	lateral >>= 16;

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
			item->pos.yPos += item->VerticalVelocity;
		}
		else
		{
			int velocity = (anim->velocity + anim->acceleration * (item->frameNumber - anim->frameBase - 1));
			item->Velocity -= velocity >> 16;
			item->Velocity += (velocity + anim->acceleration) >> 16;
			item->VerticalVelocity += (item->VerticalVelocity >= 128 ? 1 : GRAVITY);
			item->pos.yPos += item->VerticalVelocity;
		}
	}
	else
	{
		int velocity;

		if (info->waterStatus == LW_WADE && TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			velocity = (anim->velocity >> 1);
			if (anim->acceleration)
				velocity += (anim->acceleration * (item->frameNumber - anim->frameBase)) >> 2;
		}
		else
		{
			velocity = anim->velocity;
			if (anim->acceleration)
				velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		}

		item->Velocity = velocity >> 16;
	}

	if (info->ropeParameters.Ptr != -1)
		DelAlignLaraToRope(item);

	if (!info->isMoving)
		MoveItem(item, info->moveAngle, item->Velocity, lateral);

	// Update matrices
	g_Renderer.updateLaraAnimations(true);
}

void AnimateItem(ITEM_INFO* item)
{
	item->touchBits = 0;
	item->hitStatus = false;

	item->frameNumber++;

	auto* anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];

		item->activeState = anim->activeState;
		if (item->requiredState == item->activeState)
			item->requiredState = 0;
	}

	if (item->frameNumber > anim->frameEnd)
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
					if (Objects[item->objectNumber].intelligent && !item->afterDeath)
						item->afterDeath = 1;
					item->status = ITEM_DEACTIVATED;
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

		item->animNumber = anim->jumpAnimNum;
		item->frameNumber = anim->jumpFrameNum;

		anim = &g_Level.Anims[item->animNumber];
		if (item->activeState != anim->activeState)
		{
			item->activeState = anim->activeState;
			item->targetState = anim->activeState;
		}

		if (item->requiredState == item->activeState)
			item->requiredState = 0;
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
				if (item->frameNumber != *cmd)
				{
					cmd += 2;
					break;
				}

				flags = cmd[1] & 0xC000;

				if (!Objects[item->objectNumber].waterCreature)
				{
					if (item->roomNumber == NO_ROOM)
					{
						item->pos.xPos = LaraItem->pos.xPos;
						item->pos.yPos = LaraItem->pos.yPos - 762;
						item->pos.zPos = LaraItem->pos.zPos;

						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
					else if (TestEnvironment(ENV_FLAG_WATER, item))
					{
						if (!flags || flags == (int)SOUND_PLAYCONDITION::Water && (TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber)  || Objects[item->objectNumber].intelligent))
							SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
					else if (!flags || flags == (int)SOUND_PLAYCONDITION::Land && !TestEnvironment(ENV_FLAG_WATER, Camera.pos.roomNumber))
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
				}
				else
				{
					if (TestEnvironment(ENV_FLAG_WATER, item))
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 1);
					else
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 0);
				}

				break;

			case COMMAND_EFFECT:
				if (item->frameNumber != *cmd)
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
		item->pos.yPos += item->VerticalVelocity;
	}
	else
	{
		int velocity = anim->velocity;
		if (anim->acceleration)
			velocity += anim->acceleration * (item->frameNumber - anim->frameBase);

		item->Velocity = velocity >> 16;

		lateral = anim->Xvelocity;
		if (anim->Xacceleration)
			lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);

		lateral >>= 16;
	}

	MoveItem(item, item->pos.yRot, item->Velocity, lateral);

	// Update matrices.
	short itemNumber = item - g_Level.Items.data();
	g_Renderer.updateItemAnimations(itemNumber, true);
}

void TranslateItem(ITEM_INFO* item, int x, int y, int z)
{
	float c = phd_cos(item->pos.yRot);
	float s = phd_sin(item->pos.yRot);

	item->pos.xPos += roundf(c * x + s * z);
	item->pos.yPos += y;
	item->pos.zPos += roundf(-s * x + c * z);
}

bool GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	if (item->activeState == item->targetState)
		return false;

	if (anim->numberChanges <= 0)
		return false;

	for (int i = 0; i < anim->numberChanges; i++)
	{
		auto* change = &g_Level.Changes[anim->changeIndex + i];
		if (change->targetState == item->targetState)
		{
			for (int j = 0; j < change->numberRanges; j++)
			{
				auto* range = &g_Level.Ranges[change->rangeIndex + j];
				if (item->frameNumber >= range->startFrame && item->frameNumber <= range->endFrame)
				{
					item->animNumber = range->linkAnimNum;
					item->frameNumber = range->linkFrameNum;
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
	int frame = item->frameNumber;
	auto* anim = &g_Level.Anims[item->animNumber];
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
	return item->frameNumber - GetFrameNumber(item, 0);
}

int GetFrameNumber(ITEM_INFO* item, int frameToStart)
{
	return GetFrameNumber(item->objectNumber, item->animNumber, frameToStart);
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
	return GetNextAnimState(item->objectNumber, item->animNumber);
}

int GetNextAnimState(int objectID, int animNumber)
{
	auto nextAnim = g_Level.Anims[Objects[objectID].animIndex + animNumber].jumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnim].activeState;
}

void SetAnimation(ITEM_INFO* item, int animIndex, int frameToStart)
{
	auto index = Objects[item->objectNumber].animIndex + animIndex;

	if (index < 0 || index >= g_Level.Anims.size())
	{
		TENLog(std::string("Attempt to set nonexistent animation ") + std::to_string(animIndex) + std::string(" for object ") + std::to_string(item->objectNumber), LogLevel::Warning);
		return;
	}

	if (item->animNumber == animIndex)
		return;

	item->animNumber = index;
	item->frameNumber = g_Level.Anims[index].frameBase + frameToStart;
	item->activeState = g_Level.Anims[index].activeState;
	item->targetState = item->activeState;
}

bool TestLastFrame(ITEM_INFO* item, int animNumber)
{
	if (animNumber < 0)
		animNumber = item->animNumber;

	if (item->animNumber != animNumber)
		return false;

	auto* anim = &g_Level.Anims[animNumber];
	return (item->frameNumber >= anim->frameEnd);
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
