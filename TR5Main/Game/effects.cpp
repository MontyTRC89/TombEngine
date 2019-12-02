#include "effects.h"
#include "..\Global\global.h"
#include "Lara.h"
#include "draw.h"
#include "sphere.h"

void __cdecl SetupSplash(SPLASH_SETUP* setup)
{
	for (int i = 0; i < 4; i++)
	{
		SPLASH_STRUCT* splash = &Splashes[i];

		if (!(splash->flags & 1))
		{
			splash->flags = 1;
			splash->x = setup->x;
			splash->y = setup->y;
			splash->z = setup->z;
			splash->life = 62;
			splash->innerRad = setup->innerRad;
			splash->innerSize = setup->innerSize;
			splash->innerRadVel = setup->innerRadVel;
			splash->innerYVel = setup->innerYVel;
			splash->innerY = setup->innerYVel >> 2;
			splash->middleRad = setup->middleRad;
			splash->middleSize = setup->middleSize;
			splash->middleRadVel = setup->middleRadVel;
			splash->middleYVel = setup->middleYVel;
			splash->middleY = setup->middleYVel >> 2;
			splash->outerRad = setup->outerRad;
			splash->outerSize = setup->outerSize;
			splash->outerRadVel = setup->outerRadVel;

			break;
		}
	}

	SoundEffect(SFX_LARA_SPLASH, (PHD_3DPOS*)setup, 0);
}

void __cdecl WadeSplash(ITEM_INFO* item, int wh, int wd)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	
	ROOM_INFO* room = &Rooms[roomNumber];
	if (room->flags & ENV_FLAG_WATER)
	{
		short roomNumber2 = item->roomNumber;
		GetFloor(item->pos.xPos, room->y - 128, item->pos.zPos, &roomNumber2);
		
		ROOM_INFO* room2 = &Rooms[roomNumber2];
		if (!(room2->flags & ENV_FLAG_WATER))
		{
			short* frame = GetBestFrame(item);
			if (item->pos.yPos + frame[2] <= wh)
			{
				if (item->pos.yPos + frame[3] >= wh)
				{
					if (item->fallspeed <= 0 || wd >= 474 || SplashCount != 0)
					{
						if (!(Wibble & 0xF))
						{
							if (!(GetRandomControl() & 0xF) || item->currentAnimState != STATE_LARA_STOP)
							{
								if (item->currentAnimState == STATE_LARA_STOP)
								{
									SetupRipple(item->pos.xPos, wh, item->pos.zPos, (GetRandomControl() & 0xF) + 112, 16);
								}
								else
								{
									SetupRipple(item->pos.xPos, wh, item->pos.zPos, (GetRandomControl() & 0xF) + 112, 18);
								}
							}
						}
					}
					else
					{
						SplashSetup.y = wh;
						SplashSetup.x = item->pos.xPos;
						SplashSetup.z = item->pos.zPos;
						SplashSetup.innerRad = 16;
						SplashSetup.innerSize = 12;
						SplashSetup.innerRadVel = 160;
						SplashSetup.innerYVel = -72 * item->fallspeed;
						SplashSetup.middleRad = 24;
						SplashSetup.middleSize = 24;
						SplashSetup.middleRadVel = 224;
						SplashSetup.middleYVel = -36 * item->fallspeed;
						SplashSetup.outerRad = 32;
						SplashSetup.outerSize = 32;
						SplashSetup.outerRadVel = 272;
						SetupSplash(&SplashSetup);
						SplashCount = 16;
					}
				}
			}
		}
	}
}

void __cdecl Splash(ITEM_INFO* item)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	
	ROOM_INFO* room = &Rooms[roomNumber];
	if (room->flags & ENV_FLAG_WATER)
	{
		int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);
		SplashSetup.y = wh;
		SplashSetup.x = item->pos.xPos;
		SplashSetup.z = item->pos.zPos;
		SplashSetup.innerRad = 32;
		SplashSetup.innerSize = 8;
		SplashSetup.innerRadVel = 320;
		SplashSetup.innerYVel = -40 * item->fallspeed;
		SplashSetup.middleRad = 48;
		SplashSetup.middleSize = 32;
		SplashSetup.middleRadVel = 480;
		SplashSetup.middleYVel = -20 * item->fallspeed;
		SplashSetup.outerRad = 32;
		SplashSetup.outerSize = 128;
		SplashSetup.outerRadVel = 544;
		SetupSplash(&SplashSetup);
	}
}
