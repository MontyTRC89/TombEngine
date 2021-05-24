#include "framework.h"
#include "tr4_wheel_of_fortune.h"
#include "sound.h"
#include "items.h"
#include "control.h"
#include "setup.h"
#include "tomb4fx.h"

short SenetPiecesNumber[6];
char ActivePiece, SenetDisplacement, ActiveSenetPieces[6], SenetBoard[17];
int SenetTargetX, SenetTargetZ;

void InitialiseGameStix(short itemNumber)
{
	ITEM_INFO* item;
	
	item = &g_Level.Items[itemNumber];
	item->triggerFlags = -1;
	item->data = &item->itemFlags;
}

void GameStixControl(short itemNumber)
{
	ITEM_INFO* item, *item2, *item3;
	int i, number, x, z;
	bool flag;
	short piece, roomNumber;

	item = &g_Level.Items[itemNumber];
	if (item->triggerFlags > -1)
	{
		if (item->hitPoints == 100)
			SoundEffect(SFX_TR4_SPINNING_PUZZLE, &item->pos, 0);
		for (i = 0; i < 4; ++i)
		{
			if (item->hitPoints < 100 - 2 * i)
			{
				item->itemFlags[i] -= 128 * item->hitPoints;
				if (item->hitPoints < 40 - 2 * i)
				{
					if (abs(item->itemFlags[i]) < 4096 && item->triggerFlags & 1 << i)
					{
						item->itemFlags[i] = 0;
					}
					else if ((item->itemFlags[i] > 28672 || item->itemFlags[i] < -28672) && !(item->triggerFlags & 1 << i))
					{
						item->itemFlags[i] = -32768;
					}
				}
			}
		}
		--item->hitPoints;
		if (!item->hitPoints)
		{
			for (i = 0; i < 3; ++i)
				g_Level.Items[SenetPiecesNumber[i]].triggerFlags = 0;
			item->triggerFlags = -1;
			if (ActivePiece == -1 && !SenetDisplacement)
			{
				RemoveActiveItem(itemNumber);
				item->status = ITEM_NOT_ACTIVE;
			}
		}
	}
	else if (ActivePiece > -1)
	{
		number = ActivePiece >= 3 ? 2 : 1;
		item2 = &g_Level.Items[SenetPiecesNumber[ActivePiece]];
		item2->flags |= 32;
		item2->afterDeath = 48;
		piece = ActiveSenetPieces[ActivePiece];
		if (piece == -1 || piece >= 5)
		{
			if (piece == -1)
				piece = 16;
			x = SenetTargetX + SECTOR(1);
			z = SenetTargetZ + SECTOR(piece - 5);
		}
		else
		{
			x = SenetTargetX + SECTOR(2 * number - 2);
			z = SenetTargetZ + SECTOR(4 - piece);
		}
		if (abs(x - item2->pos.xPos) < 128)
		{
			item2->pos.xPos = x;
		}
		else if (x > item2->pos.xPos)
		{
			item2->pos.xPos += 128;
		}
		else
		{
			item2->pos.xPos -= 128;
		}
		if (abs(z - item2->pos.zPos) < 128)
		{
			item2->pos.zPos = z;
		}
		else if (z > item2->pos.zPos)
		{
			item2->pos.zPos += 128;
		}
		else
		{
			item2->pos.zPos -= 128;
		}
		roomNumber = item2->roomNumber;
		GetFloor(item2->pos.xPos, item2->pos.yPos - 32, item2->pos.zPos, &roomNumber);
		if (item2->roomNumber != roomNumber)
			ItemNewRoom(SenetPiecesNumber[ActivePiece], roomNumber);
		if (x != item2->pos.xPos || z != item2->pos.zPos)
		{
			item2->afterDeath = 0;
			if (piece == 16)
			{
				if (number == 1)
				{
					SenetPieceExplosionEffect(item2, 0x6060E0, -32);
					SenetPieceExplosionEffect(item2, 0x6060E0, 48);
				}
				else
				{
					SenetPieceExplosionEffect(item2, 0xFF8020, -32);
					SenetPieceExplosionEffect(item2, 0xFF8020, 48);
				}
				KillItem(SenetPiecesNumber[ActivePiece]);
				if (CheckSenetWinner(number))
				{
					for (i = 0; i < g_Level.NumItems; ++i)
					{
						item3 = &g_Level.Items[i];
						if (item3->objectNumber >= ID_GAME_PIECE1 && item3->objectNumber <= ID_GAME_PIECE3 || item3->objectNumber == ID_ENEMY_PIECE || item3->objectNumber == ID_WHEEL_OF_FORTUNE)
						{
							item3->flags |= IFLAG_INVISIBLE | IFLAG_ACTIVATION_MASK;
							RemoveActiveItem(i);
							item3->status = ITEM_NOT_ACTIVE;
							item3->afterDeath = 1;
						}
					}
				}
			}
			else
			{
				for (i = 0; i < 6; ++i)
				{
					if (ActivePiece != i)
					{
						item2 = &g_Level.Items[SenetPiecesNumber[i]];
						if (x == item2->pos.xPos && z == item2->pos.zPos)
						{
							SenetPieceExplosionEffect(item2, number == 1 ? 0xFF8020 : 0x6060E0, -64);
							item2->pos.xPos = SenetTargetX - SECTOR(4 * number) + SECTOR(7);
							item2->pos.zPos = SenetTargetZ + SECTOR(i % 3);
							roomNumber = item2->roomNumber;
							GetFloor(item2->pos.xPos, item2->pos.yPos - 32, item2->pos.zPos, &roomNumber);
							if (item2->roomNumber != roomNumber)
								ItemNewRoom(SenetPiecesNumber[i], roomNumber);
							SenetPieceExplosionEffect(item2, number == 1 ? 0xFF8020 : 0x6060E0, -64);
						}
					}
				}
			}
			if (!SenetDisplacement)
			{
				RemoveActiveItem(itemNumber);
				item2->status = ITEM_NOT_ACTIVE;
			}
			ActivePiece = -1;
		}
	}
	else if (SenetDisplacement == -1)
	{
		_0x0040FAE0(item);
		for (i = 3; i < 6; ++i)
		{
			MakeMove(i, SenetDisplacement);
			if (SenetDisplacement == -1 || !SenetDisplacement)
				break;
		}
		if (SenetDisplacement && SenetDisplacement != 6)
		{
			SenetDisplacement = 0;
		}
		else
		{
			SenetDisplacement = -1;
		}
	}
	else if (!SenetDisplacement)
	{
		_0x0040FAE0(item);
		for (i = 0; i < 3; ++i)
		{
			flag = false;
			if (ActiveSenetPieces[i] != -1 && SenetDisplacement && ActiveSenetPieces[i] + SenetDisplacement < 17 && !(SenetBoard[ActiveSenetPieces[i] + SenetDisplacement] & 1))
				flag = true;
			if (!flag)
				SenetDisplacement = SenetDisplacement == 6 ? 0 : -1;
		}
	}
}

