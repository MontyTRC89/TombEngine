#include "framework.h"
#include "tr5_bodypart.h"
#include "Game/effects/effects.h"
#include "Specific/trmath.h"
#include "Sound/sound.h"
#include "tr5_missile.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/prng.h"

using namespace TEN::Math::Random;

constexpr int BODY_PART_LIFE = 64;

void ControlBodyPart(short fxNumber)
{
	FX_INFO* fx = &EffectList[fxNumber];
	int x = fx->pos.Position.x;
	int y = fx->pos.Position.y;
	int z = fx->pos.Position.z;

	if (fx->counter <= 0)
	{
		if (fx->speed)
			fx->pos.Orientation.x += 4 * fx->fallspeed;

		fx->fallspeed += 6;
	}
	else
	{
		int modulus = 62 - fx->counter;
		int random = modulus <= 1 ? 0 : 2 * GetRandomControl() % modulus;
		if (fxNumber & 1)
		{
			fx->pos.Orientation.z -= random;
			fx->pos.Orientation.x += random;
		}
		else
		{
			fx->pos.Orientation.z += random;
			fx->pos.Orientation.x -= random;
		}

		if (--fx->counter < 8)
			fx->fallspeed += 2;
	}

	int fallspeed = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx->roomNumber) ? fx->fallspeed / 4 : fx->fallspeed;
	int speed = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx->roomNumber) ? fx->speed / 4 : fx->speed;

	fx->pos.Position.x += speed * phd_sin(fx->pos.Orientation.y);
	fx->pos.Position.y += fallspeed;
	fx->pos.Position.z += speed * phd_cos(fx->pos.Orientation.y);

	if ((fx->flag2 & EXPLODE_NORMAL) &&
		!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx->roomNumber))
	{
		TriggerFireFlame(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, -1, 3);
	}

	auto probe = GetCollision(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, fx->roomNumber);

	if (!fx->counter)
	{
		if (fx->pos.Position.y < probe.Position.Ceiling)
		{
			fx->pos.Position.y = probe.Position.Ceiling;
			fx->fallspeed = -fx->fallspeed;
			fx->speed -= (fx->speed / 8);
		}

		if (fx->pos.Position.y >= probe.Position.Floor)
		{
			if (fx->flag2 & 1)
			{
				fx->pos.Position.x = x;
				fx->pos.Position.y = y;
				fx->pos.Position.z = z;

				if (fx->flag2 & 0x200)
					ExplodeFX(fx, -2, 32);
				else
					ExplodeFX(fx, -1, 32);

				KillEffect(fxNumber);

				if (fx->flag2 & 0x800)
					SoundEffect(SFX_TR4_ROCK_FALL_LAND, &fx->pos);

				return;
			}

			if (y <= probe.Position.Floor)
			{
				if (fx->fallspeed <= 32)
					fx->fallspeed = 0;
				else
					fx->fallspeed = -fx->fallspeed / 4;
			}
			else
			{
				fx->pos.Orientation.y += -ANGLE(180);
				fx->pos.Position.x = x;
				fx->pos.Position.z = z;
			}

			fx->speed -= (fx->speed / 4);
			if (abs(fx->speed) < 4)
				fx->speed = 0;
			fx->pos.Position.y = y;
		}

		if (!fx->speed && ++fx->flag1 > BODY_PART_LIFE)
		{
			for (int i = 0; i < 8; i++)
			{
				TriggerFlashSmoke(fx->pos.Position.x + GenerateInt(-16, 16), 
								  fx->pos.Position.y + GenerateInt( 16, 32),
								  fx->pos.Position.z + GenerateInt(-16, 16), fx->roomNumber);
			}
			KillEffect(fxNumber);
			return;
		}

		if (fx->flag2 & 2 && (GetRandomControl() & 1))
		{
			DoBloodSplat(
				(GetRandomControl() & 0x3F) + fx->pos.Position.x - 32,
				(GetRandomControl() & 0x1F) + fx->pos.Position.y - 16,
				(GetRandomControl() & 0x3F) + fx->pos.Position.z - 32,
				1,
				2 * GetRandomControl(),
				fx->roomNumber);
		}
	}

	if (probe.RoomNumber != fx->roomNumber)
	{
		if ( TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, probe.RoomNumber) &&
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx->roomNumber))
		{
			int waterHeight = GetWaterHeight(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z, probe.RoomNumber);
			SplashSetup.y = waterHeight - 1;
			SplashSetup.x = fx->pos.Position.x;
			SplashSetup.z = fx->pos.Position.z;
			SplashSetup.splashPower = fx->fallspeed;
			SplashSetup.innerRadius = 48;
			SetupSplash(&SplashSetup, probe.RoomNumber);
		}

		EffectNewRoom(fxNumber, probe.RoomNumber);
	}
}