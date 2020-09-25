#include "framework.h"
#include "lara.h"
#include "input.h"
#include "lara_tests.h"
#include "lara_slide.h"
#include "lara_collide.h"
#include "draw.h"

// CRAWLING & CROUCHING

// ------------------------------
// Auxiliary Functions
// ------------------------------

bool TestLaraKeepCrouched(ITEM_INFO* player, COLL_INFO* coll)
{
	if (coll->midCeiling >= -362 ||										// Low ceiling.
		((TrInput & IN_FORWARD) &&										// Moving forward and low ceiling ahead.
			TestWall(player, 512, 0, -768) &&	// TODO: This condition needs more specificity. There could be a thin platform above, making this fail. Maybe check for height to ceiling from location ahead?
			!TestWall(player, 512, 0, -362) &&
			!LaraFloorFront(player, player->pos.yRot, 512) < 384) ||
		((TrInput & IN_BACK) &&											// Moving back and low ceiling behind.
			TestWall(player, -512, 0, -768) &&
			!TestWall(player, -512, 0, -362) &&
			!LaraFloorFront(player, -player->pos.yRot, 512) < 384))
	{
		return true;
	}

	return false;
}

bool TestLaraCrawl(ITEM_INFO* player)
{
	if (Lara.gunStatus == LG_NO_ARMS && Lara.waterStatus != LW_WADE ||
		Lara.waterSurfaceDist == 256 && !(Lara.waterSurfaceDist > 256) &&
		(player->animNumber == LA_CROUCH_IDLE || player->animNumber == LA_STAND_TO_CROUCH_END) &&
		!(TrInput & IN_FLARE || TrInput & IN_DRAW) &&
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge < 900 && Lara.flareAge != 0))
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchTurn(ITEM_INFO* player)
{
	if (Lara.waterStatus != LW_WADE ||
		Lara.keepDucked)
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchRoll(ITEM_INFO* player)
{
	if (Lara.gunStatus == LG_NO_ARMS &&
		Lara.waterStatus != LW_WADE ||
		Lara.waterSurfaceDist == 256 &&
		!(Lara.waterSurfaceDist > 256) &&
		(!(LaraFloorFront(player, player ->pos.yRot, 1024) >= 384) ||					// 1 block away from hole in floor.
				!TestWall(player , 1024, 0, -256)) &&									// 1 block away from wall.
		!(TrInput & IN_FLARE || TrInput & IN_DRAW) &&									// Avoids some flare spawning/wep stuff.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge < 900 && Lara.flareAge != 0))
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlStepUp(ITEM_INFO* item)
{
	if (LaraFloorFront(item, item->pos.yRot, 256) == -256 &&
		LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
		LaraCeilingFront(item, item->pos.yRot, 256, 256) <= -512)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlStepDown(ITEM_INFO* item)
{
	if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
		LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
		LaraCeilingFront(item, item->pos.yRot, 256, -256) <= -512)
	{
		return true;
	}

	return false;
}

// TODO: Why have a different function for crawling? I've replaced every call to this for now. Test fidelity after new collision.
void SetLaraCrawlWallDeflect(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->collType)
	{
	case CT_FRONT:
	case CT_TOP_FRONT:
	{
		ShiftItem(item, coll);
		item->gravityStatus = false;
		item->speed = 0;
		break;
	}
	case CT_LEFT:
	{
		ShiftItem(item, coll);
		item->pos.yRot += ANGLE(3.0f);
		break;
	}
	case CT_RIGHT:
	{
		ShiftItem(item, coll);
		item->pos.yRot -= ANGLE(3.0f);
		break;
	}
	}
}

// ------------------------------
// CROUCHING
// Control & Collision Functions
// ------------------------------

// State:		71
// Collision	lara_col_crouch()
void lara_as_crouch(ITEM_INFO* item, COLL_INFO* coll)//14688, 14738 (F)
{
	short roomNum = item->roomNumber;

	Lara.isDucked = true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNum);

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_DUCK || Lara.keepDucked)
	{
		if (TestLaraCrouchTurn(item))
		{
			if (TrInput & IN_LEFT)
			{
				item->goalAnimState = LS_CROUCH_TURN_LEFT;
			}
			else if (TrInput & IN_RIGHT)
			{
				item->goalAnimState = LS_CROUCH_TURN_RIGHT;
			}
		}

		if (TestLaraCrawl(item))
		{
			Lara.torsoYrot = 0;	// Deprecate.
			Lara.torsoXrot = 0;

			// TODO: With interpolation, dispatches to crawl forward and back anims can be made directly.
			if (TrInput & IN_FORWARD)
			{
				item->goalAnimState = LS_CRAWL_IDLE;
			}
			else if (TrInput & IN_BACK)
			{
				item->goalAnimState = LS_CRAWL_IDLE;
			}

			return;
		}

		if (TrInput & IN_SPRINT &&		// TODO: Broken??
			TestLaraCrouchRoll(item))
		{
			item->goalAnimState = LS_CROUCH_ROLL;
			Lara.torsoYrot = 0;	// Deprecate.
			Lara.torsoXrot = 0;

			return;
		}
	}

	if (!(TrInput & IN_DUCK))
	{
		item->goalAnimState = LS_STOP;
		return;
	}
}

