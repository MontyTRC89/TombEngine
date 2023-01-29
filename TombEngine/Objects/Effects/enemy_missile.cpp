#include "framework.h"
#include "Objects/Effects/enemy_missile.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Objects/TR4/Entity/tr4_mutant.h"
#include "Objects/TR4/Entity/tr4_demigod.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/level.h"

using namespace TEN::Effects::Items;
using namespace TEN::Entities::TR4;
using namespace TEN::Math;

namespace TEN::Entities::Effects
{
	void TriggerSethMissileFlame(short fxNumber, short xVel, short yVel, short zVel)
	{
		const auto& fx = EffectList[fxNumber];
		auto& flame = *GetFreeParticle();

		flame.on = true;
		flame.sR = 0;
		flame.dR = 0;
		flame.sG = (GetRandomControl() & 0x7F) + 32;
		flame.sB = flame.dG + 64;
		flame.dB = (GetRandomControl() & 0x7F) + 32;
		flame.dG = flame.dB + 64;
		flame.fadeToBlack = 8;
		flame.colFadeSpeed = (GetRandomControl() & 3) + 4;
		flame.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		flame.life = flame.sLife = (GetRandomControl() & 3) + 16;
		flame.y = 0;
		flame.x = (GetRandomControl() & 0xF) - 8;
		flame.xVel = xVel;
		flame.yVel = yVel;
		flame.z = (GetRandomControl() & 0xF) - 8;
		flame.zVel = zVel;
		flame.friction = 68;
		flame.flags = 602;
		flame.rotAng = GetRandomControl() & 0xFFF;

		if (GetRandomControl() & 1)
			flame.rotAdd = -32 - (GetRandomControl() & 0x1F);
		else
			flame.rotAdd = (GetRandomControl() & 0x1F) + 32;

		flame.gravity = 0;
		flame.maxYvel = 0;
		flame.fxObj = fxNumber;

		if (fx.flag1 == 1)
			flame.scalar = 3;
		else
			flame.scalar = 2;

		flame.sSize = flame.size = (GetRandomControl() & 7) + 64;
		flame.dSize = flame.size / 32;
	}

	void TriggerHarpyFlameFlame(short fxNumber, short xVel, short yVel, short zVel)
	{
		const auto& fx = EffectList[fxNumber];
		auto& flame = *GetFreeParticle();

		flame.on = true;
		flame.sR = 0;
		flame.sG = (GetRandomControl() & 0x7F) + 32;
		flame.sB = flame.dG + 64;
		flame.dB = 0;
		flame.dG = flame.dR = (GetRandomControl() & 0x7F) + 32;
		flame.fadeToBlack = 8;
		flame.colFadeSpeed = (GetRandomControl() & 3) + 4;
		flame.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		flame.life = flame.sLife = (GetRandomControl() & 3) + 16;
		flame.y = 0;
		flame.x = (GetRandomControl() & 0xF) - 8;
		flame.xVel = xVel;
		flame.zVel = zVel;
		flame.z = (GetRandomControl() & 0xF) - 8;
		flame.yVel = yVel;
		flame.friction = 68;
		flame.flags = 602;
		flame.rotAng = GetRandomControl() & 0xFFF;

		if (GetRandomControl() & 1)
			flame.rotAdd = -32 - (GetRandomControl() & 0x1F);
		else
			flame.rotAdd = (GetRandomControl() & 0x1F) + 32;

		flame.gravity = 0;
		flame.maxYvel = 0;
		flame.fxObj = fxNumber;
		flame.scalar = 2;
		flame.sSize = flame.size = (GetRandomControl() & 7) + 64;
		flame.dSize = flame.size / 32;
	}

	void BubblesShatterFunction(FX_INFO* fx, int param1, int param2)
	{
		ShatterItem.yRot = fx->pos.Orientation.y;
		ShatterItem.meshIndex = fx->frameNumber;
		ShatterItem.color = Vector4::One;
		ShatterItem.sphere.x = fx->pos.Position.x;
		ShatterItem.sphere.y = fx->pos.Position.y;
		ShatterItem.sphere.z = fx->pos.Position.z;
		ShatterItem.bit = 0;
		ShatterItem.flags = fx->flag2 & 0x400;
		ShatterObject(&ShatterItem, 0, param2, fx->roomNumber, param1);
	}

