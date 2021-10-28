#include "framework.h"
#include "tr4_senet.h"
#include "Sound/sound.h"
#include "items.h"
#include "control/control.h"
#include "setup.h"
#include "effects/tomb4fx.h"
#include "lara.h"
#include "lara_struct.h"
#include "input.h"
#include "level.h"
#include "collide.h"

short SenetPiecesNumber[6];
char SenetDisplacement, ActiveSenetPieces[6], SenetBoard[17];
int SenetTargetX, SenetTargetZ;
char ActivePiece = -1;

static PHD_VECTOR GameStixPosition = { 0, 0, -100 };
OBJECT_COLLISION_BOUNDS GameStixBounds =
{ -256, 256, -200, 200, -256, 256, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

void InitialiseGameStix(short itemNumber)
{
	ITEM_INFO* item;
	
	item = &g_Level.Items[itemNumber];
	item->itemFlags[7] = -1;
	//not needed
	//item->data = &item->itemFlags;
	ActivePiece = -1;
	SenetDisplacement = 0;
}

void GameStixControl(short itemNumber)
{
	ITEM_INFO* item, *item2, *item3;
	int i, number, x, z;
	bool flag;
	short piece, roomNumber;

	item = &g_Level.Items[itemNumber];
	if (item->itemFlags[7] > -1)
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
					if (abs(item->itemFlags[i]) < 4096 && item->itemFlags[7] & 1 << i)
					{
						item->itemFlags[i] = 0;
					}
					else if ((item->itemFlags[i] > 28672 || item->itemFlags[i] < -28672) && !(item->itemFlags[7] & 1 << i))
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
			item->itemFlags[7] = -1;
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
		if (x == item2->pos.xPos && z == item2->pos.zPos)
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
		flag = false;
		if (item->triggerFlags)
		{
			for (i = 3; i < 6; ++i)
			{
				if (ActiveSenetPieces[i] != -1 && SenetDisplacement && ActiveSenetPieces[i] + SenetDisplacement == 16)
				{
					MakeMove(i, SenetDisplacement);
					if (SenetDisplacement == -1 || !SenetDisplacement)
					{
						flag = true;
						break;
					}
				}

			}
			if (!flag)
			{
				for (i = 3; i < 6; ++i)
				{
					if (ActiveSenetPieces[i] != -1 && SenetDisplacement && ActiveSenetPieces[i] + SenetDisplacement >= 5 && ActiveSenetPieces[i] + SenetDisplacement <= 16 && SenetBoard[ActiveSenetPieces[i] + SenetDisplacement] & 1)
					{
						MakeMove(i, SenetDisplacement);
						if (SenetDisplacement == -1 || !SenetDisplacement)
						{
							flag = true;
							break;
						}
					}

				}
			}
			if (!flag && SenetDisplacement != 6)
			{
				for (i = 3; i < 6; ++i)
				{
					if (ActiveSenetPieces[i] != -1 && SenetDisplacement && ActiveSenetPieces[i] + SenetDisplacement <= 16 && !(ActiveSenetPieces[i] + SenetDisplacement & 3))
					{
						MakeMove(i, SenetDisplacement);
						if (SenetDisplacement == -1 || !SenetDisplacement)
						{
							flag = true;
							break;
						}
					}

				}
			}
		}
		if (!flag)
		{
			for (i = 3; i < 6; ++i)
			{
				MakeMove(i, SenetDisplacement);
				if (SenetDisplacement == -1 || !SenetDisplacement)
					break;
			}
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
		flag = false;
		for (i = 0; i < 3; ++i)
		{
			if (ActiveSenetPieces[i] != -1 && SenetDisplacement && ActiveSenetPieces[i] + SenetDisplacement < 17 && !(SenetBoard[ActiveSenetPieces[i] + SenetDisplacement] & 1))
				flag = true;
		}
		if (!flag)
			SenetDisplacement = SenetDisplacement == 6 ? 0 : -1;
	}
}

void _0x0040FAE0(ITEM_INFO* item)
{
	SenetDisplacement = 0;
	item->itemFlags[7] = 0;

	for (int i = 0; i < 4; i++)
	{
		int rnd = GetRandomControl() & 1;
		SenetDisplacement += rnd;

		if (rnd)
			item->itemFlags[7] |= (1 << i);
	}

	if (!SenetDisplacement)
		SenetDisplacement = 6;

	item->hitPoints = 120;

	for (int i = 0; i < 3; i++)
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

void trigger_item_in_room(short room_number, int object)//originally this is in deltapak
{
	short num, nex;
	ITEM_INFO* item;

	num = g_Level.Rooms[room_number].itemNumber;

	while (num != NO_ITEM)
	{
		item = &g_Level.Items[num];
		nex = item->nextItem;

		if (item->objectNumber == object)
		{
			AddActiveItem(num);
			item->status = ITEM_ACTIVE;
			item->flags |= IFLAG_ACTIVATION_MASK;
		}

		num = nex;
	}
}

int CheckSenetWinner(short num)//original TR4 numbers :>
{
	if (num == 1)
	{
		int i = 0;
		while (ActiveSenetPieces[i] == -1)
		{
			if (++i >= 3)
			{
				trigger_item_in_room(0, ID_RAISING_BLOCK2);
				trigger_item_in_room(19, ID_RAISING_BLOCK2);
				return 1;
			}
		}
	}
	else
	{
		int j = 3;
		while (ActiveSenetPieces[j] == -1)
		{
			if (++j >= 6)
			{
				trigger_item_in_room(20, ID_TRAPDOOR1);
				trigger_item_in_room(21, ID_TRAPDOOR1);
				trigger_item_in_room(22, ID_TRAPDOOR1);
				trigger_item_in_room(81, ID_TRAPDOOR1);
				return 1;
			}
		}
	}

	return 0;
}

void MakeMove(int piece, int displacement)
{
	int number, i;

	number = piece >= 3 ? 2 : 1;
	if (ActiveSenetPieces[piece] != -1 && displacement && ActiveSenetPieces[piece] + displacement <= 16 && !(SenetBoard[ActiveSenetPieces[piece] + displacement] & number))
	{
		SenetBoard[ActiveSenetPieces[piece]] &= ~number;
		if (!ActiveSenetPieces[piece])
		{
			for (i = 3 * number - 3; i < 3 * number; ++i)
			{
				if (i != piece && !ActiveSenetPieces[i])
					SenetBoard[0] |= number;
			}
		}
		ActivePiece = piece;
		ActiveSenetPieces[ActivePiece] += displacement;
		if (ActiveSenetPieces[ActivePiece] > 4)
		{
			SenetBoard[ActiveSenetPieces[ActivePiece]] = 0;
			for (i = 6 - 3 * number; i < 9 - 3 * number; ++i)
			{
				if (ActiveSenetPieces[i] == ActiveSenetPieces[ActivePiece])
				{
					ActiveSenetPieces[i] = 0;
					SenetBoard[0] |= 3 - number;
				}
			}
		}
		if (!(ActiveSenetPieces[ActivePiece] & 3) || SenetDisplacement == 6)
		{
			SenetDisplacement = 0;
		}
		else
		{
			SenetDisplacement = -1;
		}
		if (ActiveSenetPieces[ActivePiece] < 16)
		{
			SenetBoard[ActiveSenetPieces[ActivePiece]] |= number;
		}
		else
		{
			ActiveSenetPieces[ActivePiece] = -1;
		}
	}
}

void GameStixCollision(short item_num, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[item_num];

	if (TrInput & IN_ACTION && laraitem->currentAnimState == LS_STOP && laraitem->animNumber == LA_STAND_IDLE && Lara.gunStatus == LG_NO_ARMS &&
		!item->active || Lara.isMoving && Lara.interactedItem == item_num)
	{
		laraitem->pos.yRot ^= 0x8000;

		if (TestLaraPosition(&GameStixBounds, item, laraitem))
		{
			if (MoveLaraPosition(&GameStixPosition, item, laraitem))
			{
				laraitem->animNumber = LA_SENET_ROLL;
				laraitem->frameNumber = g_Level.Anims[LA_SENET_ROLL].frameBase;
				laraitem->currentAnimState = LS_MISC_CONTROL;
				Lara.isMoving = 0;
				Lara.torsoXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoZrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->status = ITEM_ACTIVE;
				AddActiveItem(item_num);
				laraitem->pos.yRot ^= 0x8000;
				return;
			}

			Lara.interactedItem = item_num;
		}

		laraitem->pos.yRot ^= 0x8000;
	}
	else
		ObjectCollision(item_num, laraitem, coll);
}

void ControlGodHead(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->pos.yRot == 0)
			item->pos.zPos &= ~1023;
		else if (item->pos.yRot == 0x4000)
			item->pos.xPos &= ~1023;
		else if (item->pos.yRot == -0x4000)
			item->pos.xPos |= 1023;
		else if (item->pos.yRot == -0x8000)
			item->pos.zPos |= 1023;

		if (item->itemFlags[0])
		{
			if (item->itemFlags[2])
				item->itemFlags[2]--;
			else
			{
				if (item->itemFlags[1] < 128)
					KillItem(itemNumber);
				else
					item->itemFlags[1] -= 128;
			}
		}
		else
		{
			if (item->itemFlags[1] > 0x1000)
			{
				item->itemFlags[0] = 1;
				item->itemFlags[1] = 4096;
				item->itemFlags[2] = 210;
			}
			else
				item->itemFlags[1] += 128;
		}
	}
}

