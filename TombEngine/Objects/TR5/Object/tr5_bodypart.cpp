#include "framework.h"
#include "Objects/TR5/Object/tr5_bodypart.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Objects/TR5/Object/tr5_missile.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

constexpr auto BODY_PART_LIFE				  = 64;
constexpr auto BOUNCE_FALLSPEED				  = 32;
constexpr auto BODY_PART_EXPLODE_DAMAGE		  = 50;
constexpr auto BODY_PART_EXPLODE_DAMAGE_RANGE = CLICK(3.0f);

// TODO: Remove LaraItem global - TokyoSU: 12/6/2023
static void BodyPartExplode(ItemInfo& fx)
{
	TriggerExplosionSparks(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 3, -2, 0, fx.RoomNumber);
	TriggerExplosionSparks(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, 3, -1, 0, fx.RoomNumber);
	TriggerShockwave(&fx.Pose, 48, 304, (GetRandomControl() & 0x1F) + 112, 128, 32, 32, 32, EulerAngles(ANGLE(12.0f), 0, 0), 0, true, false, (int)ShockwaveStyle::Normal);
	
	if (ItemNearLara(fx.Pose.Position, BODY_PART_EXPLODE_DAMAGE_RANGE))
		DoDamage(LaraItem, BODY_PART_EXPLODE_DAMAGE);
}

