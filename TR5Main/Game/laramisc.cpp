static struct COLL_INFO* coll = &mycoll; // offset 0xA0B8C
short SubsuitAir = 0; // offset 0xA122E
struct COLL_INFO mycoll;
short cheat_hitPoints; // offset 0xA3828

void GetLaraDeadlyBounds()//4B408(<), 4B86C (F)
{
#if PSX_VERSION || PSXPC_VERSION///@TODO PC subs not there yet.
	short* bounds;
	short tbounds[6];

	bounds = GetBoundsAccurate(lara_item);
	mPushUnitMatrix();
	mRotYXZ(lara_item->pos.yRot, lara_item->pos.xRot, lara_item->pos.z_rot);
	mSetTrans(0, 0, 0);
	mRotBoundingBoxNoPersp(bounds, &tbounds[0]);
	mPopMatrix();

	DeadlyBounds[0] = lara_item->pos.xPos + tbounds[0];
	DeadlyBounds[1] = lara_item->pos.xPos + tbounds[1];
	DeadlyBounds[2] = lara_item->pos.yPos + tbounds[2];
	DeadlyBounds[3] = lara_item->pos.yPos + tbounds[3];
	DeadlyBounds[4] = lara_item->pos.zPos + tbounds[4];
	DeadlyBounds[5] = lara_item->pos.zPos + tbounds[5];
#else
	UNIMPLEMENTED();
#endif
}

void DelAlignLaraToRope(struct ITEM_INFO* item)//4B3D8, 4B83C
{
	UNIMPLEMENTED();
}

void InitialiseLaraAnims(struct ITEM_INFO* item)//4B340(<), 4B7A4 (F)
{
	if (room[item->room_number].flags & RF_FILL_WATER)
	{
		Lara.waterStatus = LW_UNDERWATER;
		item->goalAnimState = STATE_LARA_UNDERWATER_STOP;
		item->currentAnimState = STATE_LARA_UNDERWATER_STOP;
		item->fallspeed = 0;
		item->animNumber = ANIMATION_LARA_UNDERWATER_IDLE;
		item->frameNumber = Anims[ANIMATION_LARA_UNDERWATER_IDLE].frameBase;
	}
	else
	{
		Lara.waterStatus = LW_ABOVE_WATER;
		item->goalAnimState = STATE_LARA_STOP;
		item->currentAnimState = STATE_LARA_STOP;
		item->animNumber = ANIMATION_LARA_STAY_SOLID;
		item->frameNumber = Anims[ANIMATION_LARA_STAY_SOLID].frameBase;
	}
}

void InitialiseLaraLoad(short item_num)//4B308, 4B76C (F)
{
	Lara.item_number = item_num;
	lara_item = &items[item_num];
}