// State:		71
// State code:	lara_as_crouch()
void lara_col_crouch(ITEM_INFO* item, COLL_INFO* coll)//147C4, 148CC (F)
{
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (TestLaraKeepCrouched(item, coll))
	{
		Lara.keepDucked = true;
	}
	else
	{
		Lara.keepDucked = false;
	}

	if (TestLaraFall(coll))
	{
		Lara.gunStatus = LG_NO_ARMS;	//??
		SetLaraFall(item);

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		72
// Collision:	lara_col_crouch_roll()
void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetElevation = ANGLE(-20.0f);
	item->goalAnimState = LS_CROUCH_IDLE;
}

// State:		72
// State code:	lara_as_crouch_roll()
void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.isDucked = true;
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// ------------------------------
// CRAWLING
// Control & Collision Functions
// ------------------------------

// Puke.
// State:		80
// Collision:	lara_col_crawl_stop()
void lara_as_crawl_stop(ITEM_INFO* item, COLL_INFO* coll)//14970, 14A78 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	if (TrInput & IN_JUMP)
	{
		GAME_VECTOR s, d;
		MESH_INFO* StaticMesh;
		PHD_VECTOR v;

		if (LaraFloorFront(item, item->pos.yRot, 512) > 768 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{

			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)
			{
				// TODO: fix ObjectOnLOS2
				/*if (ObjectOnLOS2(&s, &d, &v, (PHD_VECTOR*)&StaticMesh) == 999)
				{*/
				item->animNumber = LA_CRAWL_JUMP_FLIP_DOWN;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;
				/*}*/
			}
		}
		else if (LaraFloorFront(item, item->pos.yRot, 256) == 768 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{
			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)// && EnableCrawlFlex3clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_23CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
		}
		else	if (LaraFloorFront(item, item->pos.yRot, 256) == 512 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{

			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)// && EnableCrawlFlex2clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_23CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
		}
		else if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 768, 512) <= 0)
		{
			s.x = LaraItem->pos.xPos;
			s.y = LaraItem->pos.yPos - 96;
			s.z = LaraItem->pos.zPos;
			s.roomNumber = LaraItem->roomNumber;

			d.x = s.x + (768 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
			d.y = s.y + 160;
			d.z = s.z + (768 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)// && EnableCrawlFlex1clickE == true)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_1CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
		}
	}

	// TODO: state dispatches need testing.
	if ((TrInput & IN_FORWARD)/* && (TrInput & IN_FORWARD) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE*/)
	{
		if (LaraFloorFront(item, item->pos.yRot, 256) == -256 &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) <= -512)
		{
			//item->animNumber = LA_CRAWL_UP_STEP;
			//item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_STEP_UP;
			//item->currentAnimState = LS_MISC_CONTROL;
		}
		else
		{
			if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
				LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
				LaraCeilingFront(item, item->pos.yRot, 256, -256) <= -512)
			{
				//item->animNumber = LA_CRAWL_DOWN_STEP;
				//item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STEP_DOWN;
				//item->currentAnimState = LS_MISC_CONTROL;
			}
		}
	}

	Lara.gunStatus = LG_HANDS_BUSY;

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	if (item->animNumber == LA_CROUCH_TO_CRAWL_START)
	{
		Lara.gunStatus = LG_HANDS_BUSY;
	}

	Camera.targetElevation = ANGLE(-23.0f);

	if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		item->requiredAnimState = LS_STOP;
	}
}

