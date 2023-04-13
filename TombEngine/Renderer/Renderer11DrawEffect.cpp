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
#include "Game/effects/Electricity.h"
#include "Game/effects/explosion.h"
#include "Game/effects/Footprint.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/Streamer.h"
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

using namespace TEN::Effects::Blood;
using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Drip;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Footprint;
using namespace TEN::Effects::Ripple;
using namespace TEN::Effects::Streamer;
using namespace TEN::Entities::Creatures::TR5;
using namespace TEN::Math;

extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];
extern GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH]; // offset 0xA31D8
extern Particle Particles[MAX_PARTICLES];
extern SPLASH_STRUCT Splashes[MAX_SPLASHES];

// TODO: EnemyBites must be eradicated and kept directly in object structs or passed to gunflash functions.

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
	{ 10, -60, 200, 11 }, // Baddy 2
	{ 20, -60, 400, 7 },  // SAS
	{ 0, -64, 250, 7 }	  // Troops
};

namespace TEN::Renderer 
{
	constexpr auto ELECTRICITY_RANGE_MAX = BLOCK(24);

	struct RendererSpriteBucket
	{
		RendererSprite* Sprite;
		BLEND_MODES BlendMode;
		std::vector<RendererSpriteToDraw> SpritesToDraw;
		bool IsBillboard;
		bool IsSoftParticle;
	};

	void Renderer11::DrawStreamers(RenderView& view)
	{
		constexpr auto BLEND_MODE_DEFAULT = BLENDMODE_ADDITIVE;

		for (const auto& [entityNumber, module] : StreamerEffect.Modules)
		{
			for (const auto& [tag, pool] : module.Pools)
			{
				for (const auto& streamer : pool)
				{
					for (int i = 0; i < streamer.Segments.size(); i++)
					{
						const auto& segment = streamer.Segments[i];
						const auto& prevSegment = streamer.Segments[std::max(i - 1, 0)];

						if (segment.Life <= 0.0f)
							continue;

						// Determine blend mode.
						auto blendMode = BLEND_MODE_DEFAULT;
						if (segment.Flags & (int)StreamerFlags::BlendModeAdditive)
							blendMode = BLENDMODE_ALPHABLEND;

						if (segment.Flags & (int)StreamerFlags::FadeLeft)
						{
							AddColoredQuad(
								segment.Vertices[0], segment.Vertices[1],
								prevSegment.Vertices[1], prevSegment.Vertices[0],
								Vector4::Zero, segment.Color,
								prevSegment.Color, Vector4::Zero,
								blendMode, view);
						}
						else if (segment.Flags & (int)StreamerFlags::FadeRight)
						{
							AddColoredQuad(
								segment.Vertices[0], segment.Vertices[1],
								prevSegment.Vertices[1], prevSegment.Vertices[0],
								segment.Color, Vector4::Zero,
								Vector4::Zero, prevSegment.Color,
								blendMode, view);
						}
						else
						{
							AddColoredQuad(
								segment.Vertices[0], segment.Vertices[1],
								prevSegment.Vertices[1], prevSegment.Vertices[0],
								segment.Color, segment.Color,
								prevSegment.Color, prevSegment.Color,
								blendMode, view);
						}
					}
				}
			}
		}
	}

