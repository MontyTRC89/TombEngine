#include "framework.h"
#include "Renderer/Renderer.h"

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
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/TR5/Trap/LaserBarrier.h"
#include "Objects/Utils/object_helper.h"
#include "Renderer/Structures/RendererSprite2D.h"
#include "Renderer/Structures/RendererSprite.h"
#include "Specific/level.h"
#include "Structures/SpriteRenderBucket.h"

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
using namespace TEN::Traps::TR5;

extern BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD];
extern FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE];
extern SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE];
extern SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE];
extern FIRE_LIST Fires[MAX_FIRE_LIST];
extern Particle Particles[MAX_PARTICLES];
extern SPLASH_STRUCT Splashes[MAX_SPLASHES];

namespace TEN::Renderer 
{
	constexpr auto ELECTRICITY_RANGE_MAX = BLOCK(24);
	
	void Renderer::DrawLaserBarriers(RenderView& view)
	{
		if (LaserBarriers.empty())
			return;

		for (const auto& [entityID, barrier] : LaserBarriers)
		{
			for (const auto& beam : barrier.Beams)
			{
				AddColoredQuad(
					beam.VertexPoints[0], beam.VertexPoints[1],
					beam.VertexPoints[2], beam.VertexPoints[3],
					barrier.Color, barrier.Color,
					barrier.Color, barrier.Color,
					BlendMode::Additive, view, SpriteRenderType::LaserBarrier);
			}
		}
	}