void InitialiseGamePiece(short itemNumber)
{
	int i;
	ITEM_INFO* item;

	if (!SenetPiecesNumber[0])
	{
		for (i = 1; i < 17; ++i)
			SenetBoard[i] = 0;
		for (i = 0; i < 6; ++i)
			ActiveSenetPieces[i] = 0;
		SenetBoard[0] = 3;
		for (i = 0; i < g_Level.NumItems; ++i)
		{
			item = &g_Level.Items[i];
			if (item->objectNumber == ID_GAME_PIECE1)
			{
				SenetPiecesNumber[0] = i;
				SenetTargetX = item->pos.xPos + 1024;
				SenetTargetZ = item->pos.zPos;
			}
			else if (item->objectNumber == ID_GAME_PIECE2)
			{
				SenetPiecesNumber[1] = i;
			}
			else if (item->objectNumber == ID_GAME_PIECE3)
			{
				SenetPiecesNumber[2] = i;
			}
			else if (item->objectNumber == ID_ENEMY_PIECE)
			{
				SenetPiecesNumber[3 + item->triggerFlags] = i;
			}
		}
	}
}

void SenetControl(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];

	if (SenetDisplacement > 0 && item->triggerFlags != 1)
		MakeMove(item->objectNumber - ID_GAME_PIECE1, SenetDisplacement);

	RemoveActiveItem(itemNumber);
}