	void Renderer11::DrawHelicalLasers(RenderView& view)
	{
		if (HelicalLasers.empty())
			return;

		for (const auto& laser : HelicalLasers)
		{
			if (laser.Life <= 0.0f)
				continue;

			auto color = laser.Color;
			color.w = laser.Opacity;

			ElectricityKnots[0] = laser.Target;
			ElectricityKnots[1] = laser.Origin;
			
			for (int j = 0; j < 2; j++)
				ElectricityKnots[j] -= laser.Target;

			CalculateHelixSpline(laser, ElectricityKnots, ElectricityBuffer);

			if (abs(ElectricityKnots[0].x) <= ELECTRICITY_RANGE_MAX &&
				abs(ElectricityKnots[0].y) <= ELECTRICITY_RANGE_MAX &&
				abs(ElectricityKnots[0].z) <= ELECTRICITY_RANGE_MAX)
			{
				int bufferIndex = 0;

				auto& interpPosArray = ElectricityBuffer;
				for (int s = 0; s < laser.NumSegments ; s++)
				{
					auto origin = laser.Target + interpPosArray[bufferIndex];
					bufferIndex++;
					auto target = laser.Target + interpPosArray[bufferIndex];

					auto center = (origin + target) / 2;
					auto direction = target - origin;
					direction.Normalize();

					AddSpriteBillboardConstrained(
						&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LIGHTHING],
						center,
						color,
						PI_DIV_2, 1.0f, Vector2(5 * 8.0f, Vector3::Distance(origin, target)), BLENDMODE_ADDITIVE, direction, true, view);							
				}
			}				
		}
	}

	void Renderer11::DrawElectricity(RenderView& view)
	{
		if (ElectricityArcs.empty())
			return;

		for (const auto& arc : ElectricityArcs)
		{
			if (arc.life <= 0)
				continue;

			ElectricityKnots[0] = arc.pos1;
			memcpy(&ElectricityKnots[1], &arc, 96); // TODO: What? Copying 94 / 4 = 24 floats, or 24 / 3 = 8 Vector3 objects, but that doesn't fit. Does it spill into the buffer?
			ElectricityKnots[5] = arc.pos4;

			for (int j = 0; j < ElectricityKnots.size(); j++)
				ElectricityKnots[j] -= LaraItem->Pose.Position.ToVector3();

			CalculateElectricitySpline(arc, ElectricityKnots, ElectricityBuffer);

			if (abs(ElectricityKnots[0].x) <= ELECTRICITY_RANGE_MAX &&
				abs(ElectricityKnots[0].y) <= ELECTRICITY_RANGE_MAX &&
				abs(ElectricityKnots[0].z) <= ELECTRICITY_RANGE_MAX)
			{
				int bufferIndex = 0;

				auto& interpPosArray = ElectricityBuffer;
				for (int s = 0; s < ((arc.segments * 3) - 1); s++)
				{
					auto origin = (LaraItem->Pose.Position + interpPosArray[bufferIndex]).ToVector3();
					bufferIndex++;
					auto target = (LaraItem->Pose.Position + interpPosArray[bufferIndex]).ToVector3();

					auto center = (origin + target) / 2;
					auto direction = target - origin;
					direction.Normalize();

					byte r, g, b;
					if (arc.life >= 16)
					{
						r = arc.r;
						g = arc.g;
						b = arc.b;
					}
					else
					{
						r = (arc.life * arc.r) / 16;
						g = (arc.life * arc.g) / 16;
						b = (arc.life * arc.b) / 16;
					}

					AddSpriteBillboardConstrained(
						&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LIGHTHING],
						center,
						Vector4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f),
						PI_DIV_2, 1.0f, Vector2(arc.width * 8, Vector3::Distance(origin, target)), BLENDMODE_ADDITIVE, direction, true, view);
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
		for (int i = 0; i < ParticleNodeOffsetIDs::NodeMax; i++)
			NodeOffsets[i].gotIt = false;

		for (auto& particle : Particles)
		{
			if (!particle.on)
				continue;

			if (particle.flags & SP_DEF)
			{
				auto pos = Vector3(particle.x, particle.y, particle.z);

				if (particle.flags & SP_FX)
				{
					const auto& fx = EffectList[particle.fxObj];

					pos += fx.pos.Position.ToVector3();

					if ((particle.sLife - particle.life) > Random::GenerateInt(8, 12))
					{
						particle.flags &= ~SP_FX;
						particle.x = pos.x;
						particle.y = pos.y;
						particle.z = pos.z;
					}
				}
				else if (!(particle.flags & SP_ITEM))
				{
					pos.x = particle.x;
					pos.y = particle.y;
					pos.z = particle.z;
				}
				else
				{
					auto* item = &g_Level.Items[particle.fxObj];

					auto nodePos = Vector3i::Zero;
					if (particle.flags & SP_NODEATTACH)
					{
						if (NodeOffsets[particle.nodeNumber].gotIt)
						{
							nodePos = NodeVectors[particle.nodeNumber];
						}
						else
						{
							nodePos.x = NodeOffsets[particle.nodeNumber].x;
							nodePos.y = NodeOffsets[particle.nodeNumber].y;
							nodePos.z = NodeOffsets[particle.nodeNumber].z;

							int meshIndex = NodeOffsets[particle.nodeNumber].meshNum;
							if (meshIndex >= 0)
								nodePos = GetJointPosition(item, meshIndex, nodePos);
							else
								nodePos = GetJointPosition(LaraItem, -meshIndex, nodePos);

							NodeOffsets[particle.nodeNumber].gotIt = true;
							NodeVectors[particle.nodeNumber] = nodePos;
						}

						pos += nodePos.ToVector3();

						if ((particle.sLife - particle.life) > Random::GenerateInt(4, 8))
						{
							particle.flags &= ~SP_ITEM;
							particle.x = pos.x;
							particle.y = pos.y;
							particle.z = pos.z;
						}
					}
					else
					{
						pos += item->Pose.Position.ToVector3();
					}
				}

				// Don't allow sprites out of bounds.
				int spriteIndex = std::clamp((int)particle.spriteIndex, 0, (int)m_sprites.size());

				AddSpriteBillboard(
					&m_sprites[spriteIndex],
					pos,
					Vector4(particle.r / (float)UCHAR_MAX, particle.g / (float)UCHAR_MAX, particle.b / (float)UCHAR_MAX, 1.0f),
					TO_RAD(particle.rotAng << 4), particle.scalar,
					Vector2(particle.size, particle.size),
					particle.blendMode, true, view);
			}
			else
			{
				auto pos = Vector3(particle.x, particle.y, particle.z);
				auto axis = Vector3(particle.xVel, particle.yVel, particle.zVel);
				axis.Normalize();

				AddSpriteBillboardConstrained(
					&m_sprites[Objects[ID_SPARK_SPRITE].meshIndex],
					pos,
					Vector4(particle.r / (float)UCHAR_MAX, particle.g / (float)UCHAR_MAX, particle.b / (float)UCHAR_MAX, 1.0f),
					TO_RAD(particle.rotAng << 4),
					particle.scalar,
					Vector2(4, particle.size), particle.blendMode, axis, true, view);
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
				float xInner;
				float zInner;
				float xOuter;
				float zOuter;
				float x2Inner;
				float z2Inner;
				float x2Outer;
				float z2Outer;
				float yInner = splash.y;
				float yOuter = splash.y - splash.height;

				for (int i = 0; i < NUM_POINTS; i++) 
				{
					xInner = innerRadius * sin(alpha * i * PI / 180);
					zInner = innerRadius * cos(alpha * i * PI / 180);
					xOuter = outerRadius * sin(alpha * i * PI / 180);
					zOuter = outerRadius * cos(alpha * i * PI / 180);
					xInner += splash.x;
					zInner += splash.z;
					xOuter += splash.x;
					zOuter += splash.z;
					int j = (i + 1) % NUM_POINTS;
					x2Inner = innerRadius * sin(alpha * j * PI / 180);
					x2Inner += splash.x;
					z2Inner = innerRadius * cos(alpha * j * PI / 180);
					z2Inner += splash.z;
					x2Outer = outerRadius * sin(alpha * j * PI / 180);
					x2Outer += splash.x;
					z2Outer = outerRadius * cos(alpha * j * PI / 180);
					z2Outer += splash.z;
					AddQuad(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + splash.spriteSequenceStart + (int)splash.animationPhase],
								Vector3(xOuter, yOuter, zOuter), 
								Vector3(x2Outer, yOuter, z2Outer), 
								Vector3(x2Inner, yInner, z2Inner), 
								Vector3(xInner, yInner, zInner), Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 
								0, 1, { 0, 0 }, BLENDMODE_ADDITIVE, false, view);
				}
			}
		}
	}

	void Renderer11::DrawBubbles(RenderView& view) 
	{
		if (Bubbles.empty())
			return;

		for (const auto& bubble : Bubbles)
		{
			if (bubble.Life <= 0.0f)
				continue;

			AddSpriteBillboard(
				&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + bubble.SpriteIndex],
				bubble.Position,
				bubble.Color, 0.0f, 1.0f, bubble.Size / 2, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawDrips(RenderView& view)
	{
		if (Drips.empty())
			return;

		for (const auto& drip : Drips)
		{
			if (drip.Life <= 0.0f)
				continue;

			auto axis = drip.Velocity;
			drip.Velocity.Normalize(axis);

			AddSpriteBillboardConstrained(
				&m_sprites[Objects[ID_DRIP_SPRITE].meshIndex],
				drip.Position,
				drip.Color, 0.0f, 1.0f, drip.Size, BLENDMODE_ADDITIVE, -axis, false, view);
		}
	}

	void Renderer11::DrawRipples(RenderView& view) 
	{
		if (Ripples.empty())
			return;

		for (const auto& ripple : Ripples)
		{
			if (ripple.Life <= 0.0f)
				continue;

			float opacity = ripple.Color.w * ((ripple.Flags & (int)RippleFlags::LowOpacity) ? 0.5f : 1.0f);
			auto color = ripple.Color;
			color.w = opacity;

			AddSpriteBillboardConstrainedLookAt(
				&m_sprites[ripple.SpriteIndex],
				ripple.Position,
				color, 0.0f, 1.0f, Vector2(ripple.Size * 2), BLENDMODE_ADDITIVE, ripple.Normal, true, view);
		}
	}

	void Renderer11::DrawUnderwaterBloodParticles(RenderView& view)
	{
		if (UnderwaterBloodParticles.empty())
			return;

		for (const auto& uwBlood : UnderwaterBloodParticles)
		{
			if (uwBlood.Life <= 0.0f)
				continue;

			auto color = Vector4::Zero;
			if (uwBlood.Init)
				color = Vector4(uwBlood.Init / 2, 0, uwBlood.Init / 16, UCHAR_MAX);
			else
				color = Vector4(uwBlood.Life / 2, 0, uwBlood.Life / 16, UCHAR_MAX);

			color.x = (int)std::clamp((int)color.x, 0, UCHAR_MAX);
			color.y = (int)std::clamp((int)color.y, 0, UCHAR_MAX);
			color.z = (int)std::clamp((int)color.z, 0, UCHAR_MAX);
			color /= UCHAR_MAX;

			AddSpriteBillboard(
				&m_sprites[uwBlood.SpriteIndex],
				uwBlood.Position,
				color, 0.0f, 1.0f, Vector2(uwBlood.Size, uwBlood.Size) * 2, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawShockwaves(RenderView& view)
	{
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
		float c = 0;
		float s = 0;
		float angle = 0;

		for (int i = 0; i < MAX_SHOCKWAVE; i++)
		{
			SHOCKWAVE_STRUCT* shockwave = &ShockWaves[i];

			if (shockwave->life)
			{
				byte color = shockwave->life * 8;

				//int dl = shockwave->outerRad - shockwave->innerRad;

				shockwave->yRot += shockwave->yRot / FPS;

				auto rotMatrix =
					Matrix::CreateRotationY(shockwave->yRot / 4) *
					Matrix::CreateRotationZ(shockwave->zRot) *
					Matrix::CreateRotationX(shockwave->xRot);

				auto pos = Vector3(shockwave->x, shockwave->y, shockwave->z);

				// Inner circle
				if (shockwave->style == (int)ShockwaveStyle::Normal)
				{
					angle = PI / 32.0f;
					c = cos(angle);
					s = sin(angle);
					angle -= PI / 8.0f;
				}
				else
				{
					angle = PI / 16.0f;
					c = cos(angle);
					s = sin(angle);
					angle -= PI / 4.0f;
				}

				float x1 = (shockwave->innerRad * c);
				float z1 = (shockwave->innerRad * s);
				float x4 = (shockwave->outerRad * c);
				float z4 = (shockwave->outerRad * s);

				auto p1 = Vector3(x1, 0, z1);
				auto p4 = Vector3(x4, 0, z4);

				p1 = Vector3::Transform(p1, rotMatrix);
				p4 = Vector3::Transform(p4, rotMatrix);

				if (shockwave->fadeIn == true)
				{
					if (shockwave->sr < shockwave->r)
					{
						shockwave->sr += shockwave->r / 18;
						r = shockwave->sr * shockwave->life / 255.0f;
					}
					else
					{
						r = shockwave->r * shockwave->life / 255.0f;
					}


					if (shockwave->sg < shockwave->g)
					{
						shockwave->sg += shockwave->g / 18;
						g = shockwave->sg * shockwave->life / 255.0f;
					}
					else
					{
						g = shockwave->g * shockwave->life / 255.0f;
					}


					if (shockwave->sb < shockwave->b)
					{
						shockwave->sb += shockwave->b / 18;
						b = shockwave->sb * shockwave->life / 255.0f;
					}
					else
					{
						b = shockwave->b * shockwave->life / 255.0f;
					}

					if (r == shockwave->r && g == shockwave->g && b == shockwave->b)
						shockwave->fadeIn = false;

				}
				else
				{
					r = shockwave->r * shockwave->life / 255.0f;
					g = shockwave->g * shockwave->life / 255.0f;
					b = shockwave->b * shockwave->life / 255.0f;
				}

				for (int j = 0; j < 16; j++)
				{
					c = cos(angle);
					s = sin(angle);

					float x2 = (shockwave->innerRad * c);
					float z2 = (shockwave->innerRad * s);

					float x3 = (shockwave->outerRad * c);
					float z3 = (shockwave->outerRad * s);

					auto p2 = Vector3(x2, 0, z2);
					auto p3 = Vector3(x3, 0, z3);

					p2 = Vector3::Transform(p2, rotMatrix);
					p3 = Vector3::Transform(p3, rotMatrix);

					if (shockwave->style == (int)ShockwaveStyle::Normal)
					{
						angle -= PI / 8.0f;

						AddQuad(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH],
							pos + p1,
							pos + p2,
							pos + p3,
							pos + p4,
							Vector4(
								r / 16.0f,
								g / 16.0f,
								b / 16.0f,
								1.0f),
							0, 1, { 0,0 }, BLENDMODE_ADDITIVE, false, view);
					}
					else if (shockwave->style == (int)ShockwaveStyle::Sophia)
					{
						angle -= PI / 4.0f;

						AddQuad(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH3],
							pos + p1,
							pos + p2,
							pos + p3,
							pos + p4,
							Vector4(
								r / 16.0f,
								g / 16.0f,
								b / 16.0f,
								1.0f),
							0, 1, { 0,0 }, BLENDMODE_ADDITIVE, true, view);

					}
					else if (shockwave->style == (int)ShockwaveStyle::Knockback)
					{
						angle -= PI / 4.0f;

						AddQuad(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH3],
							pos + p4,
							pos + p3,
							pos + p2,
							pos + p1,
							Vector4(
								r / 16.0f,
								g / 16.0f,
								b / 16.0f,
								1.0f),
							0, 1, { 0,0 }, BLENDMODE_ADDITIVE, true, view);
					}

					p1 = p2;
					p4 = p3;
				}
			}
		}
	}

	void Renderer11::DrawBlood(RenderView& view) 
	{
		for (int i = 0; i < 32; i++) 
		{
			BLOOD_STRUCT* blood = &Blood[i];

			if (blood->on) 
			{
				AddSpriteBillboard(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_BLOOD],
								   Vector3(blood->x, blood->y, blood->z),
								   Vector4(blood->shade / 255.0f, blood->shade * 0, blood->shade * 0, 1.0f),
								   TO_RAD(blood->rotAng << 4), 1.0f, { blood->size * 8.0f, blood->size * 8.0f },
								   BLENDMODE_ADDITIVE, true, view);
			}
		}
	}

	void Renderer11::DrawWeatherParticles(RenderView& view) 
	{
		constexpr auto RAIN_WIDTH = 4.0f;

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
					0.0f, 1.0f, Vector2(RAIN_WIDTH, p.Size), BLENDMODE_ADDITIVE, -v, true, view);

				break;
			}
		}
	}

	bool Renderer11::DrawGunFlashes(RenderView& view) 
	{
		m_context->VSSetShader(m_vsStatics.Get(), nullptr, 0);
		m_context->PSSetShader(m_psStatics.Get(), nullptr, 0);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (!Lara.RightArm.GunFlash && !Lara.LeftArm.GunFlash)
			return true;

		if (BinocularRange > 0)
			return true;

		RendererRoom const & room = m_rooms[LaraItem->RoomNumber];
		RendererItem* item = &m_items[Lara.ItemNumber];

		m_stStatic.Color = Vector4::One;
		m_stStatic.AmbientLight = room.AmbientLight;
		m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_STATIC;
		BindStaticLights(item->LightsToDraw);

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
					world = item->AnimationTransforms[LM_LHAND] * item->World;
					world = offset * world;
					world = rotation * world;

					m_stStatic.World = world;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
					BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}

				if (Lara.RightArm.GunFlash)
				{
					world = item->AnimationTransforms[LM_RHAND] * item->World;
					world = offset * world;
					world = rotation * world;

					m_stStatic.World = world;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
					BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}
			}
		}

		SetBlendMode(BLENDMODE_OPAQUE);

		return true;
	}

	void Renderer11::DrawBaddyGunflashes(RenderView& view)
	{
		m_context->VSSetShader(m_vsStatics.Get(), nullptr, 0);
		m_context->PSSetShader(m_psStatics.Get(), nullptr, 0);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

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
				BindStaticLights(item->LightsToDraw); // FIXME: Is it really needed for gunflashes? -- Lwmte, 15.07.22
				 
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
		std::vector<byte> data = { 128, 128, 255, 1 };
		return Texture2D(m_device.Get(), 1, 1, data.data());
	}

	void Renderer11::DrawFootprints(RenderView& view) 
	{
		for (const auto& footprint : Footprints)
		{
			AddQuad(
				&m_sprites[footprint.SpriteIndex],
				footprint.VertexPoints[0], footprint.VertexPoints[1], footprint.VertexPoints[2], footprint.VertexPoints[3],
				Vector4(footprint.Opacity), 0.0f, 1.0f, Vector2::One, BLENDMODE_SUBTRACTIVE, false, view);
		}
	}

	Matrix Renderer11::GetWorldMatrixForSprite(RendererSpriteToDraw* spr, RenderView& view)
	{
		Matrix spriteMatrix;
		Matrix scale = Matrix::CreateScale((spr->Width) * spr->Scale, (spr->Height) * spr->Scale, spr->Scale);

		if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD)
		{
			Vector3 cameraUp = Vector3(view.camera.View._12, view.camera.View._22, view.camera.View._32);
			spriteMatrix = scale * Matrix::CreateRotationZ(spr->Rotation) * Matrix::CreateBillboard(spr->pos, Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), cameraUp);
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_CUSTOM)
		{
			Matrix rotation = Matrix::CreateRotationY(spr->Rotation);
			Vector3 quadForward = Vector3(0, 0, 1);
			spriteMatrix = scale * Matrix::CreateConstrainedBillboard(
				spr->pos,
				Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z),
				spr->ConstrainAxis,
				nullptr,
				&quadForward);
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_LOOKAT)
		{
			Matrix translation = Matrix::CreateTranslation(spr->pos);
			Matrix rotation = Matrix::CreateRotationZ(spr->Rotation) * Matrix::CreateLookAt(Vector3::Zero, spr->LookAtAxis, Vector3::UnitZ);
			spriteMatrix = scale * rotation * translation;
		}
		else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
		{
			spriteMatrix = Matrix::Identity;
		}

		return spriteMatrix;
	}

	void Renderer11::DrawSprites(RenderView& view)
	{
		if (view.spritesToDraw.empty())
			return;

		// Sort sprites by sprite and blend mode for faster batching.
		std::sort(
			view.spritesToDraw.begin(),
			view.spritesToDraw.end(),
			[](RendererSpriteToDraw& rDrawSprite0, RendererSpriteToDraw& rDrawSprite1)
			{
				if (rDrawSprite0.Sprite != rDrawSprite1.Sprite)
				{
					return (rDrawSprite0.Sprite > rDrawSprite1.Sprite);
				}
				else if (rDrawSprite0.BlendMode != rDrawSprite1.BlendMode)
				{
					return (rDrawSprite0.BlendMode > rDrawSprite1.BlendMode);
				}
				else
				{
					return (rDrawSprite0.Type > rDrawSprite1.Type);
				}
			}
		);

		// Group sprites to draw in buckets for instancing (billboards only).
		std::vector<RendererSpriteBucket> spriteBuckets;
		RendererSpriteBucket currentSpriteBucket;

		currentSpriteBucket.Sprite = view.spritesToDraw[0].Sprite;
		currentSpriteBucket.BlendMode = view.spritesToDraw[0].BlendMode;
		currentSpriteBucket.IsBillboard = view.spritesToDraw[0].Type != RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
		currentSpriteBucket.IsSoftParticle = view.spritesToDraw[0].SoftParticle;

		for (auto& rDrawSprite : view.spritesToDraw)
		{
			bool isBillboard = rDrawSprite.Type != RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;

			if (rDrawSprite.Sprite != currentSpriteBucket.Sprite || 
				rDrawSprite.BlendMode != currentSpriteBucket.BlendMode ||
				rDrawSprite.SoftParticle != currentSpriteBucket.IsSoftParticle ||
				currentSpriteBucket.SpritesToDraw.size() == INSTANCED_SPRITES_BUCKET_SIZE || 
				isBillboard != currentSpriteBucket.IsBillboard)
			{
				spriteBuckets.push_back(currentSpriteBucket);

				currentSpriteBucket.Sprite = rDrawSprite.Sprite;
				currentSpriteBucket.BlendMode = rDrawSprite.BlendMode;
				currentSpriteBucket.IsBillboard = isBillboard;
				currentSpriteBucket.IsSoftParticle = rDrawSprite.SoftParticle;
				currentSpriteBucket.SpritesToDraw.clear();
			}
				 
			if (DoesBlendModeRequireSorting(rDrawSprite.BlendMode))
			{
				// If blend mode requires sorting, save sprite for later.
				int distance = (rDrawSprite.pos - Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z)).Length();
				RendererTransparentFace face;
				face.type = RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE;
				face.info.sprite = &rDrawSprite;
				face.distance = distance;
				face.info.world = GetWorldMatrixForSprite(&rDrawSprite, view);
				face.info.blendMode = rDrawSprite.BlendMode;

				for (int j = 0; j < view.roomsToDraw.size(); j++)
				{
					short roomNumber = view.roomsToDraw[j]->RoomNumber;
					if (g_Level.Rooms[roomNumber].Active() && IsPointInRoom(Vector3i(rDrawSprite.pos), roomNumber))
					{
						view.roomsToDraw[j]->TransparentFacesToDraw.push_back(face);
						break;
					}
				}
			}
			// Add sprite to current bucket.
			else
			{
				currentSpriteBucket.SpritesToDraw.push_back(rDrawSprite);
			}
		}

		spriteBuckets.push_back(currentSpriteBucket);

		BindRenderTargetAsTexture(TEXTURE_DEPTH_MAP, &m_depthMap, SAMPLER_LINEAR_CLAMP);

		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsInstancedSprites.Get(), nullptr, 0);
		m_context->PSSetShader(m_psInstancedSprites.Get(), nullptr, 0);

		// Set up vertex buffer and parameters.
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_context->IASetVertexBuffers(0, 1, quadVertexBuffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.size() == 0 || !spriteBucket.IsBillboard)
				continue;

			// Prepare constant buffer for instanced sprites.
			for (int i = 0; i < spriteBucket.SpritesToDraw.size(); i++)
			{
				auto& rDrawSprite = spriteBucket.SpritesToDraw[i];

				m_stInstancedSpriteBuffer.Sprites[i].World = GetWorldMatrixForSprite(&rDrawSprite, view);
				m_stInstancedSpriteBuffer.Sprites[i].Color = rDrawSprite.color;
				m_stInstancedSpriteBuffer.Sprites[i].IsBillboard = 1;
				m_stInstancedSpriteBuffer.Sprites[i].IsSoftParticle = rDrawSprite.SoftParticle ? 1 : 0;
				 
				// NOTE: Strange packing due to particular HLSL 16 byte alignment requirements.
				m_stInstancedSpriteBuffer.Sprites[i].UV[0].x = rDrawSprite.Sprite->UV[0].x;
				m_stInstancedSpriteBuffer.Sprites[i].UV[0].y = rDrawSprite.Sprite->UV[1].x;
				m_stInstancedSpriteBuffer.Sprites[i].UV[0].z = rDrawSprite.Sprite->UV[2].x;
				m_stInstancedSpriteBuffer.Sprites[i].UV[0].w = rDrawSprite.Sprite->UV[3].x;
				m_stInstancedSpriteBuffer.Sprites[i].UV[1].x = rDrawSprite.Sprite->UV[0].y;
				m_stInstancedSpriteBuffer.Sprites[i].UV[1].y = rDrawSprite.Sprite->UV[1].y;
				m_stInstancedSpriteBuffer.Sprites[i].UV[1].z = rDrawSprite.Sprite->UV[2].y;
				m_stInstancedSpriteBuffer.Sprites[i].UV[1].w = rDrawSprite.Sprite->UV[3].y;
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

			// Draw sprites with instancing.
			DrawInstancedTriangles(4, spriteBucket.SpritesToDraw.size(), 0);

			m_numSpritesDrawCalls++;
		}

		// Draw 3D sprites.
		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsSprites.Get(), nullptr, 0);
		m_context->PSSetShader(m_psSprites.Get(), nullptr, 0);

		stride = sizeof(RendererVertex);
		offset = 0;
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		m_context->IASetVertexBuffers(0, 1, quadVertexBuffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.empty() || spriteBucket.IsBillboard)
				continue;

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

			for (auto& rDrawSprite : spriteBucket.SpritesToDraw)
			{
				auto vertex0 = RendererVertex{};
				vertex0.Position = rDrawSprite.vtx1;
				vertex0.UV = rDrawSprite.Sprite->UV[0];
				vertex0.Color = rDrawSprite.c1;

				auto vertex1 = RendererVertex{};
				vertex1.Position = rDrawSprite.vtx2;
				vertex1.UV = rDrawSprite.Sprite->UV[1];
				vertex1.Color = rDrawSprite.c2;

				auto vertex2 = RendererVertex{};
				vertex2.Position = rDrawSprite.vtx3;
				vertex2.UV = rDrawSprite.Sprite->UV[2];
				vertex2.Color = rDrawSprite.c3;

				auto vertex3 = RendererVertex{};
				vertex3.Position = rDrawSprite.vtx4;
				vertex3.UV = rDrawSprite.Sprite->UV[3];
				vertex3.Color = rDrawSprite.c4;

				m_primitiveBatch->DrawTriangle(vertex0, vertex1, vertex3);
				m_primitiveBatch->DrawTriangle(vertex1, vertex2, vertex3);
			}

			m_primitiveBatch->End();

			m_numSpritesDrawCalls++;
			m_numDrawCalls++;
		}
	}

	void Renderer11::DrawSpritesSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view)
	{	
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->VSSetShader(m_vsSprites.Get(), NULL, 0);
		m_context->PSSetShader(m_psSprites.Get(), NULL, 0);

		m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());
		  
		m_context->IASetVertexBuffers(0, 1, m_transparentFacesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		if (resetPipeline)
		{
			m_stSprite.IsSoftParticle = info->sprite->SoftParticle ? 1 : 0;
			m_cbSprite.updateData(m_stSprite, m_context.Get());
			BindConstantBufferVS(CB_SPRITE, m_cbSprite.get());
			BindConstantBufferPS(CB_SPRITE, m_cbSprite.get());
		}

		SetBlendMode(info->sprite->BlendMode);
		SetCullMode(CULL_MODE_NONE);
		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetAlphaTest(ALPHA_TEST_NONE, 0);

		BindTexture(TEXTURE_COLOR_MAP, info->sprite->Sprite->Texture, SAMPLER_LINEAR_CLAMP);

		DrawTriangles(m_transparentFacesVertices.size(), 0);

		m_numTransparentDrawCalls++;
		m_numSpritesTransparentDrawCalls++;

		SetCullMode(CULL_MODE_CCW);
	}

	void Renderer11::DrawEffect(RenderView& view, RendererEffect* effect, bool transparent) 
	{
		RendererRoom const& room = m_rooms[effect->RoomNumber];

		m_stStatic.World = effect->World;
		m_stStatic.Color = effect->Color;
		m_stStatic.AmbientLight = effect->AmbientLight;
		m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
		BindStaticLights(effect->LightsToDraw);
		m_cbStatic.updateData(m_stStatic, m_context.Get());
		BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
		BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

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

			SetBlendMode(lastBlendMode);
			
			DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);
		}

	}

	void Renderer11::DrawEffects(RenderView& view, bool transparent) 
	{
		m_context->VSSetShader(m_vsStatics.Get(), NULL, 0);
		m_context->PSSetShader(m_psStatics.Get(), NULL, 0);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

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
		m_context->VSSetShader(m_vsStatics.Get(), NULL, 0);
		m_context->PSSetShader(m_psStatics.Get(), NULL, 0);

		extern std::vector<DebrisFragment> DebrisFragments;
		std::vector<RendererVertex> vertices;

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

			AddSpriteBillboard(
				&m_sprites[Objects[ID_SMOKE_SPRITES].meshIndex + smoke.sprite],
				smoke.position,
				smoke.color, smoke.rotation, 1.0f, { smoke.size, smoke.size }, BLENDMODE_ALPHABLEND, true, view);
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

	void Renderer11::DrawExplosionParticles(RenderView& view)
	{
		using TEN::Effects::Explosion::explosionParticles;
		using TEN::Effects::Explosion::ExplosionParticle;

		for (int i = 0; i < explosionParticles.size(); i++) 
		{
			ExplosionParticle& e = explosionParticles[i];
			if (!e.active) continue;
			AddSpriteBillboard(&m_sprites[Objects[ID_EXPLOSION_SPRITES].meshIndex + e.sprite], e.pos, e.tint, e.rotation, 1.0f, { e.size, e.size }, BLENDMODE_ADDITIVE, true, view);
		}
	}

	void Renderer11::DrawSimpleParticles(RenderView& view)
	{
		using namespace TEN::Effects;

		for(SimpleParticle& s : simpleParticles)
		{
			if(!s.active) continue;
			AddSpriteBillboard(&m_sprites[Objects[s.sequence].meshIndex + s.sprite], s.worldPosition, Vector4(1, 1, 1, 1), 0, 1.0f, { s.size, s.size / 2 }, BLENDMODE_ALPHABLEND, true, view);
		}
	}
}