void LaraControl(short item_number)//4A838, 4AC9C
{
	long oldx; // $s7
	long oldy; // $fp
	long oldz; // stack offset -44
	struct ITEM_INFO* item; // $s0
	int wh; // $s3
	int wd; // $s4
	int hfw; // $s2
	int room_water_state; // $s5
	short room_number; // stack offset -48

	//a1 = &lara
	//a0 = Lara.bitfield
	//v1 = Lara.IsMoving
	//v0 = Lara.MoveCount & 0xFF
	//v1 = Lara.MoveCount + 1;

	item = lara_item;

	if (Lara.IsMoving)
	{
		if (++Lara.MoveCount > 90)
		{
			Lara.IsMoving = 0;
			Lara.gun_status = 0;
		}//loc_4A8B0
	}//loc_4A8B0

	//a1 = &lara
	if (!bDisableLaraControl)
	{
		//v1 = &lara
		Lara.locationPad = -128;
	}//loc_4A8D0

	oldx = lara_item->pos.xPos;//s7
	oldy = lara_item->pos.yPos;//fp
	oldz = lara_item->pos.zPos;//a0

	//a0 = lara_item
	if (Lara.gun_status == 1 && lara_item->currentAnimState == 2 &&
		lara_item->goalAnimState == 2 && lara_item->animNumber == 0x67 &&
		lara_item->gravity_status == 0)
	{
		Lara.gun_status = 0;
	}//loc_4A944

	//a0 = &lara
	if (item->currentAnimState != 0x49 && DashTimer < 120)
	{
		DashTimer++;
	}//loc_4A978

	Lara.IsDucked = 0;
	//v0 = &room[item->room_number]
	room_water_state = room[item->room_number].flags & RF_FILL_WATER;
	wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->room_number);
	wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->room_number);

	hfw = -32152;
	if (wh != -32152)
	{
		hfw = item->pos.yPos - wh;
	}//loc_4A9F0
	Lara.water_surface_dist = -hfw;
	WadeSplash(item, wh, wd);
	//s1 = &lara
	//a2 = Lara.waterStatus

	switch (Lara.waterStatus)
	{
	case 0:///@DONE
	{
		//loc_4AA4C
		if (hfw < 256)
		{
			//loc_4AF90
			break;
		}

		if (wd >= 475)
		{
			if (hfw < 257 && room_water_state != 0)
			{
				Lara.air = 1800;
				Lara.waterStatus = 1;
				item->status = 0;

				UpdateLaraRoom(lara_item, 0);
				StopSoundEffect(SFX_LARA_FALL);

				if (item->currentAnimState == 0x34)
				{
					item->goalAnimState = 0x23;
					item->pos.xRot = -0x1FFE;
					AnimateLara(item);
					item->fallspeed <<= 1;
				}//loc_4AABC
				else if (item->currentAnimState == 0x35)
				{
					item->goalAnimState = 0x23;
					item->pos.xRot = -0x36CE;
					AnimateLara(item);
					item->fallspeed <<= 1;
				}
				else
				{
					item->pos.xRot = -0x1FFE;
					item->currentAnimState = 0x23;
					item->animNumber = 0x11;
					item->goalAnimState = 0x11;
					item->fallspeed = (((item->fallspeed << 1) + item->fallspeed) + (((item->fallspeed << 1) + item->fallspeed) >> 31)) >> 1;
					item->frameNumber = Anims[112].frameBase;
				}
				//loc_4AB38
				//a0 = lara_item
				//v0 = lara
				Lara.torso_yRot = 0;
				Lara.torso_xRot = 0;
				Lara.head_yRot = 0;
				Lara.head_xRot = 0;

				///Splash();
				//v0 = -0xFA4

			}//loc_4AB90
		}
		else
		{
			//loc_4AB60
			if (hfw >= 257)
			{
				//v0 = -0xFA4
				//v1 = 4
				Lara.waterStatus = 4;

				//v0 = -0xFA4
				if (!item->gravity_status)
				{
					item->goalAnimState = 2;
				}//loc_4AB94
			}//loc_4AB94

			Camera.targetElevation = -0xFA4;

			if (hfw < 256)
			{
				//v0 = &lara
				Lara.waterStatus = 0;

				if (Lara.climb_status == 0x41)
				{
					item->goalAnimState = 1;
				}
			}//loc_4ABC4

		}
		break;
	}
	case 1:///@TODO
	{
		//loc_4AD88
		room_number = item->room_number;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
		//v1 = room_number
		//a0 = room
		//v1 = room[room_number].flags
		//v0 = -32512

		//a0 = item
		if (wd != -32512 && hfw < 0
			&& ABS(hfw) < 0x100
			&& !(room[room_number].flags & RF_FILL_WATER)
			&& item->animNumber != 0x72
			&& item->animNumber != 0x77)
		{
			Lara.waterStatus = 2;
			//v0 = s3 + 1
			//v1 = anims
			//a1 = 0;
			item->pos.yPos = wh + 1;
			//a2 = Anims[114].frameBase;
			//v1 = lara_item
			item->goalAnimState = 0x21;
			item->currentAnimState = 0x21;
			//v0 = 0xB
			item->animNumber = 0x72;
			item->fallspeed = 0;
			item->frameNumber = Anims[114].frameBase;
			Lara.diveCount = 0xB;
			lara_item->pos.z_rot = 0;
			item->pos.xRot = 0;
			Lara.torso_yRot = 0;
			Lara.torso_xRot = 0;
			Lara.head_yRot = 0;
			Lara.head_xRot = 0;
			//j       loc_4AEFC
		}
		else
		{
			//loc_4AE70
			//v0 = -32512
			if (room_water_state == 0)
			{
				//a2 = &lara
				if (wd != -32512)
				{
					if (hfw < 0)
					{
						//a0 = item
						if (ABS(hfw) < 256)
						{
							//a1 = -381
							//a2 = &lara
							//v1 = anims
							//v0 = 2
							Lara.waterStatus = 2;
							item->pos.yPos = wh;
							//a3 = Anims[114].frameBase
							//v1 = 0x21
							item->goalAnimState = 0x21;
							item->currentAnimState = 0x21;
							//v1 = lara_item
							//v0 = 0x72
							item->animNumber = 0x72;
							//v0 = 0xb
							item->fallspeed = 0;
							item->frameNumber = Anims[114].frameBase;
							Lara.diveCount = 0xB;
							lara_item->pos.z_rot = 0;
							item->pos.xRot = 0;
							Lara.torso_yRot = 0;
							Lara.torso_xRot = 0;
							Lara.head_yRot = 0;
							Lara.head_xRot = 0;
						}//loc_4AF24
					}//loc_4AE8C ***********************************************
				}//loc_4AF24
			}//loc_4AF90
		}
		break;
	}
	case 2:///@TODO
		//loc_4ACCC
		break;
	case 3:///@TODO
		break;
	case 4:///@TODO
		//loc_4AB94
		break;
	}

	//loc_4AF90
	S_SetReverbType(room[item->room_number].ReverbType);

	if (item->hitPoints <= 0)
	{
		item->hitPoints = -1;

		if (Lara.death_count == 0)
		{
			S_CDStop();
		}

		Lara.death_count++;

		if ((lara_item->flags & 0x100))
		{
			Lara.death_count++;
			return;
		}
	}

	//loc_4B020
	switch (Lara.waterStatus)
	{
	case 0:
		//a0 = lara
		//v0 = lara
		if (Lara.Gassed)
		{
			if (item->hitPoints >= 0)
			{
				Lara.air--;

				if (Lara.air < 0)
				{
					Lara.air = -1;
					item->hitPoints -= 5;
				}
			}
		}
		else
		{
			//loc_4B0B0
			if (Lara.air < 1800 && item->hitPoints >= 0)
			{
				Lara.air += 10;
			}

			if (Lara.air > 1800)
			{
				Lara.air = 1800;
			}
		}
		//loc_4B0F0
		LaraAboveWater(item, coll);
		break;
	}

