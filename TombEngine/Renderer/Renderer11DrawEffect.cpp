#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/Blood.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/debris.h"
#include "Game/effects/Drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/explosion.h"
#include "Game/effects/Footprint.h"
#include "Game/effects/lightning.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Quad/RenderQuad.h"
#include "Renderer/RendererSprites.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Ripple;
using namespace TEN::Math;

extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];
extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH]; // offset 0xA31D8
extern Particle Particles[MAX_PARTICLES];
extern SPLASH_STRUCT Splashes[MAX_SPLASHES];

// TODO: EnemyBites must be eradicated and kept directly in object structs or passed to gunflash functions!

BiteInfo EnemyBites[12] =
{
	{ 20, -95, 240, 13 },
	{ 48, 0, 180, -11 },
	{ -48, 0, 180, 14 },
	{ -48, 5, 225, 14 },
	{ 15, -60, 195, 13 },
	{ -30, -65, 250, 18 },
	{ 0, -110, 480, 13 },
	{ -20, -80, 190, -10 },
	{ 10, -60, 200, 13 },
	{ 10, -60, 200, 11 },   // Baddy 2
	{ 20, -60, 400, 7 },    // SAS
	{ 0, -64, 250, 7 }      // Troops
};

namespace TEN::Renderer
{
	using namespace TEN::Effects::Blood;
	using namespace TEN::Effects::Footprint;
	using std::vector;

	struct RendererSpriteBucket
	{
		RendererSprite* Sprite;
		BLEND_MODES BlendMode;
		std::vector<RendererSpriteToDraw> SpritesToDraw;
		bool IsBillboard;
		bool IsSoftParticle;
	};

