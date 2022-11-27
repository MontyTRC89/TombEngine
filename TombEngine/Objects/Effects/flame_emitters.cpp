#include "framework.h"
#include "Objects/Effects/flame_emitters.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/effects/lightning.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using namespace TEN::Effects::Items;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Environment;

constexpr int FLAME_RADIUS = 128;
constexpr int FLAME_ITEM_BURN_TIMEOUT = 3 * FPS;

namespace TEN::Entities::Effects
{
	byte Flame3xzoffs[16][2] =
	{
		{ 9, 9 },
		{ 24, 9 },
		{ 40, 9	},
		{ 55, 9 },
		{ 9, 24 },
		{ 24, 24 },
		{ 40, 24 },
		{ 55, 24 },
		{ 9, 40 },
		{ 24, 40 },
		{ 40, 40 },
		{ 55, 40 },
		{ 9, 55	 },
		{ 24, 55 },
		{ 40, 55 },
		{ 55, 55 }
	};

	ObjectCollisionBounds FireBounds =
	{
		GameBoundingBox::Zero,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};

	bool FlameEmitterFlags[8];

	void BurnNearbyItems(ItemInfo* item)
	{
		GetCollidedObjects(item, FLAME_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], false);

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			auto* currentItem = CollidedItems[i];
			if (!currentItem)
				break;

			if (TestEnvironment(ENV_FLAG_WATER, currentItem->RoomNumber))
				continue;

			if ((!currentItem->IsCreature() && !currentItem->IsLara()) || currentItem->HitPoints <= 0)
				continue;

			if (currentItem->IsLara() && GetLaraInfo(item)->Control.WaterStatus == WaterStatus::FlyCheat)
				continue;

				ItemBurn(currentItem, currentItem->IsLara() ? -1 : FLAME_ITEM_BURN_TIMEOUT);					
		}
	}

	void FlameEmitterControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			// Jet flame
			if (item->TriggerFlags < 0)
			{
				short flags = -item->TriggerFlags;
				if ((flags & 7) == 2 || (flags & 7) == 7)
				{
					SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
					TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
					TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
						(GetRandomControl() & 3) + 20,
						(GetRandomControl() & 0x3F) + 192,
						(GetRandomControl() & 0x1F) + 96, 0);
				}
				else
				{
					if (item->ItemFlags[0])
					{
						if (item->ItemFlags[1])
							item->ItemFlags[1] = item->ItemFlags[1] - (item->ItemFlags[1] >> 2);

						if (item->ItemFlags[2] < 256)
							item->ItemFlags[2] += 8;

						item->ItemFlags[0]--;
						if (item->ItemFlags[0] == 1)
							item->ItemFlags[3] = (GetRandomControl() & 0x3F) + 150;
					}
					else
					{
						if (!--item->ItemFlags[3])
						{
							if (flags >> 3)
								item->ItemFlags[0] = (GetRandomControl() & 0x1F) + 30 * (flags >> 3);
							else
								item->ItemFlags[0] = (GetRandomControl() & 0x3F) + 60;
						}

						if (item->ItemFlags[2])
							item->ItemFlags[2] -= 8;

						if (item->ItemFlags[1] > -8192)
							item->ItemFlags[1] -= 512;
					}

					if (item->ItemFlags[2])
						AddFire(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber, 0.5f, item->ItemFlags[2]);

					if (item->ItemFlags[1])
					{
						SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);

						if (item->ItemFlags[1] <= -8192)
							TriggerSuperJetFlame(item, -256 - (3072 * GlobalCounter & 0x1C00), GlobalCounter & 1);
						else
							TriggerSuperJetFlame(item, item->ItemFlags[1], GlobalCounter & 1);

						TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
							(-item->ItemFlags[1] >> 10) - (GetRandomControl() & 1) + 16,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
					else
					{
						byte r = (GetRandomControl() & 0x3F) + 192;
						byte g = (GetRandomControl() & 0x1F) + 96;
						byte falloff = 10 - (GetRandomControl() & 1);

						TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
							10 - (GetRandomControl() & 1),
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96, 0);
					}
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
			}
			else
			{
				if (item->TriggerFlags < 8)
					FlameEmitterFlags[item->TriggerFlags] = true;

				AddFire(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber, 2.0f, 0);

				TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
									16 - (GetRandomControl() & 1),
									(GetRandomControl() & 0x3F) + 192,
									(GetRandomControl() & 0x1F) + 96, 0);

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);

				if ((Wibble & 0x04) && Random::TestProbability(1 / 2.0f))
					BurnNearbyItems(item);
			}
		}
		else
		{
			if (item->TriggerFlags > 0 && item->TriggerFlags < 8)
				FlameEmitterFlags[item->TriggerFlags] = false;
		}
	}

	void FlameEmitter2Control(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			// If not an emitter for flipmaps
			if (item->TriggerFlags >= 0)
			{
				// If not a moving flame
				if (item->TriggerFlags != 2)
				{
					if (item->TriggerFlags == 123)
					{
						// Middle of the block
						AddFire(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber, 0.25f, item->ItemFlags[3]);
					}
					else
					{
						float size = 1.0f;
						switch (item->TriggerFlags)
						{
						default:
						case 0:
							size = 2.0f;
							break;

						case 1:
							size = 1.0f;
							break;

						case 3:
							size = 0.5f;
							break;
						}

						AddFire(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber, size, item->ItemFlags[3]);
					}
				}

				if (item->TriggerFlags == 0 || item->TriggerFlags == 2)
				{
					if (item->ItemFlags[3])
					{
						TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
							10,
							((GetRandomControl() & 0x3F) + 192) * item->ItemFlags[3] >> 8,
							(GetRandomControl() & 0x1F) + 96 * item->ItemFlags[3] >> 8,
							0);
					}
					else
					{
						TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z,
							10,
							(GetRandomControl() & 0x3F) + 192,
							(GetRandomControl() & 0x1F) + 96,
							0);
					}
				}

				if (item->TriggerFlags == 2)
				{
					item->Pose.Position.x += phd_sin(item->Pose.Orientation.y - ANGLE(180)) * (CLICK(1) / FPS);
					item->Pose.Position.z += phd_cos(item->Pose.Orientation.y - ANGLE(180)) * (CLICK(1) / FPS);

					auto probe = GetCollision(item);
					
					if (TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber) ||
						probe.Position.Floor - item->Pose.Position.y > CLICK(2) ||
						probe.Position.Floor == NO_HEIGHT)
					{
						Weather.Flash(255, 128, 0, 0.03f);
						KillItem(itemNumber);
						return;
					}

					if (item->RoomNumber != probe.RoomNumber)
						ItemNewRoom(itemNumber, probe.RoomNumber);

					item->Pose.Position.y = probe.Position.Floor;

					if (Wibble & 7)
						TriggerFireFlame(item->Pose.Position.x, item->Pose.Position.y - 32, item->Pose.Position.z, FlameType::Medium);
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
			}
			else if (item->ItemFlags[0] == 0)
			{
				DoFlipMap(-item->TriggerFlags);
				FlipMap[-item->TriggerFlags] ^= 0x3E00u;
				item->ItemFlags[0] = 1;
			}
		}
	}

	void InitialiseFlameEmitter(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags < 0)
		{
			item->ItemFlags[0] = (GetRandomControl() & 0x3F) + 90;
			item->ItemFlags[2] = 256;

			if (((-item->TriggerFlags) & 7) == 7)
			{
				switch (item->Pose.Orientation.y)
				{
				case 0:
					item->Pose.Position.z += 512;
					break;

				case 0x4000:
					item->Pose.Position.x += 512;
					break;

				case -0x8000:
					item->Pose.Position.z -= 512;
					break;

				case -0x4000:
					item->Pose.Position.x -= 512;
					break;
				}
			}
		}
	}

	void InitialiseFlameEmitter2(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Pose.Position.y -= 64;

		if (item->TriggerFlags != 123)
		{
			switch (item->Pose.Orientation.y)
			{
			case 0:
				if (item->TriggerFlags == 2)
					item->Pose.Position.z += 80;
				else
					item->Pose.Position.z += 256;

				break;

			case 0x4000:
				if (item->TriggerFlags == 2)
					item->Pose.Position.x += 80;
				else
					item->Pose.Position.x += 256;

				break;

			case -0x8000:
				if (item->TriggerFlags == 2)
					item->Pose.Position.z -= 80;
				else
					item->Pose.Position.z -= 256;

				break;

			case -0x4000:
				if (item->TriggerFlags == 2)
					item->Pose.Position.x -= 80;
				else
					item->Pose.Position.x -= 256;

				break;
			}
		}
	}

	void InitialiseFlameEmitter3(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags >= 3)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				auto* currentItem = &g_Level.Items[i];

				if (currentItem->ObjectNumber == ID_ANIMATING3)
				{
					if (currentItem->TriggerFlags == item->TriggerFlags)
						item->ItemFlags[2] = i;
					else if (currentItem->TriggerFlags == 0)
						item->ItemFlags[3] = i;
				}
			}
		}
	}

	void FlameEmitter3Control(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (TriggerActive(item))
		{
			if (item->TriggerFlags)
			{
				SoundEffect(SFX_TR4_ELECTRIC_ARCING_LOOP, &item->Pose);

				byte g = (GetRandomControl() & 0x3F) + 192;
				byte b = (GetRandomControl() & 0x3F) + 192;

				auto origin = item->Pose.Position;
				Vector3i target;

				if (!(GlobalCounter & 3))
				{
					if (item->TriggerFlags == 2 || item->TriggerFlags == 4)
					{
						target.x = item->Pose.Position.x + 2048 * phd_sin(item->Pose.Orientation.y + ANGLE(180.0f));
						target.y = item->Pose.Position.y;
						target.z = item->Pose.Position.z + 2048 * phd_cos(item->Pose.Orientation.y + ANGLE(180.0f));

						if (GetRandomControl() & 3)
							TriggerLightning(&origin, &target, (GetRandomControl() & 0x1F) + 64, 0, g, b, 24, 0, 32, 3);
						else
							TriggerLightning(&origin, &target, (GetRandomControl() & 0x1F) + 96, 0, g, b, 32, LI_SPLINE, 32, 3);
					}
				}

				if (item->TriggerFlags >= 3 && !(GlobalCounter & 1))
				{
					short targetItemNumber = item->ItemFlags[((GlobalCounter >> 2) & 1) + 2];
					auto* targetItem = &g_Level.Items[targetItemNumber];

					target = GetJointPosition(targetItem, 0, Vector3i(0, -64, 20));

					if (!(GlobalCounter & 3))
					{
						if (GetRandomControl() & 3)
							TriggerLightning(&origin, &target, (GetRandomControl() & 0x1F) + 64, 0, g, b, 24, 0, 32, 5);
						else
							TriggerLightning(&origin, &target, (GetRandomControl() & 0x1F) + 96, 0, g, b, 32, LI_SPLINE, 32, 5);
					}

					if (item->TriggerFlags != 3 || targetItem->TriggerFlags)
						TriggerLightningGlow(target.x, target.y, target.z, 64, 0, g, b);
				}

				if ((GlobalCounter & 3) == 2)
				{
					origin = item->Pose.Position;

					target = Vector3i(
						(GetRandomControl() & 0x1FF) + origin.x - 256,
						(GetRandomControl() & 0x1FF) + origin.y - 256,
						(GetRandomControl() & 0x1FF) + origin.z - 256
					);

					TriggerLightning(&origin, &target, (GetRandomControl() & 0xF) + 16, 0, g, b, 24, LI_SPLINE | LI_MOVEEND, 32, 3);
					TriggerLightningGlow(target.x, target.y, target.z, 64, 0, g, b);
				}
			}
			else
			{
				// Small fires
				if (item->ItemFlags[0] != 0)
					item->ItemFlags[0]--;
				else
				{
					item->ItemFlags[0] = (GetRandomControl() & 3) + 8;
					short random = GetRandomControl() & 0x3F;
					if (item->ItemFlags[1] == random)
						random = (random + 13) & 0x3F;
					item->ItemFlags[1] = random;
				}

				int x, z, i;

				if (!(Wibble & 4))
				{
					i = item->ItemFlags[1] & 7;
					x = 16 * (Flame3xzoffs[i][0] - 32);
					z = 16 * (Flame3xzoffs[i][1] - 32);
					TriggerFireFlame(x + item->Pose.Position.x, item->Pose.Position.y, z + item->Pose.Position.z, FlameType::Small);
				}
				else
				{
					i = item->ItemFlags[1] >> 3;
					x = 16 * (Flame3xzoffs[i + 8][0] - 32);
					z = 16 * (Flame3xzoffs[i + 8][1] - 32);
					TriggerFireFlame(x + item->Pose.Position.x, item->Pose.Position.y, z + item->Pose.Position.z, FlameType::Small);
				}

				SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);

				TriggerDynamicLight(x, item->Pose.Position.y, z, 12, (GetRandomControl() & 0x3F) + 192, ((GetRandomControl() >> 4) & 0x1F) + 96, 0);

				auto pos = item->Pose.Position;
				if (ItemNearLara(pos, FLAME_RADIUS))
				{
					if (LaraItem->Effect.Type != EffectType::None && Lara.Control.WaterStatus != WaterStatus::FlyCheat)
					{
						DoDamage(LaraItem, 5);

						int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
						int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

						if (SQUARE(dx) + SQUARE(dz) < SQUARE(450))
							ItemBurn(LaraItem);
					}
				}
			}
		}
	}

	void FlameEmitterCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (Lara.Control.Weapon.GunType != LaraWeaponType::Torch ||
			Lara.Control.HandStatus != HandStatus::WeaponReady ||
			Lara.LeftArm.Locked ||
			Lara.Torch.IsLit == (item->Status & 1) ||
			item->Timer == -1 ||
			!(TrInput & IN_ACTION) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			laraItem->Animation.IsAirborne)
		{
			if (item->ObjectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			switch (item->ObjectNumber)
			{
			case ID_FLAME_EMITTER:
				FireBounds.BoundingBox.X1 = -256;
				FireBounds.BoundingBox.X2 = 256;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 1024;
				FireBounds.BoundingBox.Z1 = -800;
				FireBounds.BoundingBox.Z2 = 800;
				break;

			case ID_FLAME_EMITTER2:
				FireBounds.BoundingBox.X1 = -256;
				FireBounds.BoundingBox.X2 = 256;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 1024;
				FireBounds.BoundingBox.Z1 = -600;
				FireBounds.BoundingBox.Z2 = 600;
				break;

			case ID_BURNING_ROOTS:
				FireBounds.BoundingBox.X1 = -384;
				FireBounds.BoundingBox.X2 = 384;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 2048;
				FireBounds.BoundingBox.Z1 = -384;
				FireBounds.BoundingBox.Z2 = 384;
				break;
			}

			short oldYrot = item->Pose.Orientation.y;
			item->Pose.Orientation.y = laraItem->Pose.Orientation.y;

			if (TestLaraPosition(FireBounds, item, laraItem))
			{
				if (item->ObjectNumber == ID_BURNING_ROOTS)
					laraItem->Animation.AnimNumber = LA_TORCH_LIGHT_5;
				else
				{
					Lara.Torch.State = TorchState::JustLit;
					int dy = abs(laraItem->Pose.Position.y - item->Pose.Position.y);
					laraItem->ItemFlags[3] = 1;
					laraItem->Animation.AnimNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}

				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				Lara.Flare.ControlLeft = false;
				Lara.LeftArm.Locked = true;
				Lara.InteractedItem = itemNumber;
			}

			item->Pose.Orientation.y = oldYrot;
		}

		if (Lara.InteractedItem == itemNumber &&
			item->Status != ITEM_ACTIVE &&
			laraItem->Animation.ActiveState == LS_MISC_CONTROL)
		{
			if (laraItem->Animation.AnimNumber >= LA_TORCH_LIGHT_1 && laraItem->Animation.AnimNumber <= LA_TORCH_LIGHT_5)
			{
				if (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase == 40)
				{
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);

					item->Flags |= 0x3E00;
					item->ItemFlags[3] = 0;
					item->Status = ITEM_ACTIVE;

					AddActiveItem(itemNumber);
				}
			}
		}
	}
}
