#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "animation.h"
#include "control/los.h"
#include "Lara/lara_flare.h"
#include "collide.h"
#include "items.h"
#include "camera.h"
#include "control/control.h"
#include "item.h"
/*this file has all the related functions to ducking and crawling*/

/*crouch/duck start*/
void lara_as_duck(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 71*/
	/*collision: lara_col_duck*/
	short roomNum;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;

	Lara.isDucked = true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	roomNum = LaraItem->roomNumber;

	if (TrInput & IN_LOOK)
		LookUpDown();

	GetFloor(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, &roomNum);

	// FOR DEBUG PURPOSES UNTIL SCRIPTING IS FINISHED- ## LUA
	Lara.NewAnims.CrouchRoll = 1;


	if ((TrInput & IN_FORWARD || TrInput & IN_BACK)
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.gunStatus == LG_NO_ARMS
		&& Lara.waterStatus != LW_WADE
		|| Lara.waterSurfaceDist == 256
		&& !(Lara.waterSurfaceDist > 256))
	{

		if ((item->animNumber == LA_CROUCH_IDLE
			|| item->animNumber == LA_STAND_TO_CROUCH_END)
			&& !(TrInput & IN_FLARE || TrInput & IN_DRAW)
			&& (Lara.gunType != WEAPON_FLARE || Lara.flareAge < FLARE_AGE && Lara.flareAge != 0))
		{
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->goalAnimState = LS_CRAWL_IDLE;
		}

	}
	else if ((TrInput & IN_SPRINT) /*crouch roll*/
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.gunStatus == LG_NO_ARMS
		&& Lara.waterStatus != LW_WADE
		|| Lara.waterSurfaceDist == 256
		&& !(Lara.waterSurfaceDist > 256)
		&& Lara.NewAnims.CrouchRoll)
	{
		if (LaraFloorFront(item, item->pos.yRot, 1024) >= 384 ||  //4 clicks away from holes in the floor
			TestLaraWall(item, WALL_SIZE / 2, 0, -256))			//2 clicks away from walls + added a fix in lara_col_crouch_roll, better this way
			return;

		if (!(TrInput & IN_FLARE || TrInput & IN_DRAW) //avoids some flare spawning/wep stuff
			&& (Lara.gunType != WEAPON_FLARE || Lara.flareAge < FLARE_AGE && Lara.flareAge != 0))

		{
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->goalAnimState = LS_CROUCH_ROLL;
		}
	}
}

void lara_col_duck(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 71*/
	/*state code: lara_as_duck*/
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.ForwardAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;

	GetCollisionInfo(coll, item);

	if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		Lara.keepDucked = TestLaraStandUp(coll);
		ShiftItem(item, coll);

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;

		if (TrInput & IN_DUCK && Lara.waterStatus != LW_WADE ||
			Lara.keepDucked ||
			item->animNumber != LA_CROUCH_IDLE)
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
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)//horrible name.
{
	/*state 72*/
	/*collision: lara_col_crouch_roll*/
	Camera.targetElevation = -ANGLE(20.0f);
	item->goalAnimState = LS_CROUCH_IDLE;
}