	void Renderer11::DrawLightning(RenderView& view)
	{
		for (int i = 0; i < Lightning.size(); i++)
		{
			LIGHTNING_INFO* arc = &Lightning[i];

			if (arc->life)
			{
				LightningPos[0].x = arc->pos1.x;
				LightningPos[0].y = arc->pos1.y;
				LightningPos[0].z = arc->pos1.z;

				memcpy(&LightningPos[1], arc, 48);

				LightningPos[5].x = arc->pos4.x;
				LightningPos[5].y = arc->pos4.y;
				LightningPos[5].z = arc->pos4.z;

				for (int j = 0; j < 6; j++)
				{
					LightningPos[j].x -= LaraItem->Pose.Position.x;
					LightningPos[j].y -= LaraItem->Pose.Position.y;
					LightningPos[j].z -= LaraItem->Pose.Position.z;
				}

				CalcLightningSpline(&LightningPos[0], LightningBuffer, arc);

				if (abs(LightningPos[0].x) <= 24576 && abs(LightningPos[0].y) <= 24576 && abs(LightningPos[0].z) <= 24576)
				{
					short* interpolatedPos = &LightningBuffer[0];

					for (int s = 0; s < 3 * arc->segments - 1; s++)
					{
						int ix = LaraItem->Pose.Position.x + interpolatedPos[0];
						int iy = LaraItem->Pose.Position.y + interpolatedPos[1];
						int iz = LaraItem->Pose.Position.z + interpolatedPos[2];

						interpolatedPos += 4;

						int ix2 = LaraItem->Pose.Position.x + interpolatedPos[0];
						int iy2 = LaraItem->Pose.Position.y + interpolatedPos[1];
						int iz2 = LaraItem->Pose.Position.z + interpolatedPos[2];

						byte r, g, b;

						if (arc->life >= 16)
						{
							r = arc->r;
							g = arc->g;
							b = arc->b;
						}
						else
						{
							r = arc->life * arc->r / 16;
							g = arc->life * arc->g / 16;
							b = arc->life * arc->b / 16;
						}

						Vector3 pos1 = Vector3(ix, iy, iz);
						Vector3 pos2 = Vector3(ix2, iy2, iz2);

						Vector3 d = pos2 - pos1;
						d.Normalize();

						Vector3 c = (pos1 + pos2) / 2.0f;

						AddSpriteBillboardConstrained(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LIGHTHING],
							c,
							Vector4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f),
							(PI / 2),
							1.0f,
							{ arc->width * 8.0f,
							Vector3::Distance(pos1, pos2) },
							BLENDMODE_ADDITIVE,
							d, true, view);
					}
				}
			}
		}
	}

	void Renderer11::DrawSmokes(RenderView& view)
	{
		for (int i = 0; i < 32; i++)
		{
			SMOKE_SPARKS* spark = &SmokeSparks[i];

			if (spark->on)
			{
				AddSpriteBillboard(&m_sprites[spark->def],
					Vector3(spark->x, spark->y, spark->z),
					Vector4(spark->shade / 255.0f, spark->shade / 255.0f, spark->shade / 255.0f, 1.0f),
					TO_RAD(spark->rotAng << 4), spark->scalar, { spark->size * 4.0f, spark->size * 4.0f },
					BLENDMODE_ADDITIVE, true, view);
			}
		}
	}


	void Renderer11::DrawFires(RenderView& view)
	{
		for (int k = 0; k < MAX_FIRE_LIST; k++)
		{
			auto* fire = &Fires[k];
			if (fire->on)
			{
				auto fade = fire->on == 1 ? 1.0f : (float)(255 - fire->on) / 255.0f;

				for (int i = 0; i < MAX_SPARKS_FIRE; i++)
				{
					auto* spark = &FireSparks[i];
					if (spark->on)
					{
						AddSpriteBillboard(
							&m_sprites[spark->def],
							Vector3(fire->x + spark->x * fire->size / 2, fire->y + spark->y * fire->size / 2, fire->z + spark->z * fire->size / 2),
							Vector4(spark->r / 255.0f * fade, spark->g / 255.0f * fade, spark->b / 255.0f * fade, 1.0f),
							TO_RAD(spark->rotAng << 4),
							spark->scalar,
							Vector2(spark->size * fire->size, spark->size * fire->size), BLENDMODE_ADDITIVE, true, view);
					}
				}
			}
		}
	}

	void Renderer11::DrawParticles(RenderView& view)
	{
		for (int i = 0; i < MAX_NODE; i++)
			NodeOffsets[i].gotIt = false;

		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			auto particle = &Particles[i];
			if (particle->on)
			{
				if (particle->flags & SP_DEF)
				{
					auto pos = Vector3(particle->x, particle->y, particle->z);

					if (particle->flags & SP_FX)
					{
						auto* fx = &EffectList[particle->fxObj];

						pos.x += fx->pos.Position.x;
						pos.y += fx->pos.Position.y;
						pos.z += fx->pos.Position.z;

						if ((particle->sLife - particle->life) > (rand() & 7) + 4)
						{
							particle->flags &= ~SP_FX;
							particle->x = pos.x;
							particle->y = pos.y;
							particle->z = pos.z;
						}
					}
					else if (!(particle->flags & SP_ITEM))
					{
						pos.x = particle->x;
						pos.y = particle->y;
						pos.z = particle->z;
					}
					else
					{
						auto* item = &g_Level.Items[particle->fxObj];

						auto nodePos = Vector3i::Zero;
						if (particle->flags & SP_NODEATTACH)
						{
							if (NodeOffsets[particle->nodeNumber].gotIt)
							{
								nodePos.x = NodeVectors[particle->nodeNumber].x;
								nodePos.y = NodeVectors[particle->nodeNumber].y;
								nodePos.z = NodeVectors[particle->nodeNumber].z;
							}
							else
							{
								nodePos.x = NodeOffsets[particle->nodeNumber].x;
								nodePos.y = NodeOffsets[particle->nodeNumber].y;
								nodePos.z = NodeOffsets[particle->nodeNumber].z;

								int meshIndex = NodeOffsets[particle->nodeNumber].meshNum;
								if (meshIndex >= 0)
									nodePos = GetJointPosition(item, meshIndex, nodePos);
								else
									nodePos = GetJointPosition(LaraItem, -meshIndex, nodePos);

								NodeOffsets[particle->nodeNumber].gotIt = true;

								NodeVectors[particle->nodeNumber].x = nodePos.x;
								NodeVectors[particle->nodeNumber].y = nodePos.y;
								NodeVectors[particle->nodeNumber].z = nodePos.z;
							}

							pos += nodePos.ToVector3();

							if ((particle->sLife - particle->life) > ((rand() & 3) + 8))
							{
								particle->flags &= ~SP_ITEM;
								particle->x = pos.x;
								particle->y = pos.y;
								particle->z = pos.z;
							}
						}
						else
							pos += item->Pose.Position.ToVector3();
					}

					// Don't allow sprites out of bounds.
					int spriteIndex = std::clamp(int(particle->spriteIndex), 0, int(m_sprites.size()));

					AddSpriteBillboard(
						&m_sprites[spriteIndex],
						pos,
						Vector4(particle->r / 255.0f, particle->g / 255.0f, particle->b / 255.0f, 1.0f),
						TO_RAD(particle->rotAng << 4), particle->scalar,
						Vector2(particle->size, particle->size),
						particle->blendMode, true, view);
				}
				else
				{
					auto pos = Vector3(particle->x, particle->y, particle->z);
					auto v = Vector3(particle->xVel, particle->yVel, particle->zVel);
					v.Normalize();
					AddSpriteBillboardConstrained(
						&m_sprites[Objects[ID_SPARK_SPRITE].meshIndex],
						pos,
						Vector4(particle->r / 255.0f, particle->g / 255.0f, particle->b / 255.0f, 1.0f),
						TO_RAD(particle->rotAng << 4),
						particle->scalar,
						Vector2(4, particle->size), particle->blendMode, v, true, view);
				}
			}
		}
	}

	void Renderer11::DrawSplashes(RenderView& view)
	{
		constexpr size_t NUM_POINTS = 9;

		for (int i = 0; i < MAX_SPLASHES; i++)
		{
			auto& splash = Splashes[i];

			if (splash.isActive)
			{
				constexpr float alpha = 360 / NUM_POINTS;
				byte color = (splash.life >= 32 ? 128 : (byte)((splash.life / 32.0f) * 128));

				if (!splash.isRipple)
				{
					if (splash.heightSpeed < 0 && splash.height < 1024)
					{
						float multiplier = splash.height / 1024.0f;
						color = (float)color * multiplier;
					}
				}

				float innerRadius = splash.innerRad;
				float outerRadius = splash.outerRad;
				float yInner = splash.y;
				float yOuter = splash.y - splash.height;

				for (int i = 0; i < NUM_POINTS; i++)
				{
					float xInner = innerRadius * sin(alpha * i * RADIAN);
					float zInner = innerRadius * cos(alpha * i * RADIAN);
					float xOuter = outerRadius * sin(alpha * i * RADIAN);
					float zOuter = outerRadius * cos(alpha * i * RADIAN);
					xInner += splash.x;
					zInner += splash.z;
					xOuter += splash.x;
					zOuter += splash.z;
					int j = (i + 1) % NUM_POINTS;
					float x2Inner = innerRadius * sin(alpha * j * RADIAN);
					x2Inner += splash.x;
					float z2Inner = innerRadius * cos(alpha * j * RADIAN);
					z2Inner += splash.z;
					float x2Outer = outerRadius * sin(alpha * j * RADIAN);
					x2Outer += splash.x;
					float z2Outer = outerRadius * cos(alpha * j * RADIAN);
					z2Outer += splash.z;

					AddSprite3D(
						&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + splash.spriteSequenceStart + (int)splash.animationPhase],
						Vector3(xOuter, yOuter, zOuter),
						Vector3(x2Outer, yOuter, z2Outer),
						Vector3(x2Inner, yInner, z2Inner),
						Vector3(xInner, yInner, zInner),
						Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f),
						0, 1, { 0, 0 }, BLENDMODE_ADDITIVE, false, view);
				}
			}
		}
	}

	void Renderer11::DrawBubbles(RenderView& view)
	{
		for (const auto& bubble : Bubbles)
		{
			if (!bubble.IsActive)
				continue;

			AddSpriteBillboard(
				&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + bubble.SpriteIndex],
				bubble.Position,
				bubble.Color, bubble.Rotation, 1.0f, Vector2(bubble.Scale, bubble.Scale) / 2, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawRipples(RenderView& view)
	{
		for (const auto& ripple : Ripples)
		{
			if (!ripple.IsActive)
				continue;

			auto color = Vector4::Zero;
			if (ripple.Flags.Test(RippleFlags::LowOpacity))
			{
				if (ripple.Init)
					color = Vector4(ripple.Init, ripple.Init, ripple.Init, 255);
				else
					color = Vector4(ripple.Life, ripple.Life, ripple.Life, 255);
			}
			else
			{
				if (ripple.Init)
					color = Vector4(ripple.Init * 2, ripple.Init * 2, ripple.Init * 2, 255);
				else
					color = Vector4(ripple.Life * 2, ripple.Life * 2, ripple.Life * 2, 255);
			}

			color.x = (int)std::clamp((int)color.x, 0, 255);
			color.y = (int)std::clamp((int)color.y, 0, 255);
			color.z = (int)std::clamp((int)color.z, 0, 255);
			color /= 255.0f;

			AddSpriteBillboardConstrainedLookAt(
				&m_sprites[ripple.SpriteIndex],
				ripple.Position,
				color, 0.0f, 1.0f, Vector2(ripple.Scale * 2), BLENDMODE_ADDITIVE, ripple.Normal, true, view);
		}
	}

	void Renderer11::DrawShockwaves(RenderView& view)
	{
		for (int i = 0; i < MAX_SHOCKWAVE; i++)
		{
			SHOCKWAVE_STRUCT* shockwave = &ShockWaves[i];

			if (shockwave->life)
			{
				byte color = shockwave->life * 8;

				int dl = shockwave->outerRad - shockwave->innerRad;
				Matrix rotationMatrix = Matrix::CreateRotationX(TO_RAD(shockwave->xRot));
				Vector3 pos = Vector3(shockwave->x, shockwave->y, shockwave->z);

				// Inner circle
				float angle = PI / 32.0f;
				float c = cos(angle);
				float s = sin(angle);
				float x1 = (shockwave->innerRad * c);
				float z1 = (shockwave->innerRad * s);
				float x4 = (shockwave->outerRad * c);
				float z4 = (shockwave->outerRad * s);
				angle -= PI / 8.0f;

				Vector3 p1 = Vector3(x1, 0, z1);
				Vector3 p4 = Vector3(x4, 0, z4);

				p1 = Vector3::Transform(p1, rotationMatrix);
				p4 = Vector3::Transform(p4, rotationMatrix);

				for (int j = 0; j < 16; j++)
				{
					c = cos(angle);
					s = sin(angle);
					float x2 = (shockwave->innerRad * c);
					float z2 = (shockwave->innerRad * s);
					float x3 = (shockwave->outerRad * c);
					float z3 = (shockwave->outerRad * s);
					angle -= PI / 8.0f;

					Vector3 p2 = Vector3(x2, 0, z2);
					Vector3 p3 = Vector3(x3, 0, z3);

					p2 = Vector3::Transform(p2, rotationMatrix);
					p3 = Vector3::Transform(p3, rotationMatrix);

					AddSprite3D(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH],
						pos + p1,
						pos + p2,
						pos + p3,
						pos + p4,
						Vector4(
							shockwave->r * shockwave->life / 255.0f / 16.0f,
							shockwave->g * shockwave->life / 255.0f / 16.0f,
							shockwave->b * shockwave->life / 255.0f / 16.0f,
							1.0f),
						0, 1, { 0,0 }, BLENDMODE_ADDITIVE, false, view);

					p1 = p2;
					p4 = p3;
				}
			}
		}
	}

	void Renderer11::DrawWeatherParticles(RenderView& view)
	{
		for (auto& p : Weather.GetParticles())
		{
			if (!p.Enabled)
				continue;

			switch (p.Type)
			{
			case WeatherType::None:
				AddSpriteBillboard(
					&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST],
					p.Position,
					Vector4(1.0f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(p.Size),
					BLENDMODE_ADDITIVE, true, view);

				break;

			case WeatherType::Snow:
				AddSpriteBillboard(
					&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST],
					p.Position,
					Vector4(1.0f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(p.Size),
					BLENDMODE_ADDITIVE, true, view);

				break;

			case WeatherType::Rain:
				Vector3 v;
				p.Velocity.Normalize(v);

				AddSpriteBillboardConstrained(
					&m_sprites[Objects[ID_DRIP_SPRITE].meshIndex],
					p.Position,
					Vector4(0.8f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(TEN::Effects::Drip::DRIP_WIDTH, p.Size), BLENDMODE_ADDITIVE, -v, true, view);

				break;
			}
		}
	}

	bool Renderer11::DrawGunFlashes(RenderView& view)
	{
		if (!Lara.RightArm.GunFlash && !Lara.LeftArm.GunFlash)
			return true;

		if (BinocularRange > 0)
			return true;

		const auto& room = m_rooms[LaraItem->RoomNumber];
		auto& item = m_items[Lara.ItemNumber];

		m_stStatic.Color = Vector4::One;
		m_stStatic.AmbientLight = room.AmbientLight;
		m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_STATIC;

		BindLights(item.LightsToDraw); // FIXME: Is it really needed for gunflashes? -- Lwmte, 15.07.22

		short length = 0;
		short zOffset = 0;
		short rotationX = 0;

		SetBlendMode(BLENDMODE_ADDITIVE);
		SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

		if (Lara.Control.Weapon.GunType != LaraWeaponType::Flare &&
			Lara.Control.Weapon.GunType != LaraWeaponType::Shotgun &&
			Lara.Control.Weapon.GunType != LaraWeaponType::Crossbow)
		{
			switch (Lara.Control.Weapon.GunType)
			{
			case LaraWeaponType::Revolver:
				length = 192;
				zOffset = 68;
				rotationX = -14560;
				break;

			case LaraWeaponType::Uzi:
				length = 190;
				zOffset = 50;
				rotationX = -14560;
				break;

			case LaraWeaponType::HK:
				length = 300;
				zOffset = 92;
				rotationX = -14560;
				break;

			default:
			case LaraWeaponType::Pistol:
				length = 180;
				zOffset = 40;
				rotationX = -16830;
				break;
			}

			// Use MP5 flash if available
			auto gunflash = GAME_OBJECT_ID::ID_GUN_FLASH;
			if (Lara.Control.Weapon.GunType == LaraWeaponType::HK && Objects[GAME_OBJECT_ID::ID_GUN_FLASH2].loaded)
			{
				gunflash = GAME_OBJECT_ID::ID_GUN_FLASH2;
				length += 20;
				zOffset += 10;
			}

			ObjectInfo* flashObj = &Objects[gunflash];
			RendererObject& flashMoveable = *m_moveableObjects[gunflash];
			RendererMesh* flashMesh = flashMoveable.ObjectMeshes[0];

			for (auto& flashBucket : flashMesh->Buckets)
			{
				if (flashBucket.BlendMode == BLENDMODE_OPAQUE)
					continue;

				if (flashBucket.Polygons.size() == 0)
					continue;

				BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[flashBucket.Texture]), SAMPLER_ANISOTROPIC_CLAMP);

				Matrix offset = Matrix::CreateTranslation(0, length, zOffset);
				Matrix rotation = Matrix::CreateRotationX(TO_RAD(rotationX));

				Matrix world;

				if (Lara.LeftArm.GunFlash)
				{
					world = item.AnimationTransforms[LM_LHAND] * item.World;
					world = offset * world;
					world = rotation * world;

					m_stStatic.World = world;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}

				if (Lara.RightArm.GunFlash)
				{
					world = item.AnimationTransforms[LM_RHAND] * item.World;
					world = offset * world;
					world = rotation * world;

					m_stStatic.World = world;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}
			}
		}

		SetBlendMode(BLENDMODE_OPAQUE);

		return true;
	}

	void Renderer11::DrawBaddyGunflashes(RenderView& view)
	{
		for (auto room : view.roomsToDraw)
		{
			for (auto item : room->ItemsToDraw)
			{
				// Does the item need gunflash?
				ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];
				ObjectInfo* obj = &Objects[nativeItem->ObjectNumber];

				if (obj->biteOffset == -1)
					continue;

				if (nativeItem->Data.is<CreatureInfo>())
				{
					auto* creature = GetCreatureInfo(nativeItem);
					if (!creature->FiredWeapon)
						continue;
				}
				else
					continue;

				RendererRoom const& room = m_rooms[nativeItem->RoomNumber];
				RendererObject& flashMoveable = *m_moveableObjects[ID_GUN_FLASH];

				m_stStatic.Color = Vector4::One;
				m_stStatic.AmbientLight = room.AmbientLight;
				m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_STATIC;

				BindLights(item->LightsToDraw); // FIXME: Is it really needed for gunflashes? -- Lwmte, 15.07.22

				SetBlendMode(BLENDMODE_ADDITIVE);

				SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

				BiteInfo* bites[2] = {
					&EnemyBites[obj->biteOffset],
					&EnemyBites[obj->biteOffset + 1]
				};

				int numBites = (bites[0]->meshNum < 0) + 1;

				for (int k = 0; k < numBites; k++)
				{
					int joint = abs(bites[k]->meshNum);

					RendererMesh* flashMesh = flashMoveable.ObjectMeshes[0];

					for (auto& flashBucket : flashMesh->Buckets)
					{
						if (flashBucket.BlendMode == BLENDMODE_OPAQUE)
							continue;

						if (flashBucket.Polygons.size() == 0)
							continue;

						BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[flashBucket.Texture]), SAMPLER_ANISOTROPIC_CLAMP);

						Matrix offset = Matrix::CreateTranslation(bites[k]->Position);
						Matrix rotationX = Matrix::CreateRotationX(TO_RAD(ANGLE(270.0f)));
						Matrix rotationZ = Matrix::CreateRotationZ(TO_RAD(2 * GetRandomControl()));

						Matrix world = item->AnimationTransforms[joint] * item->World;
						world = rotationX * world;
						world = offset * world;
						world = rotationZ * world;

						m_stStatic.World = world;
						m_cbStatic.updateData(m_stStatic, m_context.Get());
						BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

						DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
					}
				}
			}
		}

		SetBlendMode(BLENDMODE_OPAQUE);
	}

	Texture2D Renderer11::CreateDefaultNormalTexture()
	{
		auto data = vector<byte>{ 128, 128, 255, 1 };
		return Texture2D(m_device.Get(), 1, 1, data.data());
	}

	void Renderer11::DrawBloodDrips(RenderView& view)
	{
		for (const auto& drip : BloodDrips)
		{
			if (!drip.IsActive)
				continue;

			if (drip.SpriteIndex >= g_Level.Sprites.size())
				continue;

			auto axis = -drip.Velocity;
			axis.Normalize();

			float width = drip.Scale * Random::GenerateFloat(1.0f, 3.0f);
			float height = drip.Scale * Random::GenerateFloat(4.0f, 12.0f);

			AddSpriteBillboardConstrained(
				&m_sprites[drip.SpriteIndex],
				drip.Position,
				drip.Color, 0.5f, 1.0f, Vector2(width, height), BLENDMODE_ALPHABLEND, axis, true, view);
		}
	}

	void Renderer11::DrawBloodMists(RenderView& view)
	{
		for (const auto& mist : BloodMists)
		{
			if (!mist.IsActive)
				continue;

			if (mist.SpriteIndex >= g_Level.Sprites.size())
				continue;

			AddSpriteBillboard(
				&m_sprites[mist.SpriteIndex],
				mist.Position,
				mist.Color, TO_RAD(mist.Orientation2D), 1.0f, Vector2(mist.Scale, mist.Scale), BLENDMODE_ALPHABLEND, true, view);
		}
	}

	void Renderer11::DrawBloodStains(RenderView& view)
	{
		for (const auto& stain : BloodStains)
		{
			if (stain.SpriteIndex >= g_Level.Sprites.size())
				continue;

			// TODO: Try setting soft particle to true.
			AddSprite3D(
				&m_sprites[stain.SpriteIndex],
				stain.VertexPoints[0], stain.VertexPoints[1], stain.VertexPoints[2], stain.VertexPoints[3],
				stain.Color, 0.0f, 1.0f, Vector2::One, BLENDMODE_ALPHABLEND, true, view);
		}
	}

	void Renderer11::DrawUnderwaterBloodParticles(RenderView& view)
	{
		for (const auto& uwBlood : UnderwaterBloodParticles)
		{
			if (!uwBlood.IsActive)
				continue;

			auto color = Vector4::Zero;
			if (uwBlood.Init)
				color = Vector4(uwBlood.Init / 2, 0, uwBlood.Init / 16, 255);
			else
				color = Vector4(uwBlood.Life / 2, 0, uwBlood.Life / 16, 255);
			
			color.x = (int)std::clamp((int)color.x, 0, 255);
			color.y = (int)std::clamp((int)color.y, 0, 255);
			color.z = (int)std::clamp((int)color.z, 0, 255);
			color /= 255.0f;

			AddSpriteBillboard(
				&m_sprites[uwBlood.SpriteIndex],
				uwBlood.Position,
				color, 0.0f, 1.0f, Vector2(uwBlood.Scale, uwBlood.Scale) * 2, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawFootprints(RenderView& view)
	{
		for (const auto& footprint : Footprints)
		{
			if (footprint.SpriteIndex >= g_Level.Sprites.size())
				continue;

			AddSprite3D(
				&m_sprites[footprint.SpriteIndex],
				footprint.VertexPoints[0], footprint.VertexPoints[1], footprint.VertexPoints[2], footprint.VertexPoints[3],
				Vector4(footprint.Opacity), 0.0f, 1.0f, Vector2::One, BLENDMODE_SUBTRACTIVE, false, view);
		}
	}

	Matrix Renderer11::GetWorldMatrixForSprite(RendererSpriteToDraw* spr, RenderView& view)
	{
		auto scale = Matrix::CreateScale(spr->Width * spr->Scale, spr->Height * spr->Scale, spr->Scale);

		Matrix spriteMatrix;
		if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD)
		{
			auto cameraUp = Vector3(view.camera.View._12, view.camera.View._22, view.camera.View._32);
			spriteMatrix = scale * Matrix::CreateRotationZ(spr->Rotation) * Matrix::CreateBillboard(spr->pos, Camera.pos.ToVector3(), cameraUp);
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_CUSTOM)
		{
			auto rotMatrix = Matrix::CreateRotationY(spr->Rotation);
			auto quadForward = Vector3(0, 0, 1);
			spriteMatrix = scale * Matrix::CreateConstrainedBillboard(
				spr->pos,
				Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z),
				spr->ConstrainAxis,
				nullptr,
				&quadForward);
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_LOOKAT)
		{
			auto transMatrix = Matrix::CreateTranslation(spr->pos);
			auto rotMatrix = Matrix::CreateRotationZ(spr->Rotation) * Matrix::CreateLookAt(Vector3::Zero, spr->LookAtAxis, Vector3::UnitZ);
			spriteMatrix = scale * rotMatrix * transMatrix;
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
		{
			spriteMatrix = Matrix::Identity;
		}

		return spriteMatrix;
	}

	void Renderer11::DrawSprites(RenderView& view)
	{
		if (view.spritesToDraw.size() == 0)
			return;

		// Sort sprites by sprite and blend mode for faster batching
		std::sort(
			view.spritesToDraw.begin(),
			view.spritesToDraw.end(),
			[](RendererSpriteToDraw& a, RendererSpriteToDraw& b)
			{
				if (a.Sprite != b.Sprite)
					return (a.Sprite > b.Sprite);
				else if (a.BlendMode != b.BlendMode)
					return (a.BlendMode > b.BlendMode);
				else
					return (a.Type > b.Type);
			}
		);

		// Group sprites to draw in buckets for instancing (only billboards)
		std::vector<RendererSpriteBucket> spriteBuckets;
		RendererSpriteBucket currentSpriteBucket;

		currentSpriteBucket.Sprite = view.spritesToDraw[0].Sprite;
		currentSpriteBucket.BlendMode = view.spritesToDraw[0].BlendMode;
		currentSpriteBucket.IsBillboard = view.spritesToDraw[0].Type != RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
		currentSpriteBucket.IsSoftParticle = view.spritesToDraw[0].SoftParticle;

		for (int i = 0; i < view.spritesToDraw.size(); i++)
		{
			RendererSpriteToDraw& spr = view.spritesToDraw[i];

			bool isBillboard = spr.Type != RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;

			if (spr.Sprite != currentSpriteBucket.Sprite ||
				spr.BlendMode != currentSpriteBucket.BlendMode ||
				spr.SoftParticle != currentSpriteBucket.IsSoftParticle ||
				currentSpriteBucket.SpritesToDraw.size() == INSTANCED_SPRITES_BUCKET_SIZE ||
				isBillboard != currentSpriteBucket.IsBillboard)
			{
				spriteBuckets.push_back(currentSpriteBucket);

				currentSpriteBucket.Sprite = spr.Sprite;
				currentSpriteBucket.BlendMode = spr.BlendMode;
				currentSpriteBucket.IsBillboard = isBillboard;
				currentSpriteBucket.IsSoftParticle = spr.SoftParticle;
				currentSpriteBucket.SpritesToDraw.clear();
			}

			if (DoesBlendModeRequireSorting(spr.BlendMode))
			{
				// If the blend mode requires sorting, save the sprite for later
				int distance = (spr.pos - Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z)).Length();
				RendererTransparentFace face;
				face.type = RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE;
				face.info.sprite = &spr;
				face.distance = distance;
				face.info.world = GetWorldMatrixForSprite(&spr, view);
				face.info.blendMode = spr.BlendMode;
				face.info.IsSoftParticle = spr.SoftParticle;

				RendererRoom& room = m_rooms[FindRoomNumber(Vector3i(spr.pos))];
				room.TransparentFacesToDraw.push_back(face);
			}
			else
			{
				// Otherwise, add the sprite to the current bucket
				currentSpriteBucket.SpritesToDraw.push_back(spr);
			}
		}

		spriteBuckets.push_back(currentSpriteBucket);

		BindRenderTargetAsTexture(TEXTURE_DEPTH_MAP, &m_depthMap, SAMPLER_LINEAR_CLAMP);

		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsInstancedSprites.Get(), NULL, 0);
		m_context->PSSetShader(m_psInstancedSprites.Get(), NULL, 0);

		// Set up vertex buffer and parameters
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_context->IASetVertexBuffers(0, 1, quadVertexBuffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.size() == 0 || !spriteBucket.IsBillboard)
				continue;

			// Prepare the constant buffer for instanced sprites
			for (int i = 0; i < spriteBucket.SpritesToDraw.size(); i++)
			{
				RendererSpriteToDraw& spr = spriteBucket.SpritesToDraw[i];

				m_stInstancedSpriteBuffer.Sprites[i].World = GetWorldMatrixForSprite(&spr, view);
				m_stInstancedSpriteBuffer.Sprites[i].Color = spr.color;
				m_stInstancedSpriteBuffer.Sprites[i].IsBillboard = 1;
				m_stInstancedSpriteBuffer.Sprites[i].IsSoftParticle = spr.SoftParticle ? 1 : 0;
			}

			SetBlendMode(spriteBucket.BlendMode);
			BindTexture(TEXTURE_COLOR_MAP, spriteBucket.Sprite->Texture, SAMPLER_LINEAR_CLAMP);

			if (spriteBucket.BlendMode == BLEND_MODES::BLENDMODE_ALPHATEST)
				SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD, true);
			else
				SetAlphaTest(ALPHA_TEST_NONE, 0);

			m_cbInstancedSpriteBuffer.updateData(m_stInstancedSpriteBuffer, m_context.Get());
			BindConstantBufferVS(CB_INSTANCED_SPRITES, m_cbInstancedSpriteBuffer.get());
			BindConstantBufferPS(CB_INSTANCED_SPRITES, m_cbInstancedSpriteBuffer.get());

			// Draw sprites with instancing
			m_context->DrawInstanced(4, spriteBucket.SpritesToDraw.size(), 0, 0);

			m_numSpritesDrawCalls++;
			m_numInstancedSpritesDrawCalls++;
			m_numDrawCalls++;
		}

		// Draw 3D sprites
		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsSprites.Get(), NULL, 0);
		m_context->PSSetShader(m_psSprites.Get(), NULL, 0);

		stride = sizeof(RendererVertex);
		offset = 0;
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_context->IASetVertexBuffers(0, 1, quadVertexBuffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.size() == 0 || spriteBucket.IsBillboard)
				continue;

			m_stSprite.Color = Vector4::One;
			m_stSprite.IsBillboard = 0;
			m_stSprite.IsSoftParticle = spriteBucket.IsSoftParticle ? 1 : 0;
			m_cbSprite.updateData(m_stSprite, m_context.Get());
			BindConstantBufferVS(CB_SPRITE, m_cbSprite.get());
			BindConstantBufferPS(CB_SPRITE, m_cbSprite.get());

			SetBlendMode(spriteBucket.BlendMode);
			BindTexture(TEXTURE_COLOR_MAP, spriteBucket.Sprite->Texture, SAMPLER_LINEAR_CLAMP);

			if (spriteBucket.BlendMode == BLEND_MODES::BLENDMODE_ALPHATEST)
				SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD, true);
			else
				SetAlphaTest(ALPHA_TEST_NONE, 0);

			m_primitiveBatch->Begin();

			for (auto& spr : spriteBucket.SpritesToDraw)
			{
				Vector3 p0t = spr.vtx1;
				Vector3 p1t = spr.vtx2;
				Vector3 p2t = spr.vtx3;
				Vector3 p3t = spr.vtx4;

				RendererVertex v0;
				v0.Position.x = p0t.x;
				v0.Position.y = p0t.y;
				v0.Position.z = p0t.z;
				v0.UV.x = spr.Sprite->UV[0].x;
				v0.UV.y = spr.Sprite->UV[0].y;
				v0.Color = spr.color;

				RendererVertex v1;
				v1.Position.x = p1t.x;
				v1.Position.y = p1t.y;
				v1.Position.z = p1t.z;
				v1.UV.x = spr.Sprite->UV[1].x;
				v1.UV.y = spr.Sprite->UV[1].y;
				v1.Color = spr.color;

				RendererVertex v2;
				v2.Position.x = p2t.x;
				v2.Position.y = p2t.y;
				v2.Position.z = p2t.z;
				v2.UV.x = spr.Sprite->UV[2].x;
				v2.UV.y = spr.Sprite->UV[2].y;
				v2.Color = spr.color;

				RendererVertex v3;
				v3.Position.x = p3t.x;
				v3.Position.y = p3t.y;
				v3.Position.z = p3t.z;
				v3.UV.x = spr.Sprite->UV[3].x;
				v3.UV.y = spr.Sprite->UV[3].y;
				v3.Color = spr.color;

				m_primitiveBatch->DrawTriangle(v0, v1, v3);
				m_primitiveBatch->DrawTriangle(v1, v2, v3);
			}

			m_primitiveBatch->End();

			m_numSpritesDrawCalls++;
			m_numDrawCalls++;
		}
	}

	void Renderer11::DrawSpritesTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->VSSetShader(m_vsSprites.Get(), NULL, 0);
		m_context->PSSetShader(m_psSprites.Get(), NULL, 0);

		m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());

		m_context->IASetVertexBuffers(0, 1, m_transparentFacesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_stSprite.BillboardMatrix = Matrix::Identity;
		m_stSprite.Color = Vector4::One;
		m_stSprite.IsBillboard = false;
		m_stSprite.IsSoftParticle = info->IsSoftParticle ? 1 : 0;
		m_cbSprite.updateData(m_stSprite, m_context.Get());
		BindConstantBufferVS(CB_SPRITE, m_cbSprite.get());

		SetBlendMode(info->blendMode);
		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);
		SetAlphaTest(ALPHA_TEST_NONE, 0);

		BindTexture(TEXTURE_COLOR_MAP, info->sprite->Sprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawTriangles(m_transparentFacesVertices.size(), 0);

		m_numTransparentDrawCalls++;
		m_numSpritesTransparentDrawCalls++;
	}

	void Renderer11::DrawEffect(RenderView& view, RendererEffect* effect, bool transparent)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		int firstBucket = (transparent ? 2 : 0);
		int lastBucket = (transparent ? 4 : 2);

		RendererRoom const& room = m_rooms[effect->RoomNumber];

		m_stStatic.World = effect->World;
		m_stStatic.Color = effect->Color;
		m_stStatic.AmbientLight = effect->AmbientLight;
		m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
		m_cbStatic.updateData(m_stStatic, m_context.Get());
		BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
		BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

		BindLights(effect->LightsToDraw);

		if (transparent)
		{
			SetAlphaTest(ALPHA_TEST_NONE, 1.0f);
		}
		else
		{
			SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);
		}

		RendererMesh* mesh = effect->Mesh;
		BLEND_MODES lastBlendMode = BLEND_MODES::BLENDMODE_UNSET;

		for (auto& bucket : mesh->Buckets)
		{
			if (bucket.NumVertices == 0)
				continue;

			if (!((bucket.BlendMode == BLENDMODE_OPAQUE || bucket.BlendMode == BLENDMODE_ALPHATEST) ^ transparent))
				continue;

			BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[bucket.Texture]), SAMPLER_ANISOTROPIC_CLAMP);
			BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[bucket.Texture]), SAMPLER_NONE);

			if (lastBlendMode != bucket.BlendMode)
			{
				lastBlendMode = bucket.BlendMode;
				SetBlendMode(lastBlendMode);
			}

			// Draw vertices
			DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);
		}

	}

	void Renderer11::DrawEffects(RenderView& view, bool transparent)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		int firstBucket = (transparent ? 2 : 0);
		int lastBucket = (transparent ? 4 : 2);

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (auto room : view.roomsToDraw)
		{
			for (auto effect : room->EffectsToDraw)
			{
				RendererRoom const& room = m_rooms[effect->RoomNumber];
				ObjectInfo* obj = &Objects[effect->ObjectNumber];

				if (obj->drawRoutine && obj->loaded)
					DrawEffect(view, effect, transparent);
			}
		}
	}

	void Renderer11::DrawDebris(RenderView& view, bool transparent)
	{
		extern vector<DebrisFragment> DebrisFragments;
		vector<RendererVertex> vertices;

		BLEND_MODES lastBlendMode = BLEND_MODES::BLENDMODE_UNSET;

		for (auto deb = DebrisFragments.begin(); deb != DebrisFragments.end(); deb++)
		{
			if (deb->active)
			{
				if (!((deb->mesh.blendMode == BLENDMODE_OPAQUE || deb->mesh.blendMode == BLENDMODE_ALPHATEST) ^ transparent))
					continue;

				Matrix translation = Matrix::CreateTranslation(deb->worldPosition.x, deb->worldPosition.y, deb->worldPosition.z);
				Matrix rotation = Matrix::CreateFromQuaternion(deb->rotation);
				Matrix world = rotation * translation;

				m_primitiveBatch->Begin();

				if (deb->isStatic)
				{
					BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_staticsTextures[deb->mesh.tex]), SAMPLER_LINEAR_CLAMP);
				}
				else
				{
					BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[deb->mesh.tex]), SAMPLER_LINEAR_CLAMP);
				}

				if (transparent)
				{
					SetAlphaTest(ALPHA_TEST_NONE, 1.0f);
				}
				else
				{
					SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);
				}

				m_stStatic.World = world;
				m_stStatic.Color = deb->color;
				m_stStatic.AmbientLight = m_rooms[deb->roomNumber].AmbientLight;
				m_stStatic.LightMode = deb->lightMode;

				m_cbStatic.updateData(m_stStatic, m_context.Get());
				BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

				RendererVertex vtx0;
				vtx0.Position = deb->mesh.Positions[0];
				vtx0.UV = deb->mesh.TextureCoordinates[0];
				vtx0.Normal = deb->mesh.Normals[0];
				vtx0.Color = deb->mesh.Colors[0];

				RendererVertex vtx1;
				vtx1.Position = deb->mesh.Positions[1];
				vtx1.UV = deb->mesh.TextureCoordinates[1];
				vtx1.Normal = deb->mesh.Normals[1];
				vtx1.Color = deb->mesh.Colors[1];

				RendererVertex vtx2;
				vtx2.Position = deb->mesh.Positions[2];
				vtx2.UV = deb->mesh.TextureCoordinates[2];
				vtx2.Normal = deb->mesh.Normals[2];
				vtx2.Color = deb->mesh.Colors[2];

				if (lastBlendMode != deb->mesh.blendMode)
				{
					lastBlendMode = deb->mesh.blendMode;
					SetBlendMode(lastBlendMode);
				}

				SetCullMode(CULL_MODE_NONE);
				m_primitiveBatch->DrawTriangle(vtx0, vtx1, vtx2);
				m_numDrawCalls++;
				m_primitiveBatch->End();
			}
		}
	}

	void Renderer11::DrawSmokeParticles(RenderView& view)
	{
		using TEN::Effects::Smoke::SmokeParticles;
		using TEN::Effects::Smoke::SmokeParticle;

		for (const auto& smoke : SmokeParticles) 
		{
			if (!smoke.active)
				continue;

			// TODO: Switch back to alpha blend mode once rendering for it is refactored. -- Sezz 2023.01.14
			AddSpriteBillboard(
				&m_sprites[Objects[ID_SMOKE_SPRITES].meshIndex + smoke.sprite],
				smoke.position,
				smoke.color, smoke.rotation, 1.0f, { smoke.size, smoke.size }, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawSparkParticles(RenderView& view)
	{
		using TEN::Effects::Spark::SparkParticle;
		using TEN::Effects::Spark::SparkParticles;

		extern std::array<SparkParticle, 128> SparkParticles;

		for (int i = 0; i < SparkParticles.size(); i++)
		{
			SparkParticle& s = SparkParticles[i];
			if (!s.active) continue;
			Vector3 v;
			s.velocity.Normalize(v);

			float normalizedLife = s.age / s.life;
			auto height = Lerp(1.0f, 0.0f, normalizedLife);
			auto color = Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);

			AddSpriteBillboardConstrained(&m_sprites[Objects[ID_SPARK_SPRITE].meshIndex], s.pos, color, 0, 1, { s.width, s.height * height }, BLENDMODE_ADDITIVE, -v, false, view);
		}
	}

	void Renderer11::DrawDripParticles(RenderView& view)
	{
		using TEN::Effects::Drip::DripParticle;
		using TEN::Effects::Drip::DripParticles;
		using TEN::Effects::Drip::DRIP_WIDTH;

		for (const auto& drip : DripParticles)
		{
			if (!drip.IsActive)
				continue;

			auto velocity = Vector3::Zero;
			drip.Velocity.Normalize(velocity);

			AddSpriteBillboardConstrained(
				&m_sprites[Objects[ID_DRIP_SPRITE].meshIndex],
				drip.Position,
				drip.Color, 0.0f, 1.0f, Vector2(DRIP_WIDTH, drip.Height), BLENDMODE_ADDITIVE, -velocity, false, view);
		}
	}

	void Renderer11::DrawExplosionParticles(RenderView& view)
	{
		using TEN::Effects::Explosion::explosionParticles;
		using TEN::Effects::Explosion::ExplosionParticle;

		for (const auto& explosion : explosionParticles)
		{
			if (!explosion.active)
				continue;

			AddSpriteBillboard(
				&m_sprites[Objects[ID_EXPLOSION_SPRITES].meshIndex + explosion.sprite],
				explosion.pos,
				explosion.tint, explosion.rotation, 1.0f, Vector2(explosion.size, explosion.size), BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawSimpleParticles(RenderView& view)
	{
		using namespace TEN::Effects;

		for (const auto& particle : simpleParticles)
		{
			if (!particle.active)
				continue;

			AddSpriteBillboard(
				&m_sprites[Objects[particle.sequence].meshIndex + particle.sprite],
				particle.worldPosition,
				Vector4::One, 0.0f, 1.0f, Vector2(particle.size, particle.size / 2), BLENDMODE_ALPHABLEND, true, view);
		}
	}
}