// State:		80
// State code:	lara_as_crawl_stop()
void lara_col_crawl_stop(ITEM_INFO* item, COLL_INFO* coll)//14B40, 14C74 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->goalAnimState == LS_CRAWL_TO_HANG)
	{
		return;
	}

	Lara.moveAngle = 0;

	coll->facing = Lara.moveAngle;
	coll->radius = 200;
	coll->badPos = 255;
	coll->badNeg = -127;
	coll->badCeiling = 400;
	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (TestLaraFall(coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
		SetLaraFall(item);

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	if (TestLaraKeepCrouched(item, coll))
	{
		Lara.keepDucked = true;
	}
	else
	{
		Lara.keepDucked = false;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
	{
		item->pos.yPos += coll->midFloor;
	}

	int slope = abs(coll->leftFloor2 - coll->rightFloor2);
	if (TrInput & IN_DUCK || Lara.keepDucked &&
		(!(TrInput & IN_FLARE) && !(TrInput & IN_DRAW) || TrInput & IN_FORWARD) &&
		Lara.waterStatus != LW_WADE)
	{
		if (item->animNumber == LA_CRAWL_IDLE ||
			item->animNumber == LA_CROUCH_TO_CRAWL_END ||
			item->animNumber == LA_CRAWL_TO_IDLE_END_RIGHT_POINTLESS ||
			item->animNumber == LA_CRAWL_TO_IDLE_END_LEFT_POINTLESS)
		{
			if (TrInput & IN_FORWARD)
			{
				if (abs(LaraFloorFront(item, item->pos.yRot, 256)) < 127 && HeightType != BIG_SLOPE)
				{
					item->goalAnimState = LS_CRAWL_FORWARD;
				}
			}
			else if (TrInput & IN_BACK)
			{
				short height = LaraCeilingFront(item, item->pos.yRot, -300, 128);
				short heightl = 0;
				short heightr = 0;

				if (height != NO_HEIGHT && height <= 256)
				{
					if (TrInput & IN_ACTION)
					{
						int x = item->pos.xPos;
						int z = item->pos.zPos;

						item->pos.xPos += 128 * phd_sin(item->pos.yRot - ANGLE(90.0f)) >> W2V_SHIFT;
						item->pos.zPos += 128 * phd_cos(item->pos.yRot - ANGLE(90.0f)) >> W2V_SHIFT;

						heightl = LaraFloorFront(item, item->pos.yRot, -300);

						item->pos.xPos += 256 * phd_sin(item->pos.yRot + ANGLE(90.0f)) >> W2V_SHIFT;
						item->pos.zPos += 256 * phd_cos(item->pos.yRot + ANGLE(90.0f)) >> W2V_SHIFT;

						heightr = LaraFloorFront(item, item->pos.yRot, -300);

						item->pos.xPos = x;
						item->pos.zPos = z;
					}

					height = LaraFloorFront(item, item->pos.yRot, -300);

					if (abs(height) >= 255 || HeightType == BIG_SLOPE)
					{
						if (TrInput & IN_ACTION)
						{
							if (height > 768 &&
								heightl > 768 &&
								heightr > 768 &&
								slope < 120)
							{
								int tmp;
								ITEM_INFO* tmp1;
								MESH_INFO* tmp2;
								int x = item->pos.xPos;
								int z = item->pos.zPos;

								item->pos.xPos -= 100 * phd_sin(coll->facing) >> W2V_SHIFT;
								item->pos.zPos -= 100 * phd_cos(coll->facing) >> W2V_SHIFT;

								tmp = GetCollidedObjects(item, 100, 1, &tmp1, &tmp2, 0);

								item->pos.xPos = x;
								item->pos.zPos = z;

								if (!tmp)
								{
									switch (GetQuadrant(item->pos.yRot))
									{
									case 0:
										item->pos.yRot = 0;
										item->pos.zPos = (item->pos.zPos & 0xFFFFFC00) + 225;
										break;
									case 1:
										item->pos.yRot = ANGLE(90.0f);
										item->pos.xPos = (item->pos.xPos & 0xFFFFFC00) + 225;
										break;
									case 2:
										item->pos.yRot = -ANGLE(180.0f);
										item->pos.zPos = (item->pos.zPos | 0x3FF) - 225;
										break;
									case 3:
										item->pos.yRot = -ANGLE(90.0f);
										item->pos.xPos = (item->pos.xPos | 0x3FF) - 225;
										break;
									}

									item->goalAnimState = LS_CRAWL_TO_HANG;
								}
							}
						}
					}
					else if (!(abs(height) >= 127))
					{
						item->goalAnimState = LS_CRAWL_BACK;
					}
				}
			}
			else if (TrInput & IN_LEFT)
			{
			item->goalAnimState = LS_CRAWL_TURN_LEFT;
			}
			else if (TrInput & IN_RIGHT)
			{
			item->goalAnimState = LS_CRAWL_TURN_RIGHT;
			}
		}
	}
	else
	{
	item->goalAnimState = LS_CROUCH_IDLE;
	}
}

// State:		81
// Collision:	lara_col_crawl()
void lara_as_crawl_forward(ITEM_INFO* item, COLL_INFO* coll)//150F4, 15228 (F)
{
	Lara.torsoXrot = 0;	// Deprecate.
	Lara.torsoYrot = 0;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	Camera.targetElevation = item->pos.zRot + ANGLE(-23.0f);	// TODO: Test if this jives with Krys' crawl surface adaptation.

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;	// TODO: Interpolate into crawl death.
		return;
	}

	/*if (!TestLaraCrawl(item))
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}*/

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;

		if (Lara.turnRate < ANGLE(-3.0f))
		{
			Lara.turnRate = ANGLE(-3.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;

		if (Lara.turnRate > ANGLE(3.0f))
		{
			Lara.turnRate = ANGLE(3.0f);
		}
	}

	if (TrInput & IN_FORWARD &&
		(TrInput & IN_DUCK || Lara.keepDucked))
	{
		item->goalAnimState = LS_CRAWL_FORWARD;
		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		81
// State code:	lara_as_crawl_forward()
void lara_col_crawl_forward(ITEM_INFO* item, COLL_INFO* coll)//1523C, 15370 (F)
{
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->radius = 200;
	coll->badPos = 255;
	coll->badNeg = -127;
	coll->badCeiling = 400;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, -400);

	if (TestLaraKeepCrouched(item, coll))
	{
		Lara.keepDucked = true;
	}
	else
	{
		Lara.keepDucked = false;
	}

	// TODO: Fix wall deflection causing spazz when colliding at steep angle.
	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);

		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		return;
	}

	if (TestLaraFall(coll))
	{
		SetLaraFall(item);
		Lara.gunStatus = LG_NO_ARMS;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraCrawlStepUp(item))
	{
		item->goalAnimState = LS_STEP_UP;
	}
	else if (TestLaraCrawlStepDown(item))
	{
		item->goalAnimState = LS_STEP_DOWN;
	}

	if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		84
// Collision:	lara_col_crawl_turn()
void lara_as_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll)//15390(<), 154C4(<) (F)
{
	Camera.targetElevation = ANGLE(-23.0f);
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	item->pos.yRot -= ANGLE(1.5f);

	coll->enableSpaz = 0;
	coll->enableBaddiePush = 1;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_LEFT &&
		(TrInput & IN_DUCK || Lara.keepDucked))
	{
		item->goalAnimState = LS_CRAWL_TURN_LEFT;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		85
// Collision:	lara_col_crawl_turn()
void lara_as_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll)//15484(<), 155B8(<) (F)
{
	Camera.targetElevation = ANGLE(-23.0f);
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	item->pos.yRot += ANGLE(1.5f);

	coll->enableSpaz = 0;
	coll->enableBaddiePush = 1;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_RIGHT &&
		(TrInput & IN_DUCK || Lara.keepDucked))
	{
		item->goalAnimState = LS_CRAWL_TURN_RIGHT;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// States:		84, 85
// State codes:	lara_as_turn_left(), lara_as_turn_right()
void lara_col_crawl_turn(ITEM_INFO* item, COLL_INFO* coll)//153FC, 15530 (F)
{
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		86
// Collision:	lara_col_crawl_back()
void lara_as_crawl_back(ITEM_INFO* item, COLL_INFO* coll)//154F0, 15624 (F)
{
	Lara.torsoYrot = 0;	// Deprecate.
	Lara.torsoXrot = 0;

	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	Camera.targetElevation = item->pos.zRot + ANGLE(-23.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	//if (!TestLaraCrawl(item))	// TODO: This check doesn't work as expected.
	//{
	//	item->goalAnimState = LS_CRAWL_IDLE;
	//	return;
	//}

	if (TrInput & IN_RIGHT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-3.0f))
		{
			Lara.turnRate = ANGLE(-3.0f);
		}
	}
	else if (TrInput & IN_LEFT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(3.0f))
		{
			Lara.turnRate = ANGLE(3.0f);
		}
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_DUCK || Lara.keepDucked) &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CRAWL_BACK;
		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		86
// State code:	lara_as_crawlb()
void lara_col_crawl_back(ITEM_INFO* item, COLL_INFO* coll)//15614, 15748 (F)
{
	Lara.moveAngle = 0;// ANGLE(180);

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->radius = 250;
	coll->badPos = 255;
	coll->badNeg = -127;
	coll->badCeiling = 400;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (TestLaraKeepCrouched(item, coll))
	{
		Lara.keepDucked = true;
	}
	else
	{
		Lara.keepDucked = false;
	}

	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);

		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		return;
	}

	if (TestLaraFall(coll))
	{
		SetLaraFall(item);
		Lara.gunStatus = LG_NO_ARMS;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT && coll->midFloor > -256)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		105
// Collision:	lara_col_turn()
void lara_as_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.isDucked = true;
	item->pos.yRot -= ANGLE(1.5f);
	coll->enableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LEFT &&
		(TrInput & IN_DUCK || Lara.keepDucked))
	{
		item->goalAnimState = LS_CROUCH_TURN_LEFT;
		return;
	}

	item->goalAnimState = LS_CROUCH_IDLE;
}