void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)//horrible name.
{
	/*state 72*/
	/*state code: lara_as_crouch_roll*/
	item->gravityStatus = 0;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.ForwardAngle = item->pos.yRot;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;

	GetCollisionInfo(coll, item);

	if (LaraFallen(item, coll))
		Lara.gunStatus = LG_NO_ARMS;
	else if (!TestLaraSlide(item, coll))
	{
		Lara.keepDucked = TestLaraStandUp(coll);

		if (coll->Middle.Floor < coll->Setup.BadHeightUp)//hit a wall, stop
		{
			item->pos.xPos = coll->Setup.OldPosition.x;
			item->pos.yPos = coll->Setup.OldPosition.y;
			item->pos.zPos = coll->Setup.OldPosition.z;
			return;
		}

		ShiftItem(item, coll);

		if (!LaraHitCeiling(item, coll))
			item->pos.yPos += coll->Middle.Floor;
	}
}
/*crouch/duck end*/
/*-*/
/*crawl start*/
void lara_as_all4s(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 80*/
	/*collision: lara_col_all4s*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	// FOR DEBUG PURPOSES UNTIL SCRIPTING IS FINISHED
//	Lara.NewAnims.Crawl1clickdown = 1;
//	Lara.NewAnims.Crawl1clickup = 1;
//	Lara.NewAnims.CrawlExit1click = 1;
//	Lara.NewAnims.CrawlExit2click = 1;
//	Lara.NewAnims.CrawlExit3click = 1;
	Lara.NewAnims.CrawlExitJump = 1;

	if (TrInput & IN_JUMP && Lara.NewAnims.CrawlExitJump)
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

			d.x = s.x + 768 * phd_sin(LaraItem->pos.yRot);
			d.y = s.y + LARA_HEADROOM;
			d.z = s.z + 768 * phd_cos(LaraItem->pos.yRot);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)
			{
				// TODO: fix ObjectOnLOS2
				/*if (ObjectOnLOS2(&s, &d, &v, (PHD_VECTOR*)&StaticMesh) == NO_LOS_ITEM)
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

			d.x = s.x + 768 * phd_sin(LaraItem->pos.yRot);
			d.y = s.y + LARA_HEADROOM;
			d.z = s.z + 768 * phd_cos(LaraItem->pos.yRot);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && Lara.NewAnims.CrawlExit3click)
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

			d.x = s.x + 768 * phd_sin(LaraItem->pos.yRot);
			d.y = s.y + LARA_HEADROOM;
			d.z = s.z + 768 * phd_cos(LaraItem->pos.yRot);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && Lara.NewAnims.CrawlExit2click)
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

			d.x = s.x + 768 * phd_sin(LaraItem->pos.yRot);
			d.y = s.y + LARA_HEADROOM;
			d.z = s.z + 768 * phd_cos(LaraItem->pos.yRot);

			if (LOS(&s, &d) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE && Lara.NewAnims.CrawlExit1click)
			{
				item->animNumber = LA_CRAWL_JUMP_DOWN_1CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
				Lara.gunStatus = LG_HANDS_BUSY;

			}
		}

	}

	if ((TrInput & IN_ACTION) && (TrInput & IN_FORWARD) && item->animNumber != LA_CROUCH_TO_CRAWL_START && item->animNumber != LA_CROUCH_TO_CRAWL_CONTINUE)
	{
		if (LaraFloorFront(item, item->pos.yRot, 256) == -256 &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
			LaraCeilingFront(item, item->pos.yRot, 256, 256) <= -512 &&
			Lara.NewAnims.Crawl1clickup)
		{
			item->animNumber = LA_CRAWL_UP_STEP;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MISC_CONTROL;
			item->currentAnimState = LS_MISC_CONTROL;
		}
		else
			if (LaraFloorFront(item, item->pos.yRot, 256) == 256 &&
				LaraCeilingFront(item, item->pos.yRot, 256, 256) != NO_HEIGHT &&
				LaraCeilingFront(item, item->pos.yRot, 256, -256) <= -512 &&
				Lara.NewAnims.Crawl1clickdown)
			{
				item->animNumber = LA_CRAWL_DOWN_STEP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_MISC_CONTROL;
				item->currentAnimState = LS_MISC_CONTROL;
			}
	}

	Lara.gunStatus = LG_HANDS_BUSY;

	if (TrInput & IN_LOOK)
		LookUpDown();

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;

	if (item->animNumber == LA_CROUCH_TO_CRAWL_START)
		Lara.gunStatus = LG_HANDS_BUSY;

	Camera.targetElevation = -ANGLE(23.0f);

	if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		item->requiredAnimState = LS_STOP;
	}
}

void lara_col_all4s(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 80*/
	/*state code: lara_as_all4s*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->goalAnimState != LS_CRAWL_TO_HANG)
	{
		Lara.moveAngle = item->pos.yRot;

		coll->Setup.ForwardAngle = Lara.moveAngle;
		coll->Setup.Radius = LARA_RAD_CRAWL;
		coll->Setup.Height = LARA_HEIGHT_CRAWL;
		coll->Setup.BadHeightDown = STEP_SIZE - 1;
		coll->Setup.BadHeightUp = -127;
		coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
		coll->Setup.SlopesAreWalls = true;
		coll->Setup.SlopesArePits = true;

		GetCollisionInfo(coll, item);

		if (LaraFallen(item, coll))
		{
			Lara.gunStatus = LG_NO_ARMS;
		}
		else if (!TestLaraSlide(item, coll))
		{
			int slope = abs(coll->FrontLeft.Floor - coll->FrontRight.Floor);

			Lara.keepDucked = TestLaraStandUp(coll);
			ShiftItem(item, coll);

			if (coll->Middle.Floor != NO_HEIGHT && coll->Middle.Floor > -256)
				item->pos.yPos += coll->Middle.Floor;

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
						auto collResult = LaraCollisionFront(item, item->pos.yRot, 256);
						if (abs(collResult.Position.Floor < 127) && !collResult.Position.Slope)
							item->goalAnimState = LS_CRAWL_FORWARD;
					}
					else if (TrInput & IN_BACK)
					{
						short height = LaraCeilingFront(item, item->pos.yRot, -300, 128);
						short heightl = 0;
						short heightr = 0;

						if (height != NO_HEIGHT && height <= -256)
						{
							if (TrInput & IN_ACTION)
							{
								int x = item->pos.xPos;
								int z = item->pos.zPos;

								item->pos.xPos += 128 * phd_sin(item->pos.yRot - ANGLE(90.0f));
								item->pos.zPos += 128 * phd_cos(item->pos.yRot - ANGLE(90.0f));

								heightl = LaraFloorFront(item, item->pos.yRot, -300);

								item->pos.xPos += 256 * phd_sin(item->pos.yRot + ANGLE(90.0f));
								item->pos.zPos += 256 * phd_cos(item->pos.yRot + ANGLE(90.0f));

								heightr = LaraFloorFront(item, item->pos.yRot, -300);

								item->pos.xPos = x;
								item->pos.zPos = z;
							}

							auto collResult = LaraCollisionFront(item, item->pos.yRot, -300);
							height = collResult.Position.Floor;

							if (abs(height) >= STEP_SIZE - 1 || collResult.Position.Slope)
							{
								if (TrInput & IN_ACTION)
								{
									if (height > 768 &&
										heightl > 768 &&
										heightr > 768 &&
										slope < 120)
									{
										int tmp;
										int x = item->pos.xPos;
										int z = item->pos.zPos;

										item->pos.xPos -= 100 * phd_sin(coll->Setup.ForwardAngle);
										item->pos.zPos -= 100 * phd_cos(coll->Setup.ForwardAngle);

										tmp = GetCollidedObjects(item, 100, 1, CollidedItems, CollidedMeshes, 0);

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
							else if (!(abs(height) >= STEP_SIZE))
							{
								item->goalAnimState = LS_CRAWL_BACK;
							}
						}
					}
					else if (TrInput & IN_LEFT)
					{
						item->animNumber = LA_CRAWL_TURN_LEFT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_CRAWL_TURN_LEFT;
						item->goalAnimState = LS_CRAWL_TURN_LEFT;
					}
					else if (TrInput & IN_RIGHT)
					{
						item->animNumber = LA_CRAWL_TURN_RIGHT;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->currentAnimState = LS_CRAWL_TURN_RIGHT;
						item->goalAnimState = LS_CRAWL_TURN_RIGHT;
					}
				}
			}
			else
			{
				item->goalAnimState = LS_CROUCH_IDLE;
			}
		}
	}
}

void lara_as_crawl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 81*/
	/*collision: lara_col_crawl*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	Lara.torsoXrot = 0;
	Lara.torsoYrot = 0;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;

	Camera.targetElevation = -ANGLE(23.0f);

	if (TrInput & IN_FORWARD
		&& (TrInput & IN_DUCK || Lara.keepDucked)
		&& Lara.waterStatus != LW_WADE)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;

			if (Lara.turnRate < -ANGLE(3.0f))
				Lara.turnRate = -ANGLE(3.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;

			if (Lara.turnRate > ANGLE(3.0f))
				Lara.turnRate = ANGLE(3.0f);
		}
	}
	else
	{
		item->goalAnimState = LS_CRAWL_IDLE;
	}
}

void lara_col_crawl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 81*/
	/*state code: lara_as_crawl*/
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Radius = LARA_RAD_CRAWL;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;
	coll->Setup.BadHeightUp = -127;
	coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item, true);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		ShiftItem(item, coll);

		if (coll->Middle.Floor != NO_HEIGHT && coll->Middle.Floor > -256)
			item->pos.yPos += coll->Middle.Floor;
	}
}