#if DEBUG_CAM

	if ((RawPad & 0x10))
	{
		Camera.pos.x += 0x20;
		lara_item->pos.xPos += 0x20;
	}
	if ((RawPad & 0x40))
	{
		Camera.pos.x -= 0x20;
		lara_item->pos.xPos -= 0x20;
	}

	if ((RawPad & 0x80))
	{
		Camera.pos.z -= 0x20;
		lara_item->pos.zPos -= 0x20;
	}

	if ((RawPad & 0x20))
	{
		Camera.pos.z += 0x20;
		lara_item->pos.zPos += 0x20;
	}

	if ((RawPad & 0x100))
	{
		Camera.pos.y -= 0x20;
		lara_item->pos.yPos -= 0x20;
	}

	if ((RawPad & 0x400))
	{
		Camera.pos.y += 0x20;
		lara_item->pos.yPos += 0x20;
	}

	if ((RawPad & IN_L2))
	{
		Camera.pos.x += SIN(90) * 32;
	}
	/*struct FLOOR_INFO* floor;
	short rn = Camera.pos.room_number;
	floor = GetFloor(Camera.pos.x, Camera.pos.y, Camera.pos.z, &rn);
	int height = GetHeight(floor, Camera.pos.x, Camera.pos.y, Camera.pos.z);
	UpdateLaraRoom(lara_item, height);
	if (floor != NULL)
	{
		Camera.pos.room_number = rn;
	}*/

	//item->pos.xPos += (SIN(Lara.moveAngle) * item->speed) >> W2V_SHIFT;
	//item->pos.zPos += (COS(Lara.moveAngle) * item->speed) >> W2V_SHIFT;
#endif
}

void LaraCheat(struct ITEM_INFO* item, struct COLL_INFO* coll)//4A790(<), 4ABF4(<) (F)
{
	lara_item->hitPoints = 1000;
	LaraUnderWater(item, coll);
	if (input & IN_WALK)
	{
		if (!(input & IN_LOOK))
		{
			Lara.waterStatus = LW_ABOVE_WATER;
			item->frameNumber = Anims[ANIMATION_LARA_STAY_SOLID].frameBase;
			item->animNumber = ANIMATION_LARA_STAY_SOLID;
			item->pos.z_rot = 0;
			item->pos.xRot = 0;
			Lara.torso_yRot = 0;
			Lara.torso_xRot = 0;
			Lara.head_yRot = 0;
			Lara.head_xRot = 0;
			Lara.gun_status = LG_NO_ARMS;
			LaraInitialiseMeshes();
			Lara.mesh_effects = 0;
			lara_item->hitPoints = cheat_hitPoints;
		}
	}
}