void ControlBodyPart(short fxNumber)
{
	auto& fx = g_Level.Items[fxNumber];
	auto& fxInfo = GetFXInfo(fx);

	int x = fx.Pose.Position.x;
	int y = fx.Pose.Position.y;
	int z = fx.Pose.Position.z;

	if (fxInfo.Counter <= 0)
	{
		if (fx.Animation.Velocity.z)
			fx.Pose.Orientation.x += 4 * fx.Animation.Velocity.y;

		fx.Animation.Velocity.y += 6;
	}
	else
	{
		int modulus = 62 - fxInfo.Counter;
		int random = modulus <= 1 ? 0 : 2 * GetRandomControl() % modulus;
		if (fxNumber & 1)
		{
			fx.Pose.Orientation.z -= random;
			fx.Pose.Orientation.x += random;
		}
		else
		{
			fx.Pose.Orientation.z += random;
			fx.Pose.Orientation.x -= random;
		}

		if (--fxInfo.Counter < 8)
			fx.Animation.Velocity.y += 2;
	}

	int fallspeed = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber) ? fx.Animation.Velocity.y / 4 : fx.Animation.Velocity.y;
	int speed = TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber) ? fx.Animation.Velocity.z / 4 : fx.Animation.Velocity.z;

	fx.Pose.Position.x += speed * phd_sin(fx.Pose.Orientation.y);
	fx.Pose.Position.y += fallspeed;
	fx.Pose.Position.z += speed * phd_cos(fx.Pose.Orientation.y);

	if ((fxInfo.Flag2 & BODY_DO_EXPLOSION) && !(fxInfo.Flag2 & BODY_NO_FLAME) &&
		!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber))
	{
		if (Random::GenerateInt(0, 10) > (abs(fx.Animation.Velocity.y) > 0 ? 5 : 8))
			TriggerFireFlame(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, FlameType::Medium);
	}

	auto pointColl = GetCollision(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, fx.RoomNumber);

	if (!fxInfo.Counter)
	{
		if (fx.Pose.Position.y < pointColl.Position.Ceiling)
		{
			fx.Pose.Position.y = pointColl.Position.Ceiling;
			fx.Animation.Velocity.y = -fx.Animation.Velocity.y;
			fx.Animation.Velocity.z -= (fx.Animation.Velocity.z / 8);
		}

		if (fx.Pose.Position.y >= pointColl.Position.Floor)
		{
			if (fxInfo.Flag2 & BODY_NO_BOUNCE)
			{
				fx.Pose.Position.x = x;
				fx.Pose.Position.y = y;
				fx.Pose.Position.z = z;

				if (!(fxInfo.Flag2 & BODY_NO_SHATTER_EFFECT))
				{
					if (fxInfo.Flag2 & BODY_NO_BOUNCE_ALT)
					{
						ExplodeFX(fx, -2, 32);
					}
					else
					{
						ExplodeFX(fx, -1, 32);
					}
				}
				
				// Remove if touched floor (no bounce mode).
				if (fxInfo.Flag2 & BODY_PART_EXPLODE)
					BodyPartExplode(fx);

				KillEffect(fxNumber);

				if (fxInfo.Flag2 & BODY_STONE_SOUND)
					SoundEffect(SFX_TR4_ROCK_FALL_LAND, &fx.Pose);

				return;
			}

			if (y <= pointColl.Position.Floor)
			{
				// Remove if touched floor (no bounce mode).
				if (fxInfo.Flag2 & BODY_PART_EXPLODE)
				{
					BodyPartExplode(fx);
					KillEffect(fxNumber);
				}

				if (fx.Animation.Velocity.y <= BOUNCE_FALLSPEED)
				{
					fx.Animation.Velocity.y = 0;
				}
				else
				{
					fx.Animation.Velocity.y = -fx.Animation.Velocity.y / 4;

					if (!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber))
					{
						if (fxInfo.Flag2 & BODY_STONE_SOUND)
						{
							SoundEffect(SFX_TR4_ROCK_FALL_LAND, &fx.Pose);
						}
						else if (fxInfo.Flag2 & BODY_GIBS)
						{
							SoundEffect(SFX_TR4_LARA_THUD, &fx.Pose, SoundEnvironment::Land, Random::GenerateFloat(0.8f, 1.2f));
						}
					}
				}
			}
			else
			{
				fx.Pose.Orientation.y += -ANGLE(180);
				fx.Pose.Position.x = x;
				fx.Pose.Position.z = z;
			}

			fx.Animation.Velocity.z -= (fx.Animation.Velocity.z / 4);
			if (abs(fx.Animation.Velocity.z) < 4)
				fx.Animation.Velocity.z = 0;

			fx.Pose.Position.y = y;
		}

		if (!fx.Animation.Velocity.z && ++fxInfo.Flag1 > BODY_PART_LIFE)
		{
			if (!(fxInfo.Flag2 & BODY_NO_SMOKE))
			{
				for (int i = 0; i < 6; i++)
				{
					TriggerFlashSmoke(
						fx.Pose.Position.x + Random::GenerateInt(-16, 16),
						fx.Pose.Position.y + Random::GenerateInt(16, 32),
						fx.Pose.Position.z + Random::GenerateInt(-16, 16), fx.RoomNumber);
				}
			}

			if (fxInfo.Flag2 & BODY_PART_EXPLODE)
				BodyPartExplode(fx);

			if (!(fxInfo.Flag2 & BODY_NO_SHATTER_EFFECT))
				ExplodeFX(fx, -1, 32);

			KillEffect(fxNumber);
			return;
		}

		if ((fxInfo.Flag2 & BODY_GIBS) && (GetRandomControl() & 1))
		{
			for (int i = 0; i < (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber) ? 1 : 6); i++)
			{
				DoBloodSplat(
					(GetRandomControl() & 0x3F) + fx.Pose.Position.x - 32,
					(GetRandomControl() & 0x1F) + fx.Pose.Position.y - 16,
					(GetRandomControl() & 0x3F) + fx.Pose.Position.z - 32,
					1,
					2 * GetRandomControl(),
					fx.RoomNumber);
			}
		}
	}

	if (pointColl.RoomNumber != fx.RoomNumber)
	{
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, pointColl.RoomNumber) &&
			!TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, fx.RoomNumber))
		{
			int waterHeight = GetWaterHeight(fx.Pose.Position.x, fx.Pose.Position.y, fx.Pose.Position.z, pointColl.RoomNumber);

			SplashSetup.y = waterHeight - 1;
			SplashSetup.x = fx.Pose.Position.x;
			SplashSetup.z = fx.Pose.Position.z;
			SplashSetup.splashPower = fx.Animation.Velocity.y;
			SplashSetup.innerRadius = 48;
			SetupSplash(&SplashSetup, pointColl.RoomNumber);

			// Remove if touched water.
			if (fxInfo.Flag2 & BODY_PART_EXPLODE)
			{
				BodyPartExplode(fx);
				KillEffect(fxNumber);
			}
		}

		EffectNewRoom(fxNumber, pointColl.RoomNumber);
	}
}