// State:		106
// Collision:	lara_col_turn()
void lara_as_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.isDucked = true;
	item->pos.yRot += ANGLE(1.5f);
	coll->enableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_RIGHT &&
		(TrInput & IN_DUCK || Lara.keepDucked))
	{
		item->goalAnimState = LS_CROUCH_TURN_RIGHT;
		return;
	}

	item->goalAnimState = LS_CROUCH_IDLE;
}

// States:		105, 106
// State codes:	lara_as_crouch_left(), lara_col_crouch_right()
void lara_col_crouch_turn(ITEM_INFO* item, COLL_INFO* coll)//14534, 145E4 (F)
{
	Lara.isDucked = true;
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 400);

	if (coll->midCeiling < -362)
	{
		Lara.keepDucked = false;
	}
	else
	{
		Lara.keepDucked = true;
	}

	if (TestLaraFall(coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
		SetLaraFall(item);

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
}

void lara_col_crawl_to_hang(ITEM_INFO* item, COLL_INFO* coll)//15770, 158A4 (F)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	if (item->animNumber == LA_CRAWL_TO_HANG_END)
	{
		int edgeCatch;
		int edge;

		Lara.moveAngle = 0;

		item->fallspeed = 512;
		item->pos.yPos += 255;

		coll->badPos = NO_BAD_POS;
		coll->badNeg = -STEPUP_HEIGHT;
		coll->badCeiling = BAD_JUMP_CEILING;
		coll->facing = Lara.moveAngle;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 870);

		edgeCatch = LaraTestEdgeCatch(item, coll, &edge);
		if (edgeCatch)
		{
			if (edgeCatch >= 0 || LaraTestHangOnClimbWall(item, coll))
			{
				short angle = item->pos.yRot;
				if (SnapToQuadrant(angle, 35))
				{
					BOUNDING_BOX* bounds;

					if (TestHangSwingIn(item, angle))
					{

						Lara.headYrot = 0;
						Lara.headXrot = 0;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;
						item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_MONKEYSWING_IDLE;
						item->goalAnimState = LS_MONKEYSWING_IDLE;
					}
					else
					{
						if (TestHangFeet(item, angle))
						{
							item->animNumber = LA_REACH_TO_HANG;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
							item->currentAnimState = LS_HANG;
							item->goalAnimState = LS_HANG_FEET;
						}
						else
						{
							item->animNumber = LA_REACH_TO_HANG;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->currentAnimState = LS_HANG;
							item->goalAnimState = LS_HANG;
						}
					}

					bounds = GetBoundsAccurate(item);

					if (edgeCatch <= 0)
					{
						item->pos.yPos = edge - bounds->Y1;
					}
					else
					{

						/*						item->pos.xPos += coll->shift.x;
												item->pos.zPos += coll->shift.z;
												@ORIGINAL_BUG: these two caused teleportation when Lara performed crawl2hang on triangulated geometry. replacing with shifts to the edges of blocks solved it*/

						short angl = (unsigned short)(item->pos.yRot + ANGLE(45.0f)) / ANGLE(90.0f);
						switch (angl)
						{
						case NORTH:
							item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - LARA_RAD;
							break;

						case EAST:
							item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - LARA_RAD;
							break;

						case SOUTH:
							item->pos.zPos = (item->pos.zPos & -WALL_SIZE) + LARA_RAD;
							break;

						case WEST:
							item->pos.xPos = (item->pos.xPos & -WALL_SIZE) + LARA_RAD;
							break;
						}
					}
					item->pos.yPos += coll->frontFloor - bounds->Y1;
					item->pos.yRot = angle;

					item->gravityStatus = true;
					item->speed = 2;
					item->fallspeed = 1;

					Lara.gunStatus = LG_HANDS_BUSY;
				}
			}
		}
	}
}