void LaraInitialiseMeshes()//4A684, 4AAE8 (F)
{
	int i;
	for (i = 0; i < 15; i++)
	{
		Lara.mesh_ptrs[i] = meshes[objects[LARA].mesh_index + 2 * i] = meshes[objects[LARA_SKIN].mesh_index + 2 * i];
	}

	if (gfCurrentLevel >= LVL5_GALLOWS_TREE && gfCurrentLevel <= LVL5_OLD_MILL)
	{
		Lara.mesh_ptrs[LM_TORSO] = meshes[objects[ANIMATING6_MIP].mesh_index + 2 * LM_TORSO];
	}

	if (Lara.gun_type == WEAPON_HK)
	{
		Lara.back_gun = WEAPON_HK;
	}
	else if (!Lara.shotgun_type_carried)
	{
		if (Lara.hk_type_carried)
		{
			Lara.back_gun = WEAPON_HK;
		}
	}
	else
	{
		Lara.back_gun = WEAPON_UZI;
	}

	Lara.gun_status = LG_NO_ARMS;
	Lara.left_arm.frameNumber = 0;
	Lara.right_arm.frameNumber = 0;
	Lara.target = 0;
	Lara.right_arm.lock = 0;
	Lara.left_arm.lock = 0;
}

void InitialiseLara(int restore)
{
	int i;
	short item;
	short gun;

	if (Lara.item_number == -1)
		return;

	item = Lara.item_number;

	lara_item->data = &lara;
	lara_item->collidable = false;

	if (restore)
	{
		struct lara_info backup;
		memcpy(&backup, &lara, sizeof(lara));
		memset(&lara, 0, sizeof(lara));
		memcpy(&Lara.pistols_type_carried, &backup.pistols_type_carried, 59);
	}
	else
	{
		memset(&lara, 0, sizeof(lara));
	}

	Lara.look = TRUE;
	Lara.item_number = item;
	Lara.hit_direction = -1;
	Lara.air = 1800;
	Lara.weapon_item = -1;
	PoisonFlag = 0;
	Lara.dpoisoned = 0;
	Lara.poisoned = 0;
	Lara.water_surface_dist = 100;
	Lara.holster = 14;
	Lara.location = -1;
	Lara.highest_location = -1;
	Lara.RopePtr = -1;
	lara_item->hitPoints = 1000;

	for (i = 0; i < gfNumPickups; i++)
	{
		DEL_picked_up_object(convert_invobj_to_obj(gfPickups[i]));
	}

	gfNumPickups = 0;

	Lara.gun_status = LG_NO_ARMS;

	gun = WEAPON_NONE;

	if (!(gfLevelFlags & GF_LVOP_YOUNG_LARA) && objects[PISTOLS_ITEM].loaded)
		gun = WEAPON_PISTOLS;

	if ((gfLevelFlags & GF_LVOP_TRAIN) && objects[HK_ITEM].loaded && (Lara.hk_type_carried & WTYPE_PRESENT))
		gun = WEAPON_HK;

	Lara.last_gun_type = Lara.gun_type = Lara.request_gun_type = gun;

	LaraInitialiseMeshes();

	Lara.skelebob = 0;

	if (objects[PISTOLS_ITEM].loaded)
		Lara.pistols_type_carried = WTYPE_PRESENT | WTYPE_AMMO_1;

	Lara.binoculars = WTYPE_PRESENT;

	if (!restore)
	{
		if (objects[FLARE_INV_ITEM].loaded)
			Lara.num_flares = 3;

		Lara.num_small_medipack = 3;
		Lara.num_large_medipack = 1;
	}

	Lara.num_pistols_ammo = -1;

	InitialiseLaraAnims(lara_item);

	DashTimer = 120;

	for (i = 0; i < gfNumTakeaways; i++)
	{
		NailInvItem(convert_invobj_to_obj(gfTakeaways[i]));
	}

	gfNumTakeaways = 0;

	weapons[WEAPON_REVOLVER].damage = gfCurrentLevel >= LVL5_BASE ? 15 : 6;

	switch (gfCurrentLevel)
	{
	case 6u:
		Lara.pickupitems &= 0xFFF7u;

		Lara.puzzleitems[0] = 10;
		return;
	case 5u:
		Lara.pickupitems = 0;
		Lara.pickupitemscombo = 0;
		Lara.keyitems = 0;
		Lara.keyitemscombo = 0;
		Lara.puzzleitemscombo = 0;

		memset(Lara.puzzleitems, 0, 12);
		return;
	case 7u:
		Lara.pickupitems = 0;

		Lara.puzzleitems[0] = 0;
		return;
	case 0xCu:
		Lara.pickupitems &= 0xFFFEu;

		Lara.puzzleitems[2] = 0;
		Lara.puzzleitems[3] = 0;
		break;
	case 0xEu:
		Lara.pickupitems &= 0xFFFDu;
		break;
	default:
		if (gfCurrentLevel < LVL5_THIRTEENTH_FLOOR || gfCurrentLevel > LVL5_RED_ALERT)
			Lara.pickupitems &= 0xFFF7u;
		return;
	}

	Lara.bottle = 0;
	Lara.wetcloth = CLOTH_MISSING;
}