void _0x0040FAE0(ITEM_INFO* item)
{
	SenetDisplacement = 0;
	item->triggerFlags = 0;

	for (int i = 0; i < 4; i++)
	{
		SenetDisplacement += GetRandomControl() & 1;

		if (GetRandomControl() & 1)
			item->triggerFlags |= (1 << i);
	}

	if (!SenetDisplacement)
		SenetDisplacement = 6;

	item->hitPoints = 120;

	for (int i = 0; i > 3; i++)
	{
		g_Level.Items[SenetPiecesNumber[i]].triggerFlags = 1;
	}
}

void SenetPieceExplosionEffect(ITEM_INFO* item, int color, int speed)
{
	int radius = speed >= 0 ? 0xA00020 : 0x2000280;
	int clr = color | 0x18000000;
	item->pos.yPos -= STEPUP_HEIGHT;
	TriggerShockwave(&item->pos, radius & 0xFFFF, radius >> 16, speed, clr & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF, 64, 0, 0);
	TriggerShockwave(&item->pos, radius & 0xFFFF, radius >> 16, speed, clr & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF, 64, 0x2000, 0);
	TriggerShockwave(&item->pos, radius & 0xFFFF, radius >> 16, speed, clr & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF, 64, 0x4000, 0);
	TriggerShockwave(&item->pos, radius & 0xFFFF, radius >> 16, speed, clr & 0xFF, (clr >> 8) & 0xFF, (clr >> 16) & 0xFF, 64, 0x6000, 0);
	item->pos.yPos += STEPUP_HEIGHT;
}
