#include "../newobjects.h"
#include "../oldobjects.h"
#include "../../Game/lara.h"
#include "../../Game/draw.h"
#include "../../Global/global.h"
#include "../../Game/items.h"
#include "../../Game/collide.h"
#include "../../Game/effects.h"
#include "../../Game/laramisc.h"
#include "../../Game/Box.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/switch.h"
#include "../../Game/spotcam.h"
#include "../../Game/effect2.h"
#include "../../Game/sphere.h"
#include "../../Game/traps.h"

short SPyoffs[8] =
{
	0xFC00, 0x0000, 0xFE00, 0x0000, 0x0000, 0x0000, 0xFE00, 0x0000
};

short SPxzoffs[8] =
{
	0x0000, 0x0000, 0x0200, 0x0000, 0x0000, 0x0000, 0xFE00, 0x0000
};

short SPDETyoffs[8] =
{
	0x0400, 0x0200, 0x0200, 0x0200, 0x0000, 0x0200, 0x0200, 0x0200
};

void InitialiseTeethSpikes(short itemNumber)
{
	short rotations[] = { -ANGLE(180), -ANGLE(135), -ANGLE(90), -ANGLE(45), ANGLE(0), ANGLE(45), ANGLE(90), ANGLE(135) };

	ITEM_INFO* item = &Items[itemNumber];

	item->status = ITEM_INVISIBLE;

	int angle;
	if (item->triggerFlags & 8)
	{
		angle = item->triggerFlags & 7;
		item->pos.xRot = rotations[angle];
		item->pos.yRot = ANGLE(90);
		item->pos.zPos -= SPxzoffs[angle];
	}
	else
	{
		angle = item->triggerFlags & 7;
		item->pos.zRot = rotations[angle];
		item->pos.xPos += SPxzoffs[angle];
	}

	item->itemFlags[0] = 1024;
	item->itemFlags[2] = 0;
	item->pos.yPos += SPyoffs[angle];
}

int CollidedWithTeethSpikes(ITEM_INFO* item)
{
	short angle;
	int x;
	int z;

	if (item->triggerFlags & 8)
	{
		angle = item->triggerFlags & 7;
		x = item->pos.xPos & 0xFFFFFE00 | 0x200;
		z = (item->pos.zPos + SPxzoffs[angle]) & 0xFFFFFE00 | 0x200;
	}
	else
	{
		angle = item->triggerFlags & 7;
		x = (item->pos.xPos - SPxzoffs[angle]) & 0xFFFFFE00 | 0x200;
		z = item->pos.zPos & 0xFFFFFE00 | 0x200;
	}

	int delta = -((angle & 1) != 0);
	delta = delta & 0xFF4C;
	delta += 480;
	int y = item->pos.yPos + SPDETyoffs[angle];
	short* frames = GetBestFrame(LaraItem);

	if (LaraItem->pos.yPos + frames[2] <= y && LaraItem->pos.yPos + frames[3] >= y - 900)
	{
		if (LaraItem->pos.xPos + frames[0] <= (x + delta) && LaraItem->pos.xPos + frames[1] >= (x - delta))
		{
			if (LaraItem->pos.zPos + frames[4] <= (z + delta) && LaraItem->pos.zPos + frames[5] >= (z - delta))
				return 1;
		}
	}

	return 0;
}

void ControlTeethSpikes(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item) || item->itemFlags[2])
	{
		if (TriggerActive(item))
		{
			item->itemFlags[1] -= item->itemFlags[0];
			item->itemFlags[0] += (item->itemFlags[0] >> 3) + 32;
			
			if (item->itemFlags[1] < 0)
			{
				item->itemFlags[0] = 1024;
				item->itemFlags[1] = 0;
				item->status = ITEM_INVISIBLE;
			}

			if (item->triggerFlags & 0x20)
			{
				item->itemFlags[2] = 1;
			}
			else
			{
				if (item->itemFlags[2])
				{
					item->itemFlags[2]--;
				}
			}
		}
		else if (!item->timer)
		{
			item->itemFlags[0] += (item->itemFlags[0] >> 3) + 32;

			if (item->itemFlags[1] > 0)
			{
				item->itemFlags[1] -= item->itemFlags[0];
				if (item->itemFlags[1] < 0)
					item->itemFlags[1] = 0;
			}
		}
	}
	else
	{
		if (item->itemFlags[0] == 1024)
			SoundEffect(SFX_TEETH_SPIKES, &item->pos, 0);

		item->status = ITEM_ACTIVE;

		if (LaraItem->hitPoints > 0 && CollidedWithTeethSpikes(item))
		{
			short* itemFrames = GetBestFrame(item);
			short* laraFrames = GetBestFrame(LaraItem);
			
			short angle = item->triggerFlags & 7;
			int numBloods = 0;

			if ((item->itemFlags[0] > 1024 || LaraItem->gravityStatus) && angle > 2 && angle < 6)
			{
				if (LaraItem->fallspeed > 6 || item->itemFlags[0] > 1024)
				{
					LaraItem->hitPoints = -1;
					numBloods = 20;
				}
			}
			else if (LaraItem->speed < 30)
			{
				numBloods = 0;
			}
			else
			{
				LaraItem->hitPoints -= 8;
				numBloods = (GetRandomControl() & 3) + 2;
			}

			int laraY1 = LaraItem->pos.yPos + laraFrames[2];
			int laraY2 = LaraItem->pos.yPos + laraFrames[3];
			
			short triggerFlags = item->triggerFlags & 0xF;
			int itemY1;
			int itemY2;

			if (triggerFlags != 8 && triggerFlags)
			{
				itemY1 = itemFrames[2];
				itemY2 = itemFrames[3];
			}
			else
			{
				itemY1 = -itemFrames[3];
				itemY2 = -itemFrames[2];
			}
			if (laraY1 < item->pos.yPos + itemY1)
				laraY1 = itemY1 + item->pos.yPos;
			if (laraY2 > item->pos.yPos + itemY2)
				laraY2 = itemY2 + item->pos.yPos;

			long dy = laraY1 - laraY2;
			int modulus = (HIDWORD(dy) ^ dy) - HIDWORD(dy) + 1;

			angle = item->triggerFlags & 7;
			if (angle == 2 || angle == 6)
				numBloods /= 2;

			for (int i = 0; i < numBloods; i++)
			{
				TriggerBlood(
					(GetRandomControl() & 0x7F) + LaraItem->pos.xPos - 64,
					laraY2 - GetRandomControl() % modulus,
					(GetRandomControl() & 0x7F) + LaraItem->pos.zPos - 64,
					2 * GetRandomControl(),
					1);
			}

			if (LaraItem->hitPoints <= 0)
			{
				short roomNumber = LaraItem->roomNumber;
				FLOOR_INFO* floor = GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNumber);
				int height = GetFloorHeight(floor, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);

				if (item->pos.yPos >= LaraItem->pos.yPos && height - LaraItem->pos.yPos < 50)
				{
					LaraItem->animNumber = ANIMATION_LARA_SPIKED;
					LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
					LaraItem->currentAnimState = STATE_LARA_DEATH;
					LaraItem->goalAnimState = STATE_LARA_DEATH;
					LaraItem->gravityStatus = false;
				}
			}
		}

		item->itemFlags[0] += 128;
		item->itemFlags[1] += item->itemFlags[0];
		
		if (item->itemFlags[1] >= 5120)
		{
			item->itemFlags[1] = 5120;
			if (item->itemFlags[0] <= 1024)
			{
				item->itemFlags[0] = 0;
				if (!(item->triggerFlags & 0x10))
				{
					if (LaraItem->hitPoints > 0)
						item->itemFlags[2] = 64;
				}
			}
			else
			{
				item->itemFlags[0] = -item->itemFlags[0] >> 1;
			}
		}
	}
}

void InitialiseRaisingCog(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	short itemNos[32];
	int numSwitchItems = GetSwitchTrigger(item, itemNos, 1);

	if (numSwitchItems > 0)
	{
		for (int i = 0; i < numSwitchItems; i++)
		{
			ITEM_INFO* currentItem = &Items[itemNos[i]];

			if (currentItem->objectNumber == ID_TRIGGER_TRIGGERER)
			{
				item->itemFlags[1] = currentItem->roomNumber;
			}

			if (currentItem->objectNumber == ID_PULLEY || currentItem->objectNumber == ID_TRIGGER_TRIGGERER)
			{
				currentItem->itemFlags[1] = 1;
				PulleyItemNumber = itemNos[i];
			}
		}
	}
}

void RaisingCogControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TriggerActive(item))
	{
		if (item->itemFlags[0] >= 3)
		{
			AnimateItem(item);
		}
		else
		{
			if (item->itemFlags[2] >= 256)
			{
				item->itemFlags[2] = 0;
				item->itemFlags[0]++;

				if (item->itemFlags[0] == 3)
				{
					short itemNos[32];
					short numItems = GetSwitchTrigger(item, itemNos, 1);

					if (numItems > 0)
					{
						for (int i = 0; i < numItems; i++)
						{
							ITEM_INFO* currentItem = &Items[itemNos[i]];

							if (item->objectNumber == ID_PULLEY)
							{
								if (currentItem->roomNumber == item->itemFlags[1])
								{
									currentItem->itemFlags[1] = 0;
									currentItem->collidable = true;
								}
								else
								{
									currentItem->itemFlags[1] = 1;
								}
							}
							else if (item->objectNumber == ID_TRIGGER_TRIGGERER)
							{
								AddActiveItem(itemNos[i]);
								currentItem->status = ITEM_ACTIVE;
								currentItem->aiBits = (GUARD | MODIFY | AMBUSH | PATROL1 | FOLLOW);
							}
						}
					}
				}

				RemoveActiveItem(itemNumber);
				item->status = ITEM_INACTIVE;
				item->aiBits = 0;
			}
			else
			{
				if (!item->itemFlags[2])
				{
					InitialiseSpotCam(item->itemFlags[2]);
					UseSpotCam = 1;
				}
				
				int flags = 0;

				if (item->itemFlags[2] >= 31)
				{
					if (item->itemFlags[2] <= 224)
						flags = 31;
					else
						flags = 255 - item->itemFlags[2];
				}
				else
				{
					flags = item->itemFlags[2];
				}

				SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, (flags << 8) | 8);

				item->itemFlags[2] += 2;
				item->pos.yPos -= 2;
			}
		}
	}
}

void TriggerElectricityWiresSparks(int x, int z, char objNum, char node, int flags)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->sR = -1;
	spark->sG = -1;
	spark->sB = -1;
	spark->dB = -1;
	spark->dG = (GetRandomControl() & 0x7F) + 64;
	spark->dR = 0;

	if (flags)
	{
		spark->colFadeSpeed = 1;
		spark->fadeToBlack = 0;
		spark->life = spark->sLife = 4;
	}
	else
	{
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 4;
		spark->life = spark->sLife = 16;
	}

	spark->fxObj = objNum;
	spark->transType = 2;
	spark->flags = SP_ITEM | SP_NODEATTATCH | SP_SCALE | SP_DEF;
	spark->nodeNumber = node;
	spark->x = x;
	spark->z = z;
	spark->y = 0;

	if (flags)
	{
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
	}
	else
	{
		spark->xVel = (GetRandomControl() & 0x1FF) - 256;
		spark->yVel = GetRandomControl() - 64;
		spark->zVel = (GetRandomControl() & 0x1FF) - 256;
	}
	spark->friction = 51;
	spark->maxYvel = 0;
	spark->gravity = 0;

	if (flags)
	{
		spark->scalar = 1;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
		spark->size = spark->sSize = (GetRandomControl() & 0x1F) + 160;
	}
	else
	{
		spark->scalar = 0;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 14;
		spark->size = spark->sSize = (GetRandomControl() & 7) + 8;
	}

	spark->dSize = spark->size >> 1;
}

void TriggerLaraElectricitySparks(int flame)
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	GetLaraJointPosition(&pos, GetRandomControl() % 15);
	
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	spark->dR = 0;
	spark->colFadeSpeed = 8;
	byte color = (GetRandomControl() & 0x3F) - 64;
	spark->sR = color;
	spark->sB = color;
	spark->sG = color;
	spark->dB = color;
	spark->dG = color >> 1;
	spark->transType = 2;
	spark->fadeToBlack = 4;
	spark->life = 12;
	spark->sLife = 12;
	spark->x = pos.x;
	spark->y = pos.y;
	spark->z = pos.z;
	spark->xVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->yVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->zVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->friction = 51;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->flags = 0;

	if (flame)
		TriggerFireFlame(pos.x, pos.y, pos.z, -1, 254);
}

int ElectricityWireCheckDeadlyBounds(PHD_VECTOR* pos, short delta)
{
	if (pos->x + delta >= DeadlyBounds[0] && pos->x - delta <= DeadlyBounds[1]
		&& pos->y + delta >= DeadlyBounds[2] && pos->y - delta <= DeadlyBounds[3] 
		&& pos->z + delta >= DeadlyBounds[4] && pos->z - delta <= DeadlyBounds[5])
	{
		return 1;
	}

	return 0;
}

