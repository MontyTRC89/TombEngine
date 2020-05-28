#include "framework.h"
#include "newobjects.h"
#include "oldobjects.h"
#include "lara.h"
#include "draw.h"
#include "global.h"
#include "items.h"
#include "collide.h"
#include "effect.h"
#include "laramisc.h"
#include "box.h"
#include "tomb4fx.h"
#include "switch.h"
#include "spotcam.h"
#include "effect2.h"
#include "sphere.h"
#include "traps.h"
#include "camera.h"
#include "setup.h"
#include "level.h"
#include "sound.h"

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