void lara_as_all4turnl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 84*/
	/*collision: lara_col_all4turnlr*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	Camera.targetElevation = -ANGLE(23.0f);
	item->pos.yRot -= ANGLE(1.5f);

	if (!(TrInput & IN_LEFT))
		item->goalAnimState = LS_CRAWL_IDLE;
}

void lara_as_all4turnr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 85*/
	/*collision: lara_col_all4turnlr*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	Camera.targetElevation = -ANGLE(23.0f);
	item->pos.yRot += ANGLE(1.5f);

	if (!(TrInput & IN_RIGHT))
		item->goalAnimState = LS_CRAWL_IDLE;
}

void lara_col_all4turnlr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 84 and 85*/
	/*state code: lara_as_all4turnl(84) and lara_as_all4turnr(85)*/
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	GetCollisionInfo(coll, item);

	if (!TestLaraSlide(item, coll))
	{
		if (coll->Middle.Floor != NO_HEIGHT && coll->Middle.Floor > -256)
			item->pos.yPos += coll->Middle.Floor;
	}
}

void lara_as_crawlb(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 86*/
	/*collision: lara_col_crawlb*/
	if (item->hitPoints <= 0 || Lara.waterStatus == 4)
	{
		item->goalAnimState = LS_CRAWL_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	Camera.targetElevation = -ANGLE(23.0f);

	if (TrInput & IN_BACK)
	{
		if (TrInput & IN_RIGHT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -ANGLE(3.0f))
				Lara.turnRate = -ANGLE(3.0f);
		}
		else if (TrInput & IN_LEFT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(3.0f))
				Lara.turnRate = ANGLE(3.0f);
		}
	}
	else
	{
		item->goalAnimState = LS_CRAWL_IDLE;
	}
}