void ElectricityWiresControl(short itemNumber)
{
	bool flag = false;
	int counter = 3;

	ITEM_INFO* item = &Items[itemNumber];

	if (item->itemFlags[0] > 2)
	{
		TriggerDynamicLight(
			LaraItem->pos.xPos,
			LaraItem->pos.yPos,
			LaraItem->pos.zPos,
			item->itemFlags[0],
			0,
			(GetRandomControl() & 0x1F) + 8 * item->itemFlags[0],
			(GetRandomControl() & 0x1F) + 8 * item->itemFlags[0]);

		item->itemFlags[0] -= 2;
	}

	if (TriggerActive(item))
	{
		SoundEffect(SFX_ELECTRIC_WIRES, &item->pos, 0);
	
		counter = (abs(LaraItem->pos.xPos - item->pos.xPos) > 2048)
			+ (abs(LaraItem->pos.zPos - item->pos.zPos) > 2048)
			+ (abs(LaraItem->pos.yPos - item->pos.yPos) > 4096);
		
		int x = (GetRandomControl() & 0x1F) - 16;
		int z = (GetRandomControl() & 0x1F) - 16;

		for (int i = 0; i < 3; i++)
		{
			if (GetRandomControl() & 1)
				TriggerElectricityWiresSparks(x, z, itemNumber, i + 2, 0);
		}

		if (!(GlobalCounter & 3))
		{
			TriggerElectricityWiresSparks(0, 0, itemNumber, 2, 1);
			TriggerElectricityWiresSparks(0, 0, itemNumber, 3, 1);
			TriggerElectricityWiresSparks(0, 0, itemNumber, 4, 1);
		}
	}
	else
	{
		flag = true;
	}

	AnimateItem(item);

	if (!Lara.burn && !flag && !counter)
	{
		GetLaraDeadlyBounds();

		int i = 2;
		while (true)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;
	
			GetJointAbsPosition(item, &pos, i);

			if (ElectricityWireCheckDeadlyBounds(&pos, item->triggerFlags))
			{
				for (int i = 0; i < 48; i++)
				{
					TriggerLaraElectricitySparks(0);
				}

				item->itemFlags[0] = 28;
				LaraBurn();
				Lara.burnBlue = 1;
				Lara.BurnCount = 48;
				LaraItem->hitPoints = 0;
				return;
			}
			
			i += 3;
			if (i >= 27)
				break;
		}
	}

	int i = 8;
	int j = 0;
	counter = GlobalCounter % 3;
	short roomNumber = item->roomNumber;
	bool water = false;

	do
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 256;
		GetJointAbsPosition(item, &pos, i);

		if (GetRandomControl() & 1 && !flag)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, 0, ((GetRandomControl() & 0x3F) + 128) >> 1, (GetRandomControl() & 0x3F) + 128);
		}
		
		roomNumber = item->roomNumber;
		GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		ROOM_INFO* r = &Rooms[roomNumber];
		
		if (r->flags & ENV_FLAG_WATER)
		{
			if (counter == j)
			{
				SetupRipple(pos.x, r->maxceiling, pos.z, (GetRandomControl() & 7) + 32, 16);
			}
			
			water = true;
		}

		i += 9;
		j++;
	} while (i < 27);

	if (!flag && !Lara.burn)
	{
		if (water)
		{
			int flipNumber = Rooms[roomNumber].flipNumber;

			PHD_VECTOR pos1;
			pos1.x = 0;
			pos1.y = 0;
			pos1.z = 0;			
			GetLaraJointPosition(&pos1, LJ_LFOOT);

			short roomNumber1 = LaraItem->roomNumber;
			GetFloor(pos1.x, pos1.y, pos1.z, &roomNumber1);

			PHD_VECTOR pos2;
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = 0;
			GetLaraJointPosition(&pos2, LJ_RFOOT);

			short roomNumber2 = LaraItem->roomNumber;
			GetFloor(pos2.x, pos2.y, pos2.z, &roomNumber2);

			if (Rooms[roomNumber1].flipNumber == flipNumber
				|| Rooms[roomNumber2].flipNumber == flipNumber)
			{
				if (LaraItem->hitPoints > 32)
				{
					SoundEffect(SFX_LARA_ELECTRIC_CRACKLES, &LaraItem->pos, 0);
					TriggerLaraElectricitySparks(0);
					TriggerLaraElectricitySparks(1);
					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 8, 0, GetRandomControl() & 0x7F, (GetRandomControl() & 0x3F) + 128);
					LaraItem->hitPoints -= 10;
				}
				else
				{
					item->itemFlags[0] = 28;
					LaraBurn();
					Lara.burnBlue = 1;
					Lara.BurnCount = 48;
					LaraItem->hitPoints = 0;
				}
			}
		}
	}
}

void InitialiseRomeHammer(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->itemFlags[0] = 2;
	item->itemFlags[3] = 250;
}