	void Renderer::DrawStreamers(RenderView& view)
	{
		constexpr auto BLEND_MODE_DEFAULT = BlendMode::Additive;

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
							blendMode = BlendMode::AlphaBlend;

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

	void Renderer::DrawHelicalLasers(RenderView& view)
	{
		if (HelicalLasers.empty())
			return;

		if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Helical lasers rendering"))
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
						&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LIGHTHING],
						center,
						color,
						PI_DIV_2, 1.0f, Vector2(5 * 8.0f, Vector3::Distance(origin, target)), BlendMode::Additive, direction, true, view);							
				}
			}				
		}
	}

	void Renderer::DrawElectricity(RenderView& view)
	{
		if (ElectricityArcs.empty())
			return;

		if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Electricity rendering"))
			return;

		for (const auto& arc : ElectricityArcs)
		{
			if (arc.life <= 0)
				continue;

			ElectricityKnots[0] = arc.pos1;
			ElectricityKnots[1] = arc.pos1;
			ElectricityKnots[2] = arc.pos2;
			ElectricityKnots[3] = arc.pos3;
			ElectricityKnots[4] = arc.pos4;
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
						&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LIGHTHING],
						center,
						Vector4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f),
						PI_DIV_2, 1.0f, Vector2(arc.width * 8, Vector3::Distance(origin, target)), BlendMode::Additive, direction, true, view);
				}
			}
		}
	}

	void Renderer::DrawSmokes(RenderView& view) 
	{
		for (int i = 0; i < 32; i++) 
		{
			SMOKE_SPARKS* spark = &SmokeSparks[i];

			if (spark->on) 
			{
				AddSpriteBillboard(&_sprites[spark->def],
								   Vector3(spark->x, spark->y, spark->z),
								   Vector4(spark->shade / 255.0f, spark->shade / 255.0f, spark->shade / 255.0f, 1.0f),
								   TO_RAD(spark->rotAng << 4), spark->scalar, { spark->size * 4.0f, spark->size * 4.0f },
								   BlendMode::Additive, true, view);
			}
		}
	}


	void Renderer::DrawFires(RenderView& view) 
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
							&_sprites[spark->def],
							Vector3(fire->x + spark->x * fire->size / 2, fire->y + spark->y * fire->size / 2, fire->z + spark->z * fire->size / 2),
							Vector4(spark->r / 255.0f * fade, spark->g / 255.0f * fade, spark->b / 255.0f * fade, 1.0f),
							TO_RAD(spark->rotAng << 4),
							spark->scalar,
							Vector2(spark->size * fire->size, spark->size * fire->size), BlendMode::Additive, true, view);
					}
				}
			}
		}
	}

	void Renderer::DrawParticles(RenderView& view)
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
				int spriteIndex = std::clamp((int)particle.spriteIndex, 0, (int)_sprites.size());

				AddSpriteBillboard(
					&_sprites[spriteIndex],
					pos,
					Vector4(particle.r / (float)UCHAR_MAX, particle.g / (float)UCHAR_MAX, particle.b / (float)UCHAR_MAX, 1.0f),
					TO_RAD(particle.rotAng << 4), particle.scalar,
					Vector2(particle.size, particle.size),
					particle.blendMode, true, view);
			}
			else
			{
				if (!CheckIfSlotExists(ID_SPARK_SPRITE, "Particle rendering"))
					continue;

				auto pos = Vector3(particle.x, particle.y, particle.z);
				auto axis = Vector3(particle.xVel, particle.yVel, particle.zVel);
				axis.Normalize();

				AddSpriteBillboardConstrained(
					&_sprites[Objects[ID_SPARK_SPRITE].meshIndex],
					pos,
					Vector4(particle.r / (float)UCHAR_MAX, particle.g / (float)UCHAR_MAX, particle.b / (float)UCHAR_MAX, 1.0f),
					TO_RAD(particle.rotAng << 4),
					particle.scalar,
					Vector2(4, particle.size), particle.blendMode, axis, true, view);
			}
		}
	}

	void Renderer::DrawSplashes(RenderView& view) 
	{
		constexpr size_t NUM_POINTS = 9;

		for (int i = 0; i < MAX_SPLASHES; i++) 
		{
			auto& splash = Splashes[i];

			if (!splash.isActive)
				continue;

			if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Splashes rendering"))
				return;

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
				AddQuad(&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + splash.spriteSequenceStart + (int)splash.animationPhase],
							Vector3(xOuter, yOuter, zOuter), 
							Vector3(x2Outer, yOuter, z2Outer), 
							Vector3(x2Inner, yInner, z2Inner), 
							Vector3(xInner, yInner, zInner), Vector4(color / 255.0f, color / 255.0f, color / 255.0f, 1.0f), 
							0, 1, { 0, 0 }, BlendMode::Additive, false, view);
			}
		}
	}

	void Renderer::DrawBubbles(RenderView& view) 
	{
		if (Bubbles.empty())
			return;

		if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Bubbles rendering"))
			return;

		for (const auto& bubble : Bubbles)
		{
			if (bubble.Life <= 0.0f)
				continue;

			AddSpriteBillboard(
				&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + bubble.SpriteIndex],
				bubble.Position,
				bubble.Color, 0.0f, 1.0f, bubble.Size / 2, BlendMode::Additive, true, view);
		}
	}

	void Renderer::DrawDrips(RenderView& view)
	{
		if (Drips.empty())
			return;

		if (!CheckIfSlotExists(ID_DRIP_SPRITE, "Drips rendering"))
			return;

		for (const auto& drip : Drips)
		{
			if (drip.Life <= 0.0f)
				continue;

			auto axis = drip.Velocity;
			drip.Velocity.Normalize(axis);

			AddSpriteBillboardConstrained(
				&_sprites[Objects[ID_DRIP_SPRITE].meshIndex],
				drip.Position,
				drip.Color, 0.0f, 1.0f, drip.Size, BlendMode::Additive, -axis, false, view);
		}
	}

	void Renderer::DrawRipples(RenderView& view) 
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
				&_sprites[ripple.SpriteIndex],
				ripple.Position,
				color, 0.0f, 1.0f, Vector2(ripple.Size * 2), BlendMode::Additive, ripple.Normal, true, view);
		}
	}

	void Renderer::DrawUnderwaterBloodParticles(RenderView& view)
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
				&_sprites[uwBlood.SpriteIndex],
				uwBlood.Position,
				color, 0.0f, 1.0f, Vector2(uwBlood.Size, uwBlood.Size) * 2, BlendMode::Additive, true, view);
		}
	}

	void Renderer::DrawShockwaves(RenderView& view)
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

			if (!shockwave->life)
				continue;

			if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Shockwaves rendering"))
				return;

			byte color = shockwave->life * 8;

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

					AddQuad(&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH],
						pos + p1,
						pos + p2,
						pos + p3,
						pos + p4,
						Vector4(
							r / 16.0f,
							g / 16.0f,
							b / 16.0f,
							1.0f),
						0, 1, { 0,0 }, BlendMode::Additive, false, view);
				}
				else if (shockwave->style == (int)ShockwaveStyle::Sophia)
				{
					angle -= PI / 4.0f;

					AddQuad(&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH3],
						pos + p1,
						pos + p2,
						pos + p3,
						pos + p4,
						Vector4(
							r / 16.0f,
							g / 16.0f,
							b / 16.0f,
							1.0f),
						0, 1, { 0,0 }, BlendMode::Additive, true, view);

				}
				else if (shockwave->style == (int)ShockwaveStyle::Knockback)
				{
					angle -= PI / 4.0f;

					AddQuad(&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_SPLASH3],
						pos + p4,
						pos + p3,
						pos + p2,
						pos + p1,
						Vector4(
							r / 16.0f,
							g / 16.0f,
							b / 16.0f,
							1.0f),
						0, 1, { 0,0 }, BlendMode::Additive, true, view);
				}

				p1 = p2;
				p4 = p3;
			}
		}
	}

	void Renderer::DrawBlood(RenderView& view) 
	{
		for (int i = 0; i < 32; i++) 
		{
			BLOOD_STRUCT* blood = &Blood[i];

			if (blood->on) 
			{
				if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Blood rendering"))
					return;

				AddSpriteBillboard(&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_BLOOD],
								   Vector3(blood->x, blood->y, blood->z),
								   Vector4(blood->shade / 255.0f, blood->shade * 0, blood->shade * 0, 1.0f),
								   TO_RAD(blood->rotAng << 4), 1.0f, { blood->size * 8.0f, blood->size * 8.0f },
								   BlendMode::Additive, true, view);
			}
		}
	}

	void Renderer::DrawWeatherParticles(RenderView& view) 
	{
		constexpr auto RAIN_WIDTH = 4.0f;

		for (auto& p : Weather.GetParticles())
		{
			if (!p.Enabled)
				continue;

			switch (p.Type)
			{
			case WeatherType::None:

				if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Underwater dust rendering"))
					return;

				AddSpriteBillboard(
					&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST],
					p.Position,
					Vector4(1.0f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(p.Size),
					BlendMode::Additive, true, view);

				break;

			case WeatherType::Snow:

				if (!CheckIfSlotExists(ID_DEFAULT_SPRITES, "Snow rendering"))
					return;

				AddSpriteBillboard(
					&_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST],
					p.Position,
					Vector4(1.0f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(p.Size),
					BlendMode::Additive, true, view);

				break;

			case WeatherType::Rain:

				if (!CheckIfSlotExists(ID_DRIP_SPRITE, "Rain rendering"))
					return;

				Vector3 v;
				p.Velocity.Normalize(v);

				AddSpriteBillboardConstrained(
					&_sprites[Objects[ID_DRIP_SPRITE].meshIndex], 
					p.Position,
					Vector4(0.8f, 1.0f, 1.0f, p.Transparency()),
					0.0f, 1.0f, Vector2(RAIN_WIDTH, p.Size), BlendMode::Additive, -v, true, view);

				break;
			}
		}
	}

	bool Renderer::DrawGunFlashes(RenderView& view) 
	{
		_context->VSSetShader(_vsStatics.Get(), nullptr, 0);
		_context->PSSetShader(_psStatics.Get(), nullptr, 0);

		BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
		BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		_context->IASetVertexBuffers(0, 1, _moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());
		_context->IASetIndexBuffer(_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (!Lara.RightArm.GunFlash && !Lara.LeftArm.GunFlash)
			return true;

		if (Lara.Control.Look.OpticRange > 0)
			return true;

		const auto& room = _rooms[LaraItem->RoomNumber];
		auto* itemPtr = &_items[LaraItem->Index];

		_stStatic.Color = Vector4::One;
		_stStatic.AmbientLight = room.AmbientLight;
		_stStatic.LightMode = (int)LightMode::Static;
		BindStaticLights(itemPtr->LightsToDraw);

		short length = 0;
		short zOffset = 0;
		short rotationX = 0;

		SetBlendMode(BlendMode::Additive);
		SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD);

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

			// Use MP5 flash if available.
			auto gunflash = GAME_OBJECT_ID::ID_GUN_FLASH;
			if (Lara.Control.Weapon.GunType == LaraWeaponType::HK && Objects[GAME_OBJECT_ID::ID_GUN_FLASH2].loaded)
			{
				gunflash = GAME_OBJECT_ID::ID_GUN_FLASH2;
				length += 20;
				zOffset += 10;
			}

			const auto& flashMoveable = *_moveableObjects[gunflash];
			const auto& flashMesh = *flashMoveable.ObjectMeshes[0];

			for (const auto& flashBucket : flashMesh.Buckets) 
			{
				if (flashBucket.BlendMode == BlendMode::Opaque)
					continue;

				if (flashBucket.Polygons.size() == 0)
					continue;

				BindTexture(TextureRegister::ColorMap, &std::get<0>(_moveablesTextures[flashBucket.Texture]), SamplerStateRegister::AnisotropicClamp);

				auto tMatrix = Matrix::CreateTranslation(0, length, zOffset);
				auto rotMatrix = Matrix::CreateRotationX(TO_RAD(rotationX));

				auto worldMatrix = Matrix::Identity;
				if (Lara.LeftArm.GunFlash)
				{
					worldMatrix = itemPtr->AnimationTransforms[LM_LHAND] * itemPtr->World;
					worldMatrix = tMatrix * worldMatrix;
					worldMatrix = rotMatrix * worldMatrix;

					_stStatic.World = worldMatrix;
					_cbStatic.updateData(_stStatic, _context.Get());
					BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
					BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}

				if (Lara.RightArm.GunFlash)
				{
					worldMatrix = itemPtr->AnimationTransforms[LM_RHAND] * itemPtr->World;
					worldMatrix = tMatrix * worldMatrix;
					worldMatrix = rotMatrix * worldMatrix;

					_stStatic.World = worldMatrix;
					_cbStatic.updateData(_stStatic, _context.Get());
					BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
					BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

					DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
				}
			}
		}

		SetBlendMode(BlendMode::Opaque);
		return true;
	}

	void Renderer::DrawBaddyGunflashes(RenderView& view)
	{
		_context->VSSetShader(_vsStatics.Get(), nullptr, 0);
		_context->PSSetShader(_psStatics.Get(), nullptr, 0);

		BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
		BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		_context->IASetVertexBuffers(0, 1, _moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());
		_context->IASetIndexBuffer(_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (auto* rRoomPtr : view.RoomsToDraw)
		{
			for (auto* rItemPtr : rRoomPtr->ItemsToDraw)
			{
				auto& nativeItem = g_Level.Items[rItemPtr->ItemNumber];

				if (!nativeItem.IsCreature())
					continue;

				auto& creature = *GetCreatureInfo(&nativeItem);
				const auto& rRoom = _rooms[nativeItem.RoomNumber];

				_stStatic.Color = Vector4::One;
				_stStatic.AmbientLight = rRoom.AmbientLight;
				_stStatic.LightMode = (int)LightMode::Static;

				BindStaticLights(rItemPtr->LightsToDraw); // FIXME: Is it really needed for gunflashes? -- Lwmte, 15.07.22
				SetBlendMode(BlendMode::Additive);
				SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD);

				if (creature.MuzzleFlash[0].Delay != 0 && creature.MuzzleFlash[0].Bite.BoneID != -1)
				{
					auto flashObjectID = creature.MuzzleFlash[0].SwitchToMuzzle2 ?
						_moveableObjects[ID_GUN_FLASH2].has_value() ? ID_GUN_FLASH2 : ID_GUN_FLASH :
						ID_GUN_FLASH;

					const auto& flashMoveable = *_moveableObjects[flashObjectID]->ObjectMeshes.at(0);
					
					for (const auto& flashBucket : flashMoveable.Buckets)
					{
						if (flashBucket.BlendMode == BlendMode::Opaque)
							continue;

						if (flashBucket.Polygons.size() == 0)
							continue;

						BindTexture(TextureRegister::ColorMap, &std::get<0>(_moveablesTextures[flashBucket.Texture]), SamplerStateRegister::AnisotropicClamp);

						auto tMatrix = Matrix::CreateTranslation(creature.MuzzleFlash[0].Bite.Position);
						auto rotMatrixX = Matrix::CreateRotationX(TO_RAD(ANGLE(270.0f)));
						auto rotMatrixZ = Matrix::CreateRotationZ(TO_RAD(2 * GetRandomControl()));

						auto worldMatrix = rItemPtr->AnimationTransforms[creature.MuzzleFlash[0].Bite.BoneID] * rItemPtr->World;
						worldMatrix = tMatrix * worldMatrix;

						if (creature.MuzzleFlash[0].ApplyXRotation)
							worldMatrix = rotMatrixX * worldMatrix;

						if (creature.MuzzleFlash[0].ApplyZRotation)
							worldMatrix = rotMatrixZ * worldMatrix;

						_stStatic.World = worldMatrix;
						_cbStatic.updateData(_stStatic, _context.Get());
						BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
						BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());
						DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
					}
				}

				if (creature.MuzzleFlash[1].Delay != 0 && creature.MuzzleFlash[1].Bite.BoneID != -1)
				{
					auto flashObjectID = creature.MuzzleFlash[1].SwitchToMuzzle2 ?
						_moveableObjects[ID_GUN_FLASH2].has_value() ? ID_GUN_FLASH2 : ID_GUN_FLASH :
						ID_GUN_FLASH;

					const auto& flashMoveable = *_moveableObjects[flashObjectID]->ObjectMeshes.at(0);
					
					for (auto& flashBucket : flashMoveable.Buckets)
					{
						if (flashBucket.BlendMode == BlendMode::Opaque)
							continue;

						if (flashBucket.Polygons.size() == 0)
							continue;

						BindTexture(TextureRegister::ColorMap, &std::get<0>(_moveablesTextures[flashBucket.Texture]), SamplerStateRegister::AnisotropicClamp);

						auto tMatrix = Matrix::CreateTranslation(creature.MuzzleFlash[1].Bite.Position);
						auto rotMatrixX = Matrix::CreateRotationX(TO_RAD(ANGLE(270.0f)));
						auto rotMatrixZ = Matrix::CreateRotationZ(TO_RAD(2 * GetRandomControl()));

						auto worldMatrix = rItemPtr->AnimationTransforms[creature.MuzzleFlash[1].Bite.BoneID] * rItemPtr->World;
						worldMatrix = tMatrix * worldMatrix;

						if (creature.MuzzleFlash[1].ApplyXRotation)
							worldMatrix = rotMatrixX * worldMatrix;

						if (creature.MuzzleFlash[1].ApplyZRotation)
							worldMatrix = rotMatrixZ * worldMatrix;

						_stStatic.World = worldMatrix;
						_cbStatic.updateData(_stStatic, _context.Get());
						BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
						BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());
						DrawIndexedTriangles(flashBucket.NumIndices, flashBucket.StartIndex, 0);
					}
				}
			}
		}

		SetBlendMode(BlendMode::Opaque);
	}

	Texture2D Renderer::CreateDefaultNormalTexture() 
	{
		std::vector<byte> data = { 128, 128, 255, 1 };
		return Texture2D(_device.Get(), 1, 1, data.data());
	}

	void Renderer::DrawFootprints(RenderView& view) 
	{
		for (const auto& footprint : Footprints)
		{
			AddQuad(
				&_sprites[footprint.SpriteIndex],
				footprint.VertexPoints[0], footprint.VertexPoints[1], footprint.VertexPoints[2], footprint.VertexPoints[3],
				Vector4(footprint.Opacity), 0.0f, 1.0f, Vector2::One, BlendMode::Subtractive, false, view);
		}
	}

	Matrix Renderer::GetWorldMatrixForSprite(RendererSpriteToDraw* sprite, RenderView& view)
	{
		auto spriteMatrix = Matrix::Identity;
		auto scaleMatrix = Matrix::CreateScale(sprite->Width * sprite->Scale, sprite->Height * sprite->Scale, sprite->Scale);

		switch (sprite->Type)
		{
		case SpriteType::Billboard:
		{
			auto cameraUp = Vector3(view.Camera.View._12, view.Camera.View._22, view.Camera.View._32);
			spriteMatrix = scaleMatrix * Matrix::CreateRotationZ(sprite->Rotation) * Matrix::CreateBillboard(sprite->pos, Camera.pos.ToVector3(), cameraUp);
		}
		break;

		case SpriteType::CustomBillboard:
		{
			auto rotMatrix = Matrix::CreateRotationY(sprite->Rotation);
			auto quadForward = Vector3(0.0f, 0.0f, 1.0f);
			spriteMatrix = scaleMatrix * Matrix::CreateConstrainedBillboard(
				sprite->pos,
				Camera.pos.ToVector3(),
				sprite->ConstrainAxis,
				nullptr,
				&quadForward);
		}
		break;

		case SpriteType::LookAtBillboard:
		{
			auto tMatrix = Matrix::CreateTranslation(sprite->pos);
			auto rotMatrix = Matrix::CreateRotationZ(sprite->Rotation) * Matrix::CreateLookAt(Vector3::Zero, sprite->LookAtAxis, Vector3::UnitZ);
			spriteMatrix = scaleMatrix * rotMatrix * tMatrix;
		}
		break;

		case SpriteType::ThreeD:
		default:
			break;
		}

		return spriteMatrix;
	}

	void Renderer::DrawSprites(RenderView& view)
	{
		if (view.SpritesToDraw.empty())
			return;

		// Sort sprites by sprite and blend mode for faster batching.
		std::sort(
			view.SpritesToDraw.begin(),
			view.SpritesToDraw.end(),
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

		currentSpriteBucket.Sprite = view.SpritesToDraw[0].Sprite;
		currentSpriteBucket.BlendMode = view.SpritesToDraw[0].BlendMode;
		currentSpriteBucket.IsBillboard = view.SpritesToDraw[0].Type != SpriteType::ThreeD;
		currentSpriteBucket.IsSoftParticle = view.SpritesToDraw[0].SoftParticle;
		currentSpriteBucket.RenderType = view.SpritesToDraw[0].renderType;

		for (auto& rDrawSprite : view.SpritesToDraw)
		{
			bool isBillboard = rDrawSprite.Type != SpriteType::ThreeD;

			if (rDrawSprite.Sprite != currentSpriteBucket.Sprite || 
				rDrawSprite.BlendMode != currentSpriteBucket.BlendMode ||
				rDrawSprite.SoftParticle != currentSpriteBucket.IsSoftParticle ||
				rDrawSprite.renderType != currentSpriteBucket.RenderType ||
				currentSpriteBucket.SpritesToDraw.size() == INSTANCED_SPRITES_BUCKET_SIZE || 
				isBillboard != currentSpriteBucket.IsBillboard)
			{
				spriteBuckets.push_back(currentSpriteBucket);

				currentSpriteBucket.Sprite = rDrawSprite.Sprite;
				currentSpriteBucket.BlendMode = rDrawSprite.BlendMode;
				currentSpriteBucket.IsBillboard = isBillboard;
				currentSpriteBucket.IsSoftParticle = rDrawSprite.SoftParticle;
				currentSpriteBucket.RenderType = rDrawSprite.renderType;
				currentSpriteBucket.SpritesToDraw.clear();
			}
				 
			// HACK: Prevent sprites like Explosionsmoke which have BlendMode::Subtractive from having laser effects.
			if (DoesBlendModeRequireSorting(rDrawSprite.BlendMode) && currentSpriteBucket.RenderType != SpriteRenderType::Default)
			{
				// If blend mode requires sorting, save sprite for later.
				int distance = (rDrawSprite.pos - Camera.pos.ToVector3()).Length();
				RendererTransparentFace face;
				face.type = TransparentFaceType::Sprite;
				face.info.sprite = &rDrawSprite;
				face.distance = distance;
				face.info.world = GetWorldMatrixForSprite(&rDrawSprite, view);
				face.info.blendMode = rDrawSprite.BlendMode;

				for (int j = 0; j < view.RoomsToDraw.size(); j++)
				{
					int roomNumber = view.RoomsToDraw[j]->RoomNumber;
					if (g_Level.Rooms[roomNumber].Active() && IsPointInRoom(Vector3i(rDrawSprite.pos), roomNumber))
					{
						view.RoomsToDraw[j]->TransparentFacesToDraw.push_back(face);
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

		BindRenderTargetAsTexture(TextureRegister::DepthMap, &_depthMap, SamplerStateRegister::LinearClamp);

		SetDepthState(DepthState::Read);
		SetCullMode(CullMode::None);

		_context->VSSetShader(_vsInstancedSprites.Get(), nullptr, 0);
		_context->PSSetShader(_psInstancedSprites.Get(), nullptr, 0);

		// Set up vertex buffer and parameters.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetInputLayout(_inputLayout.Get());
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_context->IASetVertexBuffers(0, 1, _quadVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.size() == 0 || !spriteBucket.IsBillboard)
				continue;

			// Prepare constant buffer for instanced sprites.
			for (int i = 0; i < spriteBucket.SpritesToDraw.size(); i++)
			{
				auto& rDrawSprite = spriteBucket.SpritesToDraw[i];

				_stInstancedSpriteBuffer.Sprites[i].World = GetWorldMatrixForSprite(&rDrawSprite, view);
				_stInstancedSpriteBuffer.Sprites[i].Color = rDrawSprite.color;
				_stInstancedSpriteBuffer.Sprites[i].IsBillboard = 1;
				_stInstancedSpriteBuffer.Sprites[i].IsSoftParticle = rDrawSprite.SoftParticle ? 1 : 0;
				 
				// NOTE: Strange packing due to particular HLSL 16 byte alignment requirements.
				_stInstancedSpriteBuffer.Sprites[i].UV[0].x = rDrawSprite.Sprite->UV[0].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].y = rDrawSprite.Sprite->UV[1].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].z = rDrawSprite.Sprite->UV[2].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].w = rDrawSprite.Sprite->UV[3].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].x = rDrawSprite.Sprite->UV[0].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].y = rDrawSprite.Sprite->UV[1].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].z = rDrawSprite.Sprite->UV[2].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].w = rDrawSprite.Sprite->UV[3].y;
			}

			SetBlendMode(spriteBucket.BlendMode);
			BindTexture(TextureRegister::ColorMap, spriteBucket.Sprite->Texture, SamplerStateRegister::LinearClamp);

			if (spriteBucket.BlendMode == BlendMode::AlphaTest)
			{
				SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD, true);
			}
			else
			{
				SetAlphaTest(AlphaTestMode::None, 0);
			}

			_cbInstancedSpriteBuffer.updateData(_stInstancedSpriteBuffer, _context.Get());
			BindConstantBufferVS(ConstantBufferRegister::InstancedSprites, _cbInstancedSpriteBuffer.get());
			BindConstantBufferPS(ConstantBufferRegister::InstancedSprites, _cbInstancedSpriteBuffer.get());

			// Draw sprites with instancing.
			DrawInstancedTriangles(4, (unsigned int)spriteBucket.SpritesToDraw.size(), 0);

			_numSpritesDrawCalls++;
		}

		// Draw 3D sprites.
		SetDepthState(DepthState::Read);
		SetCullMode(CullMode::None);

		_context->VSSetShader(_vsSprites.Get(), nullptr, 0);
		_context->PSSetShader(_psSprites.Get(), nullptr, 0);

		stride = sizeof(Vertex);
		offset = 0;
		_context->IASetInputLayout(_inputLayout.Get());
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		_context->IASetVertexBuffers(0, 1, _quadVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.empty() || spriteBucket.IsBillboard)
				continue;

			_stSprite.IsSoftParticle = spriteBucket.IsSoftParticle ? 1 : 0;
			_stSprite.RenderType = (int)spriteBucket.RenderType;

			_cbSprite.updateData(_stSprite, _context.Get());
			BindConstantBufferVS(ConstantBufferRegister::Sprite, _cbSprite.get());
			BindConstantBufferPS(ConstantBufferRegister::Sprite, _cbSprite.get());

			SetBlendMode(spriteBucket.BlendMode);
			BindTexture(TextureRegister::ColorMap, spriteBucket.Sprite->Texture, SamplerStateRegister::LinearClamp);

			if (spriteBucket.BlendMode == BlendMode::AlphaTest)
			{
				SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD, true);
			}
			else
			{
				SetAlphaTest(AlphaTestMode::None, 0);
			}

			_primitiveBatch->Begin();

			for (auto& rDrawSprite : spriteBucket.SpritesToDraw)
			{
				auto vertex0 = Vertex{};
				vertex0.Position = rDrawSprite.vtx1;
				vertex0.UV = rDrawSprite.Sprite->UV[0];
				vertex0.Color = rDrawSprite.c1;

				auto vertex1 = Vertex{};
				vertex1.Position = rDrawSprite.vtx2;
				vertex1.UV = rDrawSprite.Sprite->UV[1];
				vertex1.Color = rDrawSprite.c2;

				auto vertex2 = Vertex{};
				vertex2.Position = rDrawSprite.vtx3;
				vertex2.UV = rDrawSprite.Sprite->UV[2];
				vertex2.Color = rDrawSprite.c3;

				auto vertex3 = Vertex{};
				vertex3.Position = rDrawSprite.vtx4;
				vertex3.UV = rDrawSprite.Sprite->UV[3];
				vertex3.Color = rDrawSprite.c4;

				_primitiveBatch->DrawTriangle(vertex0, vertex1, vertex3);
				_primitiveBatch->DrawTriangle(vertex1, vertex2, vertex3);
			}

			_primitiveBatch->End();

			_numSpritesDrawCalls++;
			_numDrawCalls++;
		}
	}

	void Renderer::DrawSpritesSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view)
	{	
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		_context->VSSetShader(_vsSprites.Get(), nullptr, 0);
		_context->PSSetShader(_psSprites.Get(), nullptr, 0);

		_transparentFacesVertexBuffer.Update(_context.Get(), _transparentFacesVertices, 0, (int)_transparentFacesVertices.size());
		  
		_context->IASetVertexBuffers(0, 1, _transparentFacesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());

		if (resetPipeline)
		{
			_stSprite.IsSoftParticle = info->sprite->SoftParticle ? 1 : 0;
			_stSprite.RenderType = (int)SpriteRenderType::Default;

			_cbSprite.updateData(_stSprite, _context.Get());
			BindConstantBufferVS(ConstantBufferRegister::Sprite, _cbSprite.get());
			BindConstantBufferPS(ConstantBufferRegister::Sprite, _cbSprite.get());
		}

		SetBlendMode(info->sprite->BlendMode);
		SetCullMode(CullMode::None);
		SetDepthState(DepthState::Read);
		SetAlphaTest(AlphaTestMode::None, 0);

		BindTexture(TextureRegister::ColorMap, info->sprite->Sprite->Texture, SamplerStateRegister::LinearClamp);

		DrawTriangles((int)_transparentFacesVertices.size(), 0);

		_numTransparentDrawCalls++;
		_numSpritesTransparentDrawCalls++;

		SetCullMode(CullMode::CounterClockwise);
	}

	void Renderer::DrawEffect(RenderView& view, RendererEffect* effect, RendererPass rendererPass) 
	{
		const auto& room = _rooms[effect->RoomNumber];

		_stStatic.World = effect->World;
		_stStatic.Color = effect->Color;
		_stStatic.AmbientLight = effect->AmbientLight;
		_stStatic.LightMode = (int)LightMode::Dynamic;
		BindStaticLights(effect->LightsToDraw);
		_cbStatic.updateData(_stStatic, _context.Get());
		BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
		BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

		if (rendererPass == RendererPass::Transparent)
		{
			SetAlphaTest(AlphaTestMode::None, 1.0f);
		}
		else
		{
			SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD);
		}

		auto* meshPtr = effect->Mesh;
		auto m_lastBlendMode = BlendMode::Unknown;

		for (auto& bucket : meshPtr->Buckets) 
		{
			if (bucket.NumVertices == 0)
				continue;

			if (!((bucket.BlendMode == BlendMode::Opaque || bucket.BlendMode == BlendMode::AlphaTest) ^
				(rendererPass == RendererPass::Transparent)))
			{
				continue;
			}

			BindTexture(TextureRegister::ColorMap, &std::get<0>(_moveablesTextures[bucket.Texture]), SamplerStateRegister::AnisotropicClamp);
			BindTexture(TextureRegister::NormalMap, &std::get<1>(_moveablesTextures[bucket.Texture]), SamplerStateRegister::AnisotropicClamp);

			SetBlendMode(m_lastBlendMode);
			
			DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);
		}
	}

	void Renderer::DrawEffects(RenderView& view, RendererPass rendererPass) 
	{
		_context->VSSetShader(_vsStatics.Get(), nullptr, 0);
		_context->PSSetShader(_psStatics.Get(), nullptr, 0);

		BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
		BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		_context->IASetVertexBuffers(0, 1, _moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_context->IASetInputLayout(_inputLayout.Get());
		_context->IASetIndexBuffer(_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (auto* roomPtr : view.RoomsToDraw)
		{
			for (auto* effectPtr : roomPtr->EffectsToDraw)
			{
				const auto& room = _rooms[effectPtr->RoomNumber];
				const auto& object = Objects[effectPtr->ObjectNumber];

				if (object.drawRoutine && object.loaded)
					DrawEffect(view, effectPtr, rendererPass);
			}
		}
	}

	void Renderer::DrawDebris(RenderView& view, RendererPass rendererPass)
	{		
		_context->VSSetShader(_vsStatics.Get(), nullptr, 0);
		_context->PSSetShader(_psStatics.Get(), nullptr, 0);

		BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());
		BindConstantBufferPS(ConstantBufferRegister::Static, _cbStatic.get());

		extern std::vector<DebrisFragment> DebrisFragments;
		std::vector<Vertex> vertices;

		auto m_lastBlendMode = BlendMode::Unknown;

		for (auto deb = DebrisFragments.begin(); deb != DebrisFragments.end(); deb++)
		{
			if (deb->active) 
			{
				if (!((deb->mesh.blendMode == BlendMode::Opaque || deb->mesh.blendMode == BlendMode::AlphaTest) ^
					(rendererPass == RendererPass::Transparent)))
				{
					continue;
				}

				auto translation = Matrix::CreateTranslation(deb->worldPosition.x, deb->worldPosition.y, deb->worldPosition.z);
				auto rotation = Matrix::CreateFromQuaternion(deb->rotation);
				auto world = rotation * translation;

				_primitiveBatch->Begin();

				if (deb->isStatic) 
				{
					BindTexture(TextureRegister::ColorMap, &std::get<0>(_staticTextures[deb->mesh.tex]), SamplerStateRegister::LinearClamp);
				} 
				else 
				{
					BindTexture(TextureRegister::ColorMap, &std::get<0>(_moveablesTextures[deb->mesh.tex]), SamplerStateRegister::LinearClamp);
				}

				if (rendererPass == RendererPass::Transparent)
				{
					SetAlphaTest(AlphaTestMode::None, 1.0f);
				}
				else
				{
					SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD);
				}

				_stStatic.World = world;
				_stStatic.Color = deb->color;
				_stStatic.AmbientLight = _rooms[deb->roomNumber].AmbientLight;
				_stStatic.LightMode = (int)deb->lightMode;

				_cbStatic.updateData(_stStatic, _context.Get());
				BindConstantBufferVS(ConstantBufferRegister::Static, _cbStatic.get());

				Vertex vtx0;
				vtx0.Position = deb->mesh.Positions[0];
				vtx0.UV = deb->mesh.TextureCoordinates[0];
				vtx0.Normal = deb->mesh.Normals[0];
				vtx0.Color = deb->mesh.Colors[0];

				Vertex vtx1;
				vtx1.Position = deb->mesh.Positions[1];
				vtx1.UV = deb->mesh.TextureCoordinates[1];
				vtx1.Normal = deb->mesh.Normals[1];
				vtx1.Color = deb->mesh.Colors[1];

				Vertex vtx2;
				vtx2.Position = deb->mesh.Positions[2];
				vtx2.UV = deb->mesh.TextureCoordinates[2];
				vtx2.Normal = deb->mesh.Normals[2];
				vtx2.Color = deb->mesh.Colors[2];

				if (m_lastBlendMode != deb->mesh.blendMode)
				{
					m_lastBlendMode = deb->mesh.blendMode;
					SetBlendMode(m_lastBlendMode);
				}

				SetCullMode(CullMode::None);
				_primitiveBatch->DrawTriangle(vtx0, vtx1, vtx2);
				_numDrawCalls++;
				_primitiveBatch->End();
			}
		}
	}

	void Renderer::DrawSmokeParticles(RenderView& view)
	{
		using TEN::Effects::Smoke::SmokeParticles;
		using TEN::Effects::Smoke::SmokeParticle;

		for (const auto& smoke : SmokeParticles) 
		{
			if (!smoke.active)
				continue;

			if (!CheckIfSlotExists(ID_SMOKE_SPRITES, "Smoke rendering"))
				return;

			AddSpriteBillboard(
				&_sprites[Objects[ID_SMOKE_SPRITES].meshIndex + smoke.sprite],
				smoke.position,
				smoke.color, smoke.rotation, 1.0f, { smoke.size, smoke.size }, BlendMode::AlphaBlend, true, view);
		}
	}

	void Renderer::DrawSparkParticles(RenderView& view)
	{
		using TEN::Effects::Spark::SparkParticle;
		using TEN::Effects::Spark::SparkParticles;

		extern std::array<SparkParticle, 128> SparkParticles;

		for (int i = 0; i < SparkParticles.size(); i++) 
		{
			SparkParticle& s = SparkParticles[i];
			if (!s.active) continue;

			if (!CheckIfSlotExists(ID_SPARK_SPRITE, "Spark particle rendering"))
				return;

			Vector3 v;
			s.velocity.Normalize(v);

			float normalizedLife = s.age / s.life;
			auto height = Lerp(1.0f, 0.0f, normalizedLife);
			auto color = Vector4::Lerp(s.sourceColor, s.destinationColor, normalizedLife);

			AddSpriteBillboardConstrained(&_sprites[Objects[ID_SPARK_SPRITE].meshIndex], s.pos, color, 0, 1, { s.width, s.height * height }, BlendMode::Additive, -v, false, view);
		}
	}

	void Renderer::DrawExplosionParticles(RenderView& view)
	{
		using TEN::Effects::Explosion::explosionParticles;
		using TEN::Effects::Explosion::ExplosionParticle;

		for (int i = 0; i < explosionParticles.size(); i++) 
		{
			ExplosionParticle& e = explosionParticles[i];
			if (!e.active) continue;

			if (!CheckIfSlotExists(ID_EXPLOSION_SPRITES, "Explosion particles rendering"))
				return;

			AddSpriteBillboard(&_sprites[Objects[ID_EXPLOSION_SPRITES].meshIndex + e.sprite], 
				e.pos, e.tint, e.rotation, 1.0f, { e.size, e.size }, BlendMode::Additive, true, view);
		}
	}

	void Renderer::DrawSimpleParticles(RenderView& view)
	{
		using namespace TEN::Effects;

		for (SimpleParticle& s : simpleParticles)
		{
			if (!s.active) continue;

			if (!CheckIfSlotExists(s.sequence, "Particle rendering"))
				continue;

			AddSpriteBillboard(&_sprites[Objects[s.sequence].meshIndex + s.sprite], s.worldPosition, Vector4(1, 1, 1, 1), 0, 1.0f, { s.size, s.size / 2 }, BlendMode::AlphaBlend, true, view);
		}
	}
}