void lara_col_crawlb(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 86*/
	/*state code: lara_as_crawlb*/
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->Setup.Radius = LARA_RAD_CRAWL + 50; // TODO: Check if it still works without 50?
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);
	coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;

	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item, true);

	if (LaraDeflectEdgeDuck(item, coll))
	{
		item->currentAnimState = LS_CRAWL_IDLE;
		item->goalAnimState = LS_CRAWL_IDLE;

		if (item->animNumber != LA_CRAWL_IDLE)
		{
			item->animNumber = LA_CRAWL_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		ShiftItem(item, coll);

		if (coll->Middle.Floor != NO_HEIGHT && coll->Middle.Floor > -STEP_SIZE)
			item->pos.yPos += coll->Middle.Floor;

		Lara.moveAngle = item->pos.yRot;
	}
}

void lara_as_duckl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 105*/
	/*collision: lara_col_ducklr*/
	coll->Setup.EnableSpaz = false;
	if ((TrInput & (IN_DUCK | IN_LEFT)) != (IN_DUCK | IN_LEFT) || item->hitPoints <= 0)
		item->goalAnimState = LS_CROUCH_IDLE;
	item->pos.yRot -= ANGLE(1.5f);
}

void lara_as_duckr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 106*/
	/*collision: lara_col_ducklr*/
	coll->Setup.EnableSpaz = false;
	if ((TrInput & (IN_DUCK | IN_RIGHT)) != (IN_DUCK | IN_RIGHT) || item->hitPoints <= 0)
		item->goalAnimState = LS_CROUCH_IDLE;
	item->pos.yRot += ANGLE(1.5f);
}

void lara_col_ducklr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 105 and 106*/
	/*state code: lara_as_duckl(105) and lara_col_ducklr(106)*/
	// FIXED
	Lara.isDucked = true;
	if (TrInput & IN_LOOK)
		LookUpDown();

	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.ForwardAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;

	GetCollisionInfo(coll, item);

	if (LaraFallen(item, coll))
	{
		Lara.gunStatus = LG_NO_ARMS;
	}
	else if (!TestLaraSlide(item, coll))
	{
		Lara.keepDucked = TestLaraStandUp(coll);
		ShiftItem(item, coll);

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}
/*crawling end*/

void lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	if (item->animNumber == LA_CRAWL_TO_HANG_END)
	{
		int edgeCatch;
		int edge;

		item->fallspeed = 512;
		item->pos.yPos += 255;

		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		coll->Setup.BadHeightDown = NO_BAD_POS;
		coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
		coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
		coll->Setup.ForwardAngle = Lara.moveAngle;

		Lara.moveAngle = item->pos.yRot;

		GetCollisionInfo(coll, item);
		edgeCatch = TestLaraEdgeCatch(item, coll, &edge);

		if (edgeCatch)
		{
			if (edgeCatch >= 0 || TestLaraHangOnClimbWall(item, coll))
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
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

						/*						item->pos.xPos += coll->Shift.x;
												item->pos.zPos += coll->Shift.z;
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
					item->pos.yPos += coll->Front.Floor - bounds->Y1;
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
