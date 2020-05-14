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
#include "../../Game/camera.h"
#include "../../Specific/setup.h"
#include "..\..\Specific\level.h"
#include "../../Game/sound.h"

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
	spark->flags = SP_ITEM | SP_NODEATTACH | SP_SCALE | SP_DEF;
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
				Lara.burnCount = 48;
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
			GetLaraJointPosition(&pos1, LM_LFOOT);

			short roomNumber1 = LaraItem->roomNumber;
			GetFloor(pos1.x, pos1.y, pos1.z, &roomNumber1);

			PHD_VECTOR pos2;
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = 0;
			GetLaraJointPosition(&pos2, LM_RFOOT);

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
					Lara.burnCount = 48;
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

void VentilatorEffect(short* bounds, int intensity, short rot, int speed)
{
	int x, y, z;

	if (abs(intensity) == 1)
	{
		x = (bounds[0]+bounds[1]) >> 1;
		if (intensity >= 0)
			y = bounds[3];
		else
			y = bounds[2];
		z = (bounds[4] + bounds[5]) >> 1;
	}
	else
	{
		y = (bounds[2]+bounds[3]) >> 1;
		if (rot & 0x7FFF)
		{
			if (intensity >= 0)
				z = bounds[5];
			else
				z = bounds[4];
			x = (bounds[0]+bounds[1]) >> 1;
		}
		else
		{
			if (intensity >= 0)
				x = bounds[1];
			else
				x = bounds[0];
			z = (bounds[4] + bounds[5]) >> 1;
		}
	}

	if (abs(Camera.pos.x-x) <= 7168)
	{
		if (abs(Camera.pos.y-y) <= 7168)
		{
			if (abs(Camera.pos.z-z) <= 7168)
			{
				SPARKS* spark = &Sparks[GetFreeSpark()];
				
				spark->on = 1;
				spark->sR = 0;
				spark->sG = 0;
				spark->sB = 0;
				spark->dR = spark->dG = 48 * speed >> 7;
				spark->colFadeSpeed = 4;
				spark->fadeToBlack = 8;
				spark->dB = speed * ((GetRandomControl() & 8) + 48) >> 7;
				spark->transType = COLADD;
				spark->life = spark->sLife = (GetRandomControl() & 3) + 20;
				
				if (abs(intensity) == 1)
				{
					int factor = 3 * (bounds[1]-bounds[0]) >> 3;
					short angle = 2 * GetRandomControl();

					spark->x = ((bounds[0] + bounds[1]) >> 1) + ((GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT);
					spark->z = ((bounds[4] + bounds[5]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);
					
					if (intensity >= 0)
						spark->y = bounds[3];
					else
						spark->y = bounds[2];

					spark->zVel = 0;
					spark->xVel = 0;
					spark->yVel = 32 * intensity * ((GetRandomControl() & 0x1F) + 224);
				}
				else
				{
					int factor = 3 * (bounds[3] - bounds[2]) >> 3;
					short angle = 2 * GetRandomControl();

					spark->y = (bounds[2] + bounds[3]) >> 1;

					if (rot & 0x7FFF)
					{
						if (intensity >= 0)
							spark->z = bounds[5];
						else
							spark->z = bounds[4];

						spark->x = ((bounds[0] + bounds[1]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);
						spark->y += (GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT;
						spark->xVel = 0;
						spark->zVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}
					else
					{
						if (intensity >= 0)
							spark->x = bounds[1];
						else
							spark->x = bounds[0];

						spark->y += (GetRandomControl() % factor) * phd_sin(angle) >> W2V_SHIFT;
						spark->z = ((bounds[4] + bounds[5]) >> 1) + ((GetRandomControl() % factor) * phd_cos(angle) >> W2V_SHIFT);
						spark->zVel = 0;
						spark->xVel = 16 * intensity * ((GetRandomControl() & 0x1F) + 224);
					}

					spark->yVel = 0;
				}

				spark->friction = 85;
				spark->xVel = speed * spark->xVel >> 7;
				spark->yVel = speed * spark->yVel >> 7;
				spark->zVel = speed * spark->zVel >> 7;
				spark->maxYvel = 0;
				spark->gravity = 0;
				spark->flags = 0;
			}
		}
	}
}

void InitialiseVentilator(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->itemFlags[0] = item->triggerFlags << WALL_SHIFT;
	if (item->itemFlags[0] < 2048)
		item->itemFlags[0] = 3072;
}

void VentilatorControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	AnimateItem(item);

	int xChange = 0;
	int zChange = 0;

	if (TriggerActive(item))
	{
		xChange = 1;
	}
	else
	{
		xChange = 1;
		TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, 0);
		if (item->currentAnimState == 1)
		{
			//result = 5 * item->animNumber;
			if (item->frameNumber == Anims[item->animNumber].frameEnd)
				return;
		}
		else
		{
			item->goalAnimState = 1;
		}
	}

	int speed = 0;
	if (item->currentAnimState == 1)
	{
		speed = Anims[item->animNumber].frameEnd - item->frameNumber;
	}
	else
	{
		speed = 128;
	}

	short* bounds = GetBoundsAccurate(item);
	short effectBounds[6];

	effectBounds[2] = item->pos.yPos + bounds[2];
	effectBounds[3] = item->pos.yPos + bounds[3];

	if (item->objectNumber != ID_PROPELLER_V) // TODO: check this ID
	{
		if (item->pos.yRot != -ANGLE(180))
		{
			if (item->pos.yRot == -ANGLE(90))
			{
				effectBounds[0] = item->pos.xPos - bounds[5];
				effectBounds[1] = item->pos.xPos - bounds[4];
				effectBounds[4] = item->pos.zPos + bounds[0];
				effectBounds[5] = item->pos.zPos + bounds[1];
				xChange = 0;
				zChange = 1;
			}
			else
			{
				if (item->pos.yRot != ANGLE(90))
				{
					effectBounds[0] = item->pos.xPos + bounds[0];
					effectBounds[1] = item->pos.xPos + bounds[1];
					effectBounds[4] = item->pos.zPos + bounds[4];
					effectBounds[5] = item->pos.zPos + bounds[5];
					zChange = 0;
				}
				else
				{
					effectBounds[0] = item->pos.xPos + bounds[4];
					effectBounds[1] = item->pos.xPos + bounds[5];
					effectBounds[4] = item->pos.zPos - bounds[1];
					effectBounds[5] = item->pos.zPos - bounds[0];
					xChange = 0;
					zChange = 1;
				}
			}
		}
		else
		{
			effectBounds[0] = item->pos.xPos - bounds[1];
			effectBounds[1] = item->pos.xPos - bounds[0];
			effectBounds[4] = item->pos.zPos - bounds[5];
			effectBounds[5] = item->pos.zPos - bounds[4];
			zChange = 0;
		}

		VentilatorEffect(effectBounds, 2, item->pos.yRot, speed);
		VentilatorEffect(effectBounds, -2, item->pos.yRot, speed);

		if (LaraItem->pos.yPos >= effectBounds[2] && LaraItem->pos.yPos <= effectBounds[3])
		{
			if (zChange)
			{
				if (LaraItem->pos.xPos >= effectBounds[0] && LaraItem->pos.xPos <= effectBounds[1])
				{
					int z1 = abs(LaraItem->pos.zPos - effectBounds[4]);
					int z2 = abs(LaraItem->pos.zPos - effectBounds[5]);

					if (z2 >= z1)
						zChange = -zChange;
					else
						z1 = z2;

					if (z1 < item->itemFlags[0])
					{
						int dz = 96 * zChange * (item->itemFlags[0] - z1) / item->itemFlags[0];
						if (item->currentAnimState == 1)
							dz = speed * dz / 120;
						LaraItem->pos.zPos += dz;
					}
				}
			}
			else
			{
				if (LaraItem->pos.zPos >= effectBounds[4] && LaraItem->pos.zPos <= effectBounds[5])
				{
					int x1 = abs(LaraItem->pos.xPos - effectBounds[0]);
					int x2 = abs(LaraItem->pos.xPos - effectBounds[0]);

					if (x2 >= x1)
						xChange = -xChange;
					else
						x1 = x2;

					if (x1 < item->itemFlags[0])
					{
						int dx = 96 * xChange * (item->itemFlags[0] - x1) / item->itemFlags[0];
						if (item->currentAnimState == 1)
							dx = speed * dx / 120;
						LaraItem->pos.xPos += dx;
					}
				}
			}
		}
	}
	else
	{
		short tbounds[6];
		phd_RotBoundingBoxNoPersp(&item->pos, bounds, tbounds);

		effectBounds[0] = item->pos.xPos + tbounds[0];
		effectBounds[1] = item->pos.xPos + tbounds[1];
		effectBounds[4] = item->pos.zPos + tbounds[4];
		effectBounds[5] = item->pos.zPos + tbounds[5];

		VentilatorEffect(effectBounds, 1, 0, speed);
		VentilatorEffect(effectBounds, -1, 0, speed);

		if (LaraItem->pos.xPos >= effectBounds[0] && LaraItem->pos.xPos <= effectBounds[1])
		{
			if (LaraItem->pos.zPos >= effectBounds[4] && LaraItem->pos.zPos <= effectBounds[5])
			{
				int y = effectBounds[3];

				if (LaraItem->pos.yPos <= effectBounds[3])
				{
					if (effectBounds[2] - LaraItem->pos.yPos >= item->itemFlags[0])
						return;
					y = 96 * (effectBounds[3] - item->itemFlags[0]) / item->itemFlags[0];
				}
				else
				{
					if (LaraItem->pos.yPos - effectBounds[3] >= item->itemFlags[0])
						return;
					y = 96 * (item->itemFlags[0] - (LaraItem->pos.yPos - effectBounds[3])) / item->itemFlags[0];
				}
				if (item->currentAnimState == 1)
					y = speed * y / 120;
				LaraItem->pos.yPos += y;
			}
		}
	}
}

void DartControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->touchBits)
	{
		LaraItem->hitPoints -= 25;
		LaraItem->hitStatus = true;
		Lara.poisoned += 160;
		DoBloodSplat(item->pos.xPos, item->pos.yPos, item->pos.zPos, (GetRandomControl() & 3) + 4, LaraItem->pos.yRot, LaraItem->roomNumber);
		KillItem(itemNumber);
	}
	else
	{
		item->pos.xPos += item->speed * phd_sin(item->pos.yRot) >> W2V_SHIFT;
		item->pos.yPos -= item->speed * phd_sin(item->pos.xRot) >> W2V_SHIFT;
		item->pos.xPos += item->speed * phd_cos(item->pos.yRot) >> W2V_SHIFT;

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);
		
		int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->floor = height;
	
		if (item->pos.yPos >= height)
		{
			for (int i = 0; i < 4; i++)
			{
				TriggerDartSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, 0, 1);
			}
			
			KillItem(itemNumber);
		}
	}
}

void DartEmitterControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->active)
	{
		if (item->timer > 0)
		{
			item->timer--;
			return;
		}
		else
		{
			item->timer = 24;
		}
	}
	
	short dartItemNumber = CreateItem();

	if (dartItemNumber != NO_ITEM)
	{
		ITEM_INFO* dartItem = &Items[dartItemNumber];

		dartItem->objectNumber = ID_DARTS;
		dartItem->roomNumber = item->roomNumber;
		
		int x = 0;
		int z = 0;

		if (item->pos.yRot > 0)
		{
			if (item->pos.yRot == ANGLE(90))
				x = 512;
		}
		else if (item->pos.yRot < 0)
		{
			if (item->pos.yRot == -ANGLE(180))
			{
				z = -512;
			}
			else if (item->pos.yRot == -ANGLE(90))
			{
				x = -512;
			}
		}
		else
		{
			z = 512;
		}

		dartItem->pos.xPos = x + item->pos.xPos;
		dartItem->pos.yPos = item->pos.yPos - 512;
		dartItem->pos.zPos = z + item->pos.zPos;

		InitialiseItem(dartItemNumber);

		dartItem->pos.xRot = 0;
		dartItem->pos.yRot = item->pos.yRot + -ANGLE(180);
		dartItem->speed = 256;

		int xf = 0;
		int zf = 0;

		if (x)
			xf = abs(2 * x) - 1;
		else
			zf = abs(2 * z) - 1;

		for (int i = 0; i < 5; i++)
		{
			int random = -GetRandomControl();
			
			int xv = 0;
			int zv = 0;

			if (z >= 0)
				zv = zf & random;
			else
				zv = -(zf & random);

			if (x >= 0)
				xv = xf & random;
			else
				xv = -(xf & random);

			TriggerDartSmoke(dartItem->pos.xPos, dartItem->pos.yPos, dartItem->pos.zPos, xv, zv, 0);
		}

		AddActiveItem(dartItemNumber);
		dartItem->status = ITEM_ACTIVE;
		SoundEffect(SFX_LIFT_DOORS, &dartItem->pos, 0);
	}
}

void FallingCeilingControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (item->currentAnimState)
	{
		if (item->currentAnimState == 1 && item->touchBits)
		{
			LaraItem->hitPoints -= 300;
			LaraItem->hitStatus = true;
		}
	}
	else
	{
		item->goalAnimState = 1;
		item->gravityStatus = true;;
	}
	
	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
	{
		RemoveActiveItem(itemNumber);
	}
	else
	{
		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		
		if (roomNumber != item->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->currentAnimState == 1)
		{
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->gravityStatus = false;
				item->goalAnimState = 2;
				item->fallspeed = 0;
			}
		}
	}
}

void RollingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	if (TestBoundsCollide(item, l, coll->radius))
	{
		if (TestCollision(item, l))
		{
			if (TriggerActive(item) && (item->itemFlags[0] || item->fallspeed))
			{
				LaraItem->animNumber = ANIMATION_LARA_SQUASH_BOULDER;
				LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
				LaraItem->goalAnimState = STATE_LARA_DEATH;
				LaraItem->currentAnimState = STATE_LARA_DEATH;
				LaraItem->gravityStatus = false;
			}
			else
			{
				ObjectCollision(itemNumber, l, coll);
			}
		}
	}
}

void RollingBallControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->fallspeed += GRAVITY;

	item->pos.xPos += item->itemFlags[0] >> 5;
	item->pos.yPos += item->fallspeed;
	item->pos.zPos += item->itemFlags[1] >> 5;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int dh = height - 512;

	if (item->pos.yPos > height - 512)
	{
		if (abs(item->fallspeed) > 16)
		{
			int distance = sqrt(
				SQUARE(Camera.pos.x - item->pos.xPos) +
				SQUARE(Camera.pos.y - item->pos.yPos) +
				SQUARE(Camera.pos.z - item->pos.zPos));

			if (distance < 16384)
				Camera.bounce = -((16384 - distance) * abs(item->fallspeed) >> 14);
		}

		if (item->pos.yPos - dh < 512)
			item->pos.yPos = dh;

		if (item->fallspeed <= 64)
		{
			if (abs(item->speed) <= 512 || GetRandomControl() & 0x1F)
				item->fallspeed = 0;
			else
				item->fallspeed = -(short)(GetRandomControl() % (item->speed >> 3));
		}
		else
		{
			item->fallspeed = -(short)(item->fallspeed >> 2);
		}
	}

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	floor = GetFloor(x, y, z + 128, &roomNumber);
	int y1a = GetFloorHeight(floor, x, y, z + 128) - 512;

	floor = GetFloor(x, y, z - 128, &roomNumber);
	int y2a = GetFloorHeight(floor, x, y, z - 128) - 512;

	floor = GetFloor(x + 128, y, z, &roomNumber);
	int y3a = GetFloorHeight(floor, x + 128, y, z) - 512;

	floor = GetFloor(x - 128, y, z, &roomNumber);
	int y4a = GetFloorHeight(floor, x - 128, y, z) - 512;

	floor = GetFloor(x, y, z + 512, &roomNumber);
	int y1b = GetFloorHeight(floor, x, y, z + 512) - 512;

	floor = GetFloor(x, y, z - 512, &roomNumber);
	int y2b = GetFloorHeight(floor, x, y, z - 512) - 512;

	floor = GetFloor(x + 512, y, z, &roomNumber);
	int y3b = GetFloorHeight(floor, x + 512, y, z) - 512;

	floor = GetFloor(x - 512, y, z, &roomNumber);
	int y4b = GetFloorHeight(floor, x - 512, y, z) - 512;

	if (item->pos.yPos - dh > -256
		|| item->pos.yPos - y1b >= 512
		|| item->pos.yPos - y3b >= 512
		|| item->pos.yPos - y2b >= 512
		|| item->pos.yPos - y4b >= 512)
	{
		int counterZ = 0;

		if (y1a - dh <= 256)
		{
			if (y1b - dh < -1024 || y1a - dh < -256)
			{
				if (item->itemFlags[1] <= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & 0xFFFFFE00) + 512;
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] >> 1;
					item->pos.zPos = (item->pos.zPos & 0xFFFFFE00) + 512;
				}
			}
			else if (y1a == dh)
			{
				counterZ = 1;
			}
			else
			{
				item->itemFlags[1] += (y1a - dh) >> 1;
			}
		}

		if (y2a - dh <= 256)
		{
			if (y2b - dh < -1024 || y2a - dh < -256)
			{
				if (item->itemFlags[1] >= 0)
				{
					if (!item->itemFlags[1] && item->itemFlags[0])
					{
						item->pos.zPos = (item->pos.zPos & 0xFFFFFE00) + 512;
					}
				}
				else
				{
					item->itemFlags[1] = -item->itemFlags[1] >> 1;
					item->pos.zPos = (item->pos.zPos & 0xFFFFFE00) + 512;
				}
			}
			else if (y2a == dh)
			{
				counterZ++;
			}
			else
			{
				item->itemFlags[1] -= (y2a - dh) >> 1;
			}
		}

		if (counterZ == 2)
		{
			if (abs(item->itemFlags[1]) <= 64)
				item->itemFlags[1] = 0;
			else
				item->itemFlags[1] = item->itemFlags[1] - (item->itemFlags[1] >> 6);
		}

		int counterX = 0;

		if (y4a - dh <= 256)
		{
			if (y4b - dh < -1024 || y4a - dh < -256)
			{
				if (item->itemFlags[0] >= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & 0xFFFFFE00) + 512;
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] >> 1;
					item->pos.xPos = (item->pos.xPos & 0xFFFFFE00) + 512;
				}
			}
			else if (y4a == dh)
			{
				counterX = 1;
			}
			else
			{
				item->itemFlags[0] -= (y4a - dh) >> 1;
			}
		}

		if (y3a - dh <= 256)
		{
			if (y3b - dh < -1024 || y3a - dh < -256)
			{
				if (item->itemFlags[0] <= 0)
				{
					if (!item->itemFlags[0] && item->itemFlags[1])
					{
						item->pos.xPos = (item->pos.xPos & 0xFFFFFE00) + 512;
					}
				}
				else
				{
					item->itemFlags[0] = -item->itemFlags[0] >> 1;
					item->pos.xPos = (item->pos.xPos & 0xFFFFFE00) + 512;
				}
			}
			else if (y3a == dh)
			{
				counterX++;
			}
			else
			{
				item->itemFlags[0] += (y3a - dh) >> 1;
			}
		}

		if (counterX == 2)
		{
			if (abs(item->itemFlags[0]) <= 64)
				item->itemFlags[0] = 0;
			else
				item->itemFlags[0] = item->itemFlags[0] - (item->itemFlags[0] >> 6);
		}
	}

	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->itemFlags[0] <= 3072)
	{
		if (item->itemFlags[0] < -3072)
			item->itemFlags[0] = -3072;
	}
	else
	{
		item->itemFlags[0] = 3072;
	}

	if (item->itemFlags[1] <= 3072)
	{
		if (item->itemFlags[1] < -3072)
			item->itemFlags[1] = -3072;
	}
	else
	{
		item->itemFlags[1] = 3072;
	}

	short angle = 0;

	if (item->itemFlags[1] || item->itemFlags[0])
		angle = phd_atan(item->itemFlags[1], item->itemFlags[0]);
	else
		angle = item->pos.yRot;

	if (item->pos.yRot != angle)
	{
		if (((angle - item->pos.yRot) & 0x7FFFu) >= 0x200)
		{
			if (angle <= item->pos.yRot || angle - item->pos.yRot >= 0x8000)
				item->pos.yRot -= 512;
			else
				item->pos.yRot += 512;
		}
		else
		{
			item->pos.yRot = angle;
		}
	}

	item->pos.xRot -= (abs(item->itemFlags[0]) + abs(item->itemFlags[1])) >> 1;

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	TestTriggers(TriggerIndex, 1, 0);
}