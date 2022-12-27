#include "framework.h"
#include "Objects/TR5/Entity/tr5_laser_head.h"

#include "Game/items.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/effects/debris.h"
#include "Game/animation.h"
#include "Game/control/los.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Objects/TR5/Entity/tr5_laserhead_info.h"
#include "Game/effects/item_fx.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_collide.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Lightning;
using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR5
{
	int GuardianMeshes[5] = { 1, 2 };
	auto LaserHeadBasePosition = Vector3i(0, -640, 0);
	Vector3i GuardianChargePositions[4] =
	{
		Vector3i(-188, -832, 440),
		Vector3i(188, -832, -440),
		Vector3i(440, -832, 188),
		Vector3i(-440, -832, -188)
	};

	LaserHeadInfo* GetLaserHeadInfo(ItemInfo* item)
	{
		return (LaserHeadInfo*)item->Data;
	}

	void InterpolateAngle(short angle, short& rotation, short& outAngle, int shift)
	{
		int deltaAngle = angle - rotation;

		if (deltaAngle < -32768)
			deltaAngle += 65536;
		else if (deltaAngle > 32768)
			deltaAngle -= 65536;

		if (outAngle)
			outAngle = (short)deltaAngle;

		rotation += short(deltaAngle >> shift);
	}

	void TriggerLaserHeadSparks(Vector3i* pos, int count, byte r, byte g, byte b, int unk)
	{
		if (count > 0)
		{
			for (int i = 0; i < count; i++)
			{
				auto* spark = GetFreeParticle();

				spark->on = 1;
				spark->sR = r;
				spark->sG = g;
				spark->sB = b;
				spark->dB = 0;
				spark->dG = 0;
				spark->dR = 0;
				spark->colFadeSpeed = 9 << unk;
				spark->fadeToBlack = 0;
				spark->life = 9 << unk;
				spark->sLife = 9 << unk;
				spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
				spark->x = pos->x;
				spark->y = pos->y;
				spark->z = pos->z;
				spark->gravity = (GetRandomControl() / 128) & 0x1F;
				spark->yVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
				spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
				spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) << unk;
				spark->flags = SP_NONE;
				spark->maxYvel = 0;
				spark->friction = 34 << unk;
			}
		}
	}

	void LaserHeadCharge(ItemInfo* item)
	{
		auto* creature = GetLaserHeadInfo(item);
		byte size = item->ItemFlags[3];
		byte g = ((GetRandomControl() & 0x1F) + 128);
		byte b = ((GetRandomControl() & 0x1F) + 64);

		if (size <= 32)
		{
			g = (size * g) / 32;
			b = (size * b) / 32;
		}
		else
			size = 32;

		Vector3i origin;
		auto target = GetJointPosition(&g_Level.Items[creature->BaseItem], 0, LaserHeadBasePosition);

		for (int i = 0; i < LASERHEAD_CHARGE_ARCS_COUNT; i++)
		{
			auto* arc = creature->chargeArcs[i];

			if (item->ItemFlags[3] & 15 && arc != nullptr)
			{
				arc->r = b;
				arc->g = g;
				arc->b = 0;
				arc->life = 50;
				continue;
			}

			origin = GetJointPosition(&g_Level.Items[creature->BaseItem], 0, GuardianChargePositions[i]);
		//creature->chargeArcs[i] = TriggerLightning(&origin, &target, Random::GenerateInt(8, 16), 0, 128, 64, 16, (LI_SPLINE | LI_THINOUT | LI_THININ), 6, 5);
			//creature->chargeArcs[i] = TriggerEnergyArc(&origin, &target, 0, g, b, 256, 90, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE); //  (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x240000) << 8), 13, 48, 3);
		}

		if (GlobalCounter & 1)
		{
			for (int i = 0; i < LASERHEAD_FIRE_ARCS_COUNT; i++)
			{
				if (1 << GuardianMeshes[i] & item->MeshBits.ToPackedBits())
				{
					origin = GetJointPosition(item, GuardianMeshes[i]);
					TriggerLightningGlow(origin.x, origin.y, origin.z, size + (GetRandomControl() & 3), 0, g, b);
					TriggerLaserHeadSparks(&origin, 3, 0, g, b, 0);
				}
			}

			TriggerLightningGlow(target.x, target.y, target.z, (GetRandomControl() & 3) + size + 8, 0, g, b);
			TriggerDynamicLight(target.x, target.y, target.z, (GetRandomControl() & 3) + 16, 0, g, b);
		}

		if (!(GlobalCounter & 3))
		{
		//TriggerLightning(&target, (Vector3i*)&item->Pose.Position, Random::GenerateInt(8, 16), 0, 128, 64, 16, (LI_SPLINE | LI_THINOUT | LI_THININ), 6, 5);
			//TriggerEnergyArc(&target, (Vector3i*)&item->pos, 0, g, b, 256, 3, 64, ENERGY_ARC_NO_RANDOMIZE, ENERGY_ARC_STRAIGHT_LINE);
			//TriggerEnergyArc(&target, &item->pos, (GetRandomControl() & 7) + 8, v4 | ((v1 | 0x180000) << 8), 13, 64, 3);
		}

		TriggerLaserHeadSparks(&target, 3, 0, g, b, 1);
	}

	void InitialiseLaserHead(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->Collidable = true;
		item->StartPose = item->Pose;
		item->Data = LaserHeadInfo();

		auto* creature = GetLaserHeadInfo(item);

		for (int i = 0; i < g_Level.NumItems; i++)
		{
			if (g_Level.Items[i].ObjectNumber == ID_LASERHEAD_BASE)
			{
				creature->BaseItem = i;
				break;
			}
		}

		short rotation = 0;
		for (int j = 0; j < 8; j++)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				if (g_Level.Items[i].ObjectNumber == ID_LASERHEAD_TENTACLE && g_Level.Items[i].Pose.Orientation.y == rotation)
				{
					creature->Tentacles[j] = i;
					break;
				}
			}

			rotation += ANGLE(45.0f);
		}

		for (int i = 0; i < g_Level.NumItems; i++)
		{
			if (g_Level.Items[i].ObjectNumber == ID_PUZZLE_ITEM4)
			{
				creature->PuzzleItem = i;
				break;
			}
		}

		int y = item->Pose.Position.y - 640;
		item->Pose.Position.y = y;
		item->Animation.ActiveState = 0;
		item->ItemFlags[1] = y - 640;
		item->ItemFlags[3] = 90;
	}

	void LaserHeadControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetLaserHeadInfo(item);

		auto origin = GameVector::Zero;
		auto target = GameVector::Zero;

		// NOTICE: itemFlags[0] seems to be a state machine, if it's equal to 3 then death animations is triggered
		// Other values still unknown

		if (!item->ItemFlags[0])
		{
			if (item->ItemFlags[2] < 8)
			{
				if (!(GlobalCounter & 7))
				{
					short tentacleItemNumber = creature->Tentacles[item->ItemFlags[2]];
					auto* tentacleItem = &g_Level.Items[tentacleItemNumber];
					AddActiveItem(tentacleItemNumber);
					tentacleItem->Status = ITEM_ACTIVE;
					tentacleItem->Flags |= IFLAG_ACTIVATION_MASK;
					item->ItemFlags[2]++;
				}
			}
			else
			{
				if (item->Pose.Position.y > item->ItemFlags[1])
				{
					item->Animation.Velocity.y += 3.0f;

					if (item->Animation.Velocity.y > 32.0f)
						item->Animation.Velocity.y = 32.0f;

					item->Pose.Position.y -= item->Animation.Velocity.y;
				}
				else
				{
					auto origin = GameVector(GetJointPosition(item, 0, Vector3i(0, 168, 248)), item->RoomNumber);
					origin.RoomNumber = item->RoomNumber;
					auto target = GameVector(GetJointPosition(LaraItem, LM_HEAD, Vector3i(0, 0, 0)), LaraItem->RoomNumber);
					target.RoomNumber = LaraItem->RoomNumber;

					if (LOS(&origin, &target))
					{
						item->ItemFlags[0]++;
						item->ItemFlags[1] = item->Pose.Position.y;
						item->ItemFlags[2] = 2640;
					}
				}
			}
		}
		else if (item->ItemFlags[0] <= 2)
		{
			item->TriggerFlags++;
			item->Pose.Position.y = item->ItemFlags[1] - (128 * phd_sin(item->ItemFlags[2]));
			item->ItemFlags[2] += ANGLE(3.0f);

			// Get guardian head's position.
			auto origin = GameVector(GetJointPosition(item, 0, Vector3i(0, 168, 248)), item->RoomNumber);
			origin.RoomNumber = item->RoomNumber;
			if (item->ItemFlags[0] == 1)
			{
				// Get Lara's head position
				auto target = GameVector(GetJointPosition(LaraItem, LM_HEAD, Vector3i::Zero), LaraItem->RoomNumber);
				target.RoomNumber = LaraItem->RoomNumber;
				// Calculate distance between guardian and Lara
				int distance = sqrt(SQUARE(origin.x - target.x) + SQUARE(origin.y - target.y) + SQUARE(origin.z - target.z));

				// Check if there's a valid LOS between guardian and Lara 
				// and if distance is less than 8 sectors  and if Lara is alive and not burning
				if (LOS(&origin, &target) &&
					distance <= MAX_VISIBILITY_DISTANCE &&
					LaraItem->HitPoints > 0 &&
					LaraItem->Effect.Type != EffectType::Fire &&
					(creature->target.x || creature->target.y || creature->target.z))
				{
					// Lock target for attacking.
					target = GameVector(GetJointPosition(LaraItem, LM_HIPS), target.RoomNumber);
					target.RoomNumber = LaraItem->RoomNumber;
					creature->target.x = target.x;
					creature->target.y = target.y;
					creature->target.z = target.z;
					creature->trackSpeed = 3;
					creature->trackLara = 1;
				}
				else
				{
					// Randomly turn head try to finding Lara
					bool condition = !(GetRandomControl() & 0x7F) && item->TriggerFlags > 150;
					item->ItemFlags[3]--;
					if (item->ItemFlags[3] <= 0 || condition)
					{
						short xRot = (GetRandomControl() / 4) - ANGLE(22.5f);
						short yRot;
						if (condition)
							yRot = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) + ANGLE(135.0f);
						else
							yRot = 2 * GetRandomControl();
						int v = ((GetRandomControl() & 0x1FFF) + 8192);
						int c = v * phd_cos(-xRot);
						target.x = origin.x + c * phd_sin(yRot);
						target.y = origin.y + v * phd_sin(-xRot);
						target.z = origin.z + c * phd_cos(yRot);

						if (condition)
						{
							creature->trackSpeed = 2;
							item->TriggerFlags = 0;
						}
						else
							creature->trackSpeed = (GetRandomControl() & 2) + 3;

						item->ItemFlags[3] = creature->trackSpeed * ((GetRandomControl() & 3) + 8);

						creature->target.x = target.x;
						creature->target.y = target.y;
						creature->target.z = target.z;
					}
					else
					{
						target.x = creature->target.x;
						target.y = creature->target.y;
						target.z = creature->target.z;
					}

					creature->trackLara = 0;
				}
			}
			else
			{
				creature->trackSpeed = 3;

				if (JustLoaded)
				{
					int c = phd_cos(item->Pose.Orientation.x + 3328) * ANGLE(45);

					target.x = creature->target.x = origin.x + c * phd_sin(item->Pose.Orientation.y);
					target.y = creature->target.y = origin.y + SECTOR(8) * phd_sin(3328 - item->Pose.Orientation.x);
					target.z = creature->target.z = origin.z + c * phd_cos(item->Pose.Orientation.y);
				}
				else
				{
					target.x = creature->target.x;
					target.y = creature->target.y;
					target.z = creature->target.z;
				}
			}

			auto angles = Geometry::GetOrientToPoint(origin.ToVector3(), creature->target.ToVector3());
			InterpolateAngle(angles.x + 3328, item->Pose.Orientation.x, creature->xRot, creature->trackSpeed);
			InterpolateAngle(angles.y, item->Pose.Orientation.y, creature->yRot, creature->trackSpeed);

			if (item->ItemFlags[0] == 1)
			{
				if (creature->trackLara)
				{
					if (!(GetRandomControl() & 0x1F) &&
						abs(creature->xRot) < ANGLE(5.6f) &&
						abs(creature->yRot) < ANGLE(5.6f) &&
						!LaraItem->Animation.Velocity.y ||
						!(GetRandomControl() & 0x1FF))
					{
						item->ItemFlags[0]++;
						item->ItemFlags[3] = 0;
					}
				}
				else if (!(GetRandomControl() & 0x3F) && item->TriggerFlags > 300)
				{
					item->ItemFlags[0]++;
					item->TriggerFlags = 0;
					item->ItemFlags[3] = 0;
				}
			}
			else
			{
				if (item->ItemFlags[3] <= 90)
				{
					SoundEffect(SFX_TR5_GOD_HEAD_CHARGE, &item->Pose);
					LaserHeadCharge(item);
					item->ItemFlags[3]++;
				}

				if (item->ItemFlags[3] >= 90)
				{
					byte r = 0;
					byte g = (GetRandomControl() & 0x1F) + 128;
					byte b = (GetRandomControl() & 0x1F) + 64;

					auto* arc = creature->fireArcs[0];
					if (creature->fireArcs[0] == nullptr)
						arc = creature->fireArcs[1];

					if ((item->ItemFlags[3] <= 90 ||
						!arc ||
						arc->life) &&
						LaraItem->HitPoints > 0 &&
						LaraItem->Effect.Type == EffectType::None)
					{
						if (item->ItemFlags[3] > 90 &&
							arc &&
							arc->life < 16)
						{
							g = (arc->life * g)  / 16;
							b = (arc->life * b)  / 16;
						}

						for (int i = 0; i < LASERHEAD_FIRE_ARCS_COUNT; i++)
						{
							// If eye was not destroyed then fire from it
							//TENLog("Eyes flag: " + std::to_string(!(1 << GuardianMeshes[i] & item->MeshBits.ToPackedBits())));
							if (!(1 << GuardianMeshes[i] & item->MeshBits.ToPackedBits()))
							{
								if (item->ItemFlags[3] > 90 &&
									creature->fireArcs[i])
								{
									creature->fireArcs[i]->life = 0;
									creature->fireArcs[i] = nullptr;
								}
							}
							else
							{
								GameVector origin1 = GetJointPosition(item, GuardianMeshes[i], Vector3i::Zero);
								origin1.RoomNumber = item->RoomNumber;
								GameVector eye;// = GameVector(0, 0, 0, LaraItem->RoomNumber);
								eye.RoomNumber = item->RoomNumber;
								int c = ANGLE(45.0f) * phd_cos(angles.x);
								eye.x = origin1.x + (c * phd_sin(item->Pose.Orientation.y));
								eye.y = origin1.y + phd_sin(-angles.x) * ANGLE(45);
								eye.z = origin1.z + (c * phd_cos(item->Pose.Orientation.y));

								if (item->ItemFlags[3] != 90 && creature->fireArcs[i])
								{
									// Eye is aready firing.
									SoundEffect(SFX_TR5_GOD_HEAD_LASER_LOOPS, &item->Pose);

									creature->fireArcs[i]->pos1.x =  origin1.x;
									creature->fireArcs[i]->pos1.y =  origin1.y; 
									creature->fireArcs[i]->pos1.z =  origin1.z;

									if (creature->fireArcs[i]->life > 0)
										creature->fireArcs[i]->life -= 2;
									else
									{
										creature->fireArcs[i]->life = 0;
										//creature->fireArcs[i] = nullptr;
										item->ItemFlags[0] = 1;
										item->TriggerFlags = 0;

									}
								}
								else
								{
									// Start firing from eye
									origin1.RoomNumber = item->RoomNumber;
									creature->LOS[i] = LOS(&origin1, &eye);
									creature->fireArcs[i] = TriggerLightning((Vector3i*)&origin1, (Vector3i*)&eye, Random::GenerateInt(8, 16), r, 128, 64, 46, ( LI_THININ | LI_SPLINE | LI_THINOUT), 6, 10);  // NOTE: TriggerLightning does not support ``return arc``
									StopSoundEffect(SFX_TR5_GOD_HEAD_CHARGE);
									SoundEffect(SFX_TR5_GOD_HEAD_BLAST, &item->Pose);
								}

								auto* currentArc = creature->fireArcs[i];

								if (GlobalCounter & 1)
								{
									TriggerLaserHeadSparks((Vector3i*)&origin1, 3, r, g, b, 0);
									TriggerLightningGlow(origin1.x, origin1.y, origin1.z, (GetRandomControl() & 3) + 32, r, g, b);
									TriggerDynamicLight(origin1.x, origin1.y, origin1.z, (GetRandomControl() & 3) + 16, r, g, b);

									if (!creature->LOS[i])
									{
										TriggerLightningGlow(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 16, r, g, b);
										TriggerDynamicLight(currentArc->pos4.x, currentArc->pos4.y, currentArc->pos4.z, (GetRandomControl() & 3) + 6, r, g, b);
										TriggerLaserHeadSparks((Vector3i*)&currentArc->pos4, 3, r, g, b, 0);
									}
								}

								// Check if Lara was hit by energy arcs
								if (LaraItem->Effect.Type == EffectType::None)
								{
									
									GetLaraDeadlyBounds();


									int xc = LaraItem->Pose.Position.x + ((DeadlyBounds.X1 + DeadlyBounds.X2) / 2) - target.x;
									int yc = LaraItem->Pose.Position.y + ((DeadlyBounds.Y1 + DeadlyBounds.Y2) / 2) - target.y;
									int zc = LaraItem->Pose.Position.z + ((DeadlyBounds.Z1 + DeadlyBounds.Z2) / 2) - target.z;
									

									int distance = sqrt(SQUARE(xc - origin1.x) + SQUARE(yc - origin1.y) + SQUARE(zc - origin1.z));
									if (distance < MAX_VISIBILITY_DISTANCE)
									{
										//int dl = 
											distance += CLICK(2);
										if (distance < MAX_VISIBILITY_DISTANCE)
										{
											eye.x = origin1.x + (eye.x - origin1.x) * distance / MAX_VISIBILITY_DISTANCE;
											eye.y = origin1.y + (eye.y - origin1.y) * distance / MAX_VISIBILITY_DISTANCE;
											eye.z = origin1.z + (eye.z - origin1.z) * distance / MAX_VISIBILITY_DISTANCE;
										}

										int dx = (eye.x - origin1.x) / 32;
										int dy = (eye.y - origin1.y) / 32;
										int dz = (eye.z - origin1.z) / 32;
										int adx = currentArc->pos4.x - origin1.x;
										int ady = currentArc->pos4.y - origin1.y;
										int adz = currentArc->pos4.z - origin1.z;
										int x = origin1.x;
										int y = origin1.y;
										int z = origin1.z;


										int someIndex = 0;
										for (int j = 0; j < 32; j++)
										{
											if (someIndex)
											{
												someIndex--;
												if (!someIndex)
													break;
											}

											if (abs(adx) < 280 && abs(ady) < 280 && abs(adz) < 280)
												someIndex = 2;

											if (x > DeadlyBounds.X1 && x < DeadlyBounds.X2 &&
												y > DeadlyBounds.Y1 && y < DeadlyBounds.Y2 &&
												z > DeadlyBounds.Z1 && z < DeadlyBounds.Z2)
											{
												if (LaraItem->Effect.Type != EffectType::Smoke)
												{
													ItemCustomBurn(LaraItem, Vector3(0.0f, 0.8f, 0.1f), Vector3(0.0f, 0.9f, 0.8f), 2 * FPS);
												}
												else
													ItemSmoke(LaraItem, -1);

												DoDamage(LaraItem, INT_MAX);
												break;
											}

											x += dx;
											y += dy;
											z += dz;

											adx -= dx;
											ady -= dy;
											adz -= dz;
										}
									}
								}
							}
				
						}
					}
					else
					{
						if (arc)
						{
							creature->fireArcs[0] = nullptr;
							creature->fireArcs[1] = nullptr;
						}

						item->ItemFlags[0] = 1;
						item->TriggerFlags = 0;
					}
				}
			}
		}
		else
		{
			// Maybe number of eye hits?
			if (!(GlobalCounter & 7) && item->Animation.ActiveState < 8)
			{
				short tentacleNumber = creature->Tentacles[item->Animation.ActiveState];
				g_Level.Items[tentacleNumber].Animation.TargetState = 2;
				item->Animation.ActiveState++;
			}
			// Destroy tentacle items
			if (item->Animation.ActiveState > 0)
			{
				for (int i = 0; i < 8; i++)
				{
					auto* tentacleItem = &g_Level.Items[creature->Tentacles[i]];

					if (tentacleItem->Animation.AnimNumber == Objects[tentacleItem->ObjectNumber].animIndex + 1 &&
						tentacleItem->Animation.FrameNumber == g_Level.Anims[tentacleItem->Animation.AnimNumber].frameEnd &&
						tentacleItem->MeshBits & 1)
					{
						SoundEffect(SFX_TR5_SMASH_ROCK2, &item->Pose);
						ExplodeItemNode(tentacleItem, 0, 0, 128);
						KillItem(creature->Tentacles[i]);
					}
				}
			}

			item->Pose.Position.y = item->ItemFlags[1] - (192 - item->Animation.Velocity.z) * phd_sin(item->ItemFlags[2]);
			item->ItemFlags[2] += ANGLE(item->Animation.Velocity.z);

			if (!(GlobalCounter & 7))
			{
				item->ItemFlags[3] = item->Pose.Orientation.y + (GetRandomControl() & 0x3FFF) - 4096;
				item->TriggerFlags = (GetRandomControl() & 0x1000) - 2048;
			}

			short temp = 0;
			InterpolateAngle(item->ItemFlags[3], item->Pose.Orientation.y, temp, 2);
			temp = 0;
			InterpolateAngle(item->TriggerFlags, item->Pose.Orientation.x, temp, 2);

			// Final death
			item->Animation.Velocity.z++;
			if (item->Animation.Velocity.z > 136)
			{
				ExplodeItemNode(&g_Level.Items[creature->BaseItem], 0, 0, 128);
				KillItem(creature->BaseItem);

				ExplodeItemNode(item, 0, 0, 128);

				item->Pose.Position.y -= 256;
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -2, 2, item->RoomNumber);
				TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 2, 0, 2, item->RoomNumber);

				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0, 0);
				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0x3000, 0);
				TriggerShockwave(&item->Pose, 32, 160, 64, 64, 128, 0, 36, 0x6000, 0);

				g_Level.Items[creature->PuzzleItem].Pose.Position.y = item->Pose.Position.y;
				TestTriggers(item, true);

				SoundEffect(SFX_TR5_GOD_HEAD_BLAST, &item->Pose, SoundEnvironment::Land, 0.5f);
				SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose, SoundEnvironment::Land, 1.25f);
				SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose);
				SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 0.25f);

				KillItem(itemNumber);
			}
		}

		if (item->ItemFlags[0] < 3)
		{
			int i = 0;
			for (i = 0; i < 8; i++)
			{
				short tentacleItemNumber = creature->Tentacles[i];
				auto* tentacleItem = &g_Level.Items[tentacleItemNumber];

				if (tentacleItem->Animation.AnimNumber == Objects[tentacleItem->ObjectNumber].animIndex &&
					tentacleItem->Animation.FrameNumber != g_Level.Anims[tentacleItem->Animation.AnimNumber].frameEnd)
				{
					break;
				}
			}

			// If all tentacles animations are done and both eyes are destroyed it's time to die
			if (i == 8 && !(item->MeshBits & 6))
			{
				if (creature->fireArcs[0])
					creature->fireArcs[0]->life = 2;
				if (creature->fireArcs[1])
					creature->fireArcs[1]->life = 2;

				creature->fireArcs[0] = nullptr;
				creature->fireArcs[1] = nullptr;

				item->Animation.Velocity.z = 3.0f;
				item->ItemFlags[0] = 3;
				item->ItemFlags[3] = item->Pose.Orientation.y + (GetRandomControl() & 0x1000) - 2048;
				item->TriggerFlags = item->Pose.Orientation.x + (GetRandomControl() & 0x1000) - 2048;
			}
		}
	}
}

