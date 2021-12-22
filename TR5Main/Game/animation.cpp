#include "framework.h"
#include "animation.h"
#include "Lara.h"
#include "camera.h"
#include "level.h"
#include "setup.h"
#include "Renderer11.h"
#include "Sound/sound.h"
#include "flipeffect.h"
#include "items.h"
#include "collide.h"

using TEN::Renderer::g_Renderer;

BOUNDING_BOX InterpolatedBounds;

void AnimateItem(ITEM_INFO* item)
{
	item->touchBits = 0;
	item->hitStatus = false;

	item->frameNumber++;

	ANIM_STRUCT* anim = &g_Level.Anims[item->animNumber];
	if (anim->numberChanges > 0 && GetChange(item, anim))
	{
		anim = &g_Level.Anims[item->animNumber];

		item->currentAnimState = anim->currentAnimState;

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
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
					item->fallspeed = *(cmd++);
					item->speed = *(cmd++);
					item->gravityStatus = true;
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

		if (item->currentAnimState != anim->currentAnimState)
		{
			item->currentAnimState = anim->currentAnimState;
			item->goalAnimState = anim->currentAnimState;
		}

		if (item->requiredAnimState == item->currentAnimState)
			item->requiredAnimState = 0;
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
					else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
					{
						if (!flags || flags == (int)SOUND_PLAYCONDITION::Water && (g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER || Objects[item->objectNumber].intelligent))
						{
							SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
						}
					}
					else if (!flags || flags == (int)SOUND_PLAYCONDITION::Land && !(g_Level.Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER))
					{
						SoundEffect(cmd[1] & 0x3FFF, &item->pos, 2);
					}
				}
				else
				{
					if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
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

	if (item->gravityStatus)
	{
		item->fallspeed += (item->fallspeed >= 128 ? 1 : 6);
		item->pos.yPos += item->fallspeed;
	}
	else
	{
		int velocity = anim->velocity;
		if (anim->acceleration)
			velocity += anim->acceleration * (item->frameNumber - anim->frameBase);
		item->speed = velocity >> 16;

		lateral = anim->Xvelocity;
		if (anim->Xacceleration)
			lateral += anim->Xacceleration * (item->frameNumber - anim->frameBase);

		lateral >>= 16;
	}

	MoveItem(item, item->pos.yRot, item->speed, lateral);

	// Update matrices
	short itemNumber = item - g_Level.Items.data();
	g_Renderer.updateItemAnimations(itemNumber, true);
}

void TranslateItem(ITEM_INFO* item, int x, int y, int z)
{
	float c = phd_cos(item->pos.yRot);
	float s = phd_sin(item->pos.yRot);

	item->pos.xPos += c * x + s * z;
	item->pos.yPos += y;
	item->pos.zPos += -s * x + c * z;
}

int GetChange(ITEM_INFO* item, ANIM_STRUCT* anim)
{
	if (item->currentAnimState == item->goalAnimState)
		return 0;

	if (anim->numberChanges <= 0)
		return 0;

	for (int i = 0; i < anim->numberChanges; i++)
	{
		CHANGE_STRUCT* change = &g_Level.Changes[anim->changeIndex + i];
		if (change->goalAnimState == item->goalAnimState)
		{
			for (int j = 0; j < change->numberRanges; j++)
			{
				RANGE_STRUCT* range = &g_Level.Ranges[change->rangeIndex + j];
				if (item->frameNumber >= range->startFrame && item->frameNumber <= range->endFrame)
				{
					item->animNumber = range->linkAnimNum;
					item->frameNumber = range->linkFrameNum;

					return 1;
				}
			}
		}
	}

	return 0;
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
	ANIM_STRUCT *anim;
	int frm;
	int first, second;
	int interp, rat;

	frm = item->frameNumber;
	anim = &g_Level.Anims[item->animNumber];
	framePtr[0] = framePtr[1] = &g_Level.Frames[anim->framePtr];
	rat = *rate = anim->interpolation & 0x00ff;
	frm -= anim->frameBase; 
	first = frm / rat;
	interp = frm % rat;
	framePtr[0] += first;				  // Get Frame pointers
	framePtr[1] = framePtr[0] + 1;               // and store away
	if (interp == 0)
		return(0);
	second = first * rat + rat;
	if (second>anim->frameEnd)                       // Clamp KeyFrame to End if need be
		*rate = anim->frameEnd - (second - rat);
	return(interp);
}

short GetFrameNumber(ITEM_INFO* item, short frameToStart)
{
	return GetFrameNumber(item->objectNumber, item->animNumber, frameToStart);
}

short GetFrameNumber(short objectID, short animNumber, short frameToStart)
{
	return g_Level.Anims[Objects[objectID].animIndex + animNumber].frameBase + frameToStart;
}

int GetFrameCount(short animNumber)
{
	if (animNumber < 0 || g_Level.Anims.size() <= animNumber)
		return 0;

	return &g_Level.Anims[animNumber].frameEnd - &g_Level.Anims[animNumber].frameBase;
}

int GetNextAnimState(ITEM_INFO* item)
{
	return GetNextAnimState(item->objectNumber, item->animNumber);
}

int GetNextAnimState(short objectID, short animNumber)
{
	auto nextAnim = g_Level.Anims[Objects[objectID].animIndex + animNumber].jumpAnimNum;
	return g_Level.Anims[Objects[objectID].animIndex + nextAnim].currentAnimState;
}

void SetAnimation(ITEM_INFO* item, short animIndex, short frameToStart)
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
	item->currentAnimState = g_Level.Anims[index].currentAnimState;
	item->goalAnimState = item->currentAnimState;
}

bool TestLastFrame(ITEM_INFO* item, short animNumber)
{
	if (animNumber < 0)
		animNumber = item->animNumber;

	if (item->animNumber != animNumber)
		return false;

	ANIM_STRUCT* anim = &g_Level.Anims[animNumber];
	return (item->frameNumber >= anim->frameEnd);
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is NULL in OBJECT_INFO
}

void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum)
{
	if (LM_enum >= NUM_LARA_MESHES)
		LM_enum = LM_HEAD;

	Vector3 p = Vector3(pos->x, pos->y, pos->z);
	g_Renderer.getLaraAbsBonePosition(&p, LM_enum);

	pos->x = p.x;
	pos->y = p.y;
	pos->z = p.z;
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
	{
		pos->yRot += rot;
	}
}