	void ControlEnemyMissile(short fxNumber)
	{
		auto& fx = EffectList[fxNumber];

		auto orient = Geometry::GetOrientToPoint(
			Vector3(fx.pos.Position.x, fx.pos.Position.y + CLICK(1), fx.pos.Position.z),
			LaraItem->Pose.Position.ToVector3());

		int maxRotation = 0;
		int maxVelocity = 0;

		if (fx.flag1 == (int)MissileType::SethLarge)
		{
			maxRotation = ANGLE(2.8f);
			maxVelocity = CLICK(1);
		}
		else
		{
			if (fx.flag1 == (int)MissileType::Mutant)
			{
				if (fx.counter)
					fx.counter--;

				maxRotation = ANGLE(1.4f);
			}
			else
			{
				maxRotation = ANGLE(4.5f);
			}

			maxVelocity = CLICK(0.75f);
		}

		if (fx.speed < maxVelocity)
		{
			if (fx.flag1 == (int)MissileType::Mutant)
				fx.speed++;
			else
				fx.speed += 3;
		}

		if (fx.speed < maxVelocity &&
			fx.flag1 != (int)MissileType::SophiaLeighNormal &&
			fx.flag1 != (int)MissileType::SophiaLeighLarge)
		{
			short dy = orient.y - fx.pos.Orientation.y;
			if (abs(dy) > abs(ANGLE(180.0f)))
				dy = -dy;

			short dx = orient.x - fx.pos.Orientation.x;
			if (abs(dx) > abs(ANGLE(180.0f)))
				dx = -dx;

			dy >>= 3;
			dx >>= 3;

			if (dy < -maxRotation)
			{
				dy = -maxRotation;
			}
			else if (dy > maxRotation)
			{
				dy = maxRotation;
			}

			if (dx < -maxRotation)
			{
				dx = -maxRotation;
			}
			else if (dx > maxRotation)
			{
				dx = maxRotation;
			}

			fx.pos.Orientation.x += dx;

			if (fx.flag1 != (int)MissileType::Demigod3Radial && (fx.flag1 != (int)MissileType::Mutant || !fx.counter))
				fx.pos.Orientation.y += dy;
		}

		fx.pos.Orientation.z += 16 * fx.speed;
		if (fx.flag1 == (int)MissileType::Mutant)
			fx.pos.Orientation.z += 16 * fx.speed;

		auto prevPos = fx.pos.Position;

		int speed = (fx.speed * phd_cos(fx.pos.Orientation.x));
		fx.pos.Position.x += (speed * phd_sin(fx.pos.Orientation.y));
		fx.pos.Position.y += -((fx.speed * phd_sin(fx.pos.Orientation.x))) + fx.fallspeed;
		fx.pos.Position.z += (speed * phd_cos(fx.pos.Orientation.y));

		auto probe = GetCollision(fx.pos.Position.x, fx.pos.Position.y, fx.pos.Position.z, fx.roomNumber);

		if (fx.pos.Position.y >= probe.Position.Floor || fx.pos.Position.y <= probe.Position.Ceiling)
		{
			fx.pos.Position = prevPos;

			if (fx.flag1 != (int)MissileType::Mutant &&
				fx.flag1 != (int)MissileType::SophiaLeighNormal &&
				fx.flag1 != (int)MissileType::SophiaLeighLarge)
				BubblesShatterFunction(&fx, 0, -32);

			if (fx.flag1 == (int)MissileType::SethLarge)
			{
				TriggerShockwave(&fx.pos, 32, 160, 64, 64, 128, 00, 24, EulerAngles((((~g_Level.Rooms[fx.roomNumber].flags) / 16) & 2) * 65536, 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
			}
			else if (fx.flag1 == (int)MissileType::SophiaLeighNormal)
			{
				TriggerShockwave(&fx.pos, 5, 32, 128, 0, 128, 128, 24, EulerAngles(fx.pos.Orientation.y + ANGLE(180), 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
			}
			else if (fx.flag1 == (int)MissileType::SophiaLeighLarge)
			{
				TriggerShockwave(&fx.pos, 10, 64, 128, 0, 128, 128, 24, EulerAngles(fx.pos.Orientation.y + ANGLE(180), 0.0f, 0.0f), 0, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
			}
			else
			{
				if (fx.flag1)
				{
					if (fx.flag1 == (int)MissileType::Demigod3Single || fx.flag1 == (int)MissileType::Demigod3Radial)
					{
						TriggerShockwave(&fx.pos, 32, 160, 64, 0, 96, 128, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
					}
					else if (fx.flag1 == (int)MissileType::Demigod2)
					{
						TriggerShockwave(&fx.pos, 32, 160, 64, 128, 64, 0, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
					}
					else
					{
						if (fx.flag1 != (int)MissileType::Harpy)
						{
							if (fx.flag1 == (int)MissileType::Mutant)
							{
								TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 0, fx.roomNumber);
								TriggerShockwave(&fx.pos, 48, 240, 64, 128, 96, 0, 24, EulerAngles::Zero, 15, true, false, (int)ShockwaveStyle::Normal);
								fx.pos.Position.y -= 128;
								TriggerShockwave(&fx.pos, 48, 240, 48, 128, 112, 0, 16, EulerAngles::Zero, 15, true, false, (int)ShockwaveStyle::Normal);
								fx.pos.Position.y += 256;
								TriggerShockwave(&fx.pos, 48, 240, 48, 128, 112, 0, 16, EulerAngles::Zero, 15, true, false, (int)ShockwaveStyle::Normal);
							}

						}
						else
						{
							TriggerShockwave(&fx.pos, 32, 160, 64, 128, 128, 0, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
						}
					}
				}
				else
				{
					TriggerShockwave(&fx.pos, 32, 160, 64, 0, 128, 64, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
				}
			}

			KillEffect(fxNumber);
			return;
		}

		if (ItemNearLara(fx.pos.Position, 200))
		{
			LaraItem->HitStatus = true;
			if (fx.flag1 != (int)MissileType::Mutant &&
				fx.flag1 != (int)MissileType::SophiaLeighNormal &&
				fx.flag1 != (int)MissileType::SophiaLeighLarge)
				BubblesShatterFunction(&fx, 0, -32);

			KillEffect(fxNumber);

			if (fx.flag1 == (int)MissileType::SethLarge)
			{
				TriggerShockwave(&fx.pos, 48, 240, 64, 0, 128, 64, 24, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
				ItemCustomBurn(LaraItem, Vector3(0.0f, 0.8f, 0.1f), Vector3(0.0f, 0.9f, 0.8f));
			}
			else if (fx.flag1 == (int)MissileType::SophiaLeighNormal)
			{
				TriggerShockwave(&fx.pos, 5, 32, 128, 0, 128, 128, 24, EulerAngles(fx.pos.Orientation.y, 0.0f, 0.0f), fx.flag2, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
			}
			else if (fx.flag1 == (int)MissileType::SophiaLeighLarge)
			{
				TriggerShockwave(&fx.pos, 10, 64, 128, 0, 128, 128, 24, EulerAngles(fx.pos.Orientation.y, 0.0f, 0.0f), fx.flag2, true, false, (int)ShockwaveStyle::Normal);
				TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 2, fx.roomNumber);
			}
			else if (fx.flag1)
			{
				switch (fx.flag1)
				{
				case (int)MissileType::Demigod3Single:
				case (int)MissileType::Demigod3Radial:
					TriggerShockwave(&fx.pos, 32, 160, 64, 0, 96, 128, 16, EulerAngles::Zero, 10, true, false, (int)ShockwaveStyle::Normal);
					break;

				case (int)MissileType::Demigod2:
					TriggerShockwave(&fx.pos, 32, 160, 64, 128, 64, 0, 16, EulerAngles::Zero, 5, true, false, (int)ShockwaveStyle::Normal);
					break;

				case (int)MissileType::Harpy:
					TriggerShockwave(&fx.pos, 32, 160, 64, 128, 128, 0, 16, EulerAngles::Zero, 3, true, false, (int)ShockwaveStyle::Normal);
					break;

				case (int)MissileType::Mutant:
					TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 0, fx.roomNumber);
					TriggerShockwave(&fx.pos, 48, 240, 64, 128, 96, 0, 24, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
					fx.pos.Position.y -= 128;
					TriggerShockwave(&fx.pos, 48, 240, 48, 128, 112, 0, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
					fx.pos.Position.y += 256;
					TriggerShockwave(&fx.pos, 48, 240, 48, 128, 112, 0, 16, EulerAngles::Zero, 0, true, false, (int)ShockwaveStyle::Normal);
					ItemBurn(LaraItem);
					break;
				}
			}
			else
			{
				TriggerShockwave(&fx.pos, 24, 88, 48, 0, 128, 64, 16, EulerAngles((((~g_Level.Rooms[fx->roomNumber].flags) / 16) & 2) * 65536, 0.0f, 0.0f), 1, true, false, (int)ShockwaveStyle::Normal);
			}
		}
		else
		{
			if (probe.RoomNumber != fx.roomNumber)
				EffectNewRoom(fxNumber, probe.RoomNumber);

			auto deltaPos = prevPos - fx.pos.Position;

			if (Wibble & 4)
			{
				switch (fx.flag1)
				{
				default:
				case (int)MissileType::SethLarge:
					TriggerSethMissileFlame(fxNumber, 32 * deltaPos.x, 32 * deltaPos.y, 32 * deltaPos.z);
					break;

				case (int)MissileType::Harpy:
					TriggerHarpyFlameFlame(fxNumber, 16 * deltaPos.x, 16 * deltaPos.y, 16 * deltaPos.z);
					break;

				case (int)MissileType::Demigod3Single:
				case (int)MissileType::Demigod3Radial:
				case (int)MissileType::Demigod2:
					TriggerDemigodMissileFlame(fxNumber, 16 * deltaPos.x, 16 * deltaPos.y, 16 * deltaPos.z);
					break;

				case (int)MissileType::Mutant:
					TriggerCrocgodMissileFlame(fxNumber, 16 * deltaPos.x, 16 * deltaPos.y, 16 * deltaPos.z);
					break;
				}
			}
		}
	}
}
