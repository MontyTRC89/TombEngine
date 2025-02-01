#include "framework.h"
#include "Scripting/Internal/TEN/Effects/EffectsFunctions.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/DisplaySprite.h"
#include "Game/effects/effects.h"
#include "Game/effects/Electricity.h"
#include "Game/effects/explosion.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Utils/object_helper.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"
#include "Scripting/Internal/TEN/Effects/BlendIDs.h"
#include "Scripting/Internal/TEN/Effects/EffectIDs.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"
#include "Scripting/Internal/TEN/Types/Vec2/Vec2.h"
#include "Sound/sound.h"
#include "Specific/clock.h"
#include "Specific/trutils.h"

/// Functions to generate effects.
// @tentable Effects 
// @pragma nostrip

using namespace TEN::Effects::DisplaySprite;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Spark;
using namespace TEN::Math;

namespace TEN::Scripting::Effects
{
	///Emit a lightning arc.
	//@function EmitLightningArc
	//@tparam Vec3 src
	//@tparam Vec3 dest
	//@tparam Color color (default Color(255, 255, 255))
	//@tparam float lifetime Lifetime in seconds. Clamped to [0, 4.233] for now because of strange internal maths. (default 1.0)
	//@tparam int amplitude "strength" of the lightning - the higher the value, the "taller" the arcs. Clamped to [1, 255]. (default 20)
	//@tparam int beamWidth Clamped to [1, 127]. (default 2)
	//@tparam int detail Higher numbers equal more segments, but it's not a 1:1 correlation. Clamped to [1, 127]. (default 10)
	//@tparam bool smooth If true, the arc will have large, smooth curves; if false, it will have small, jagged spikes. (default false)
	//@tparam bool endDrift If true, the end of the arc will be able to gradually drift away from its destination in a random direction (default false)
	static void EmitLightningArc(Vec3 src, Vec3 dest, TypeOrNil<ScriptColor> color, TypeOrNil<float> lifetime, TypeOrNil<int> amplitude, TypeOrNil<int> beamWidth, TypeOrNil<int> segments, TypeOrNil<bool> smooth, TypeOrNil<bool> endDrift)
	{
		auto p1 = Vector3(src.x, src.y, src.z);
		auto p2 = Vector3(dest.x, dest.y, dest.z);

		int segs = USE_IF_HAVE(int, segments, 10);

		segs = std::clamp(segs, 1, 127);

		int width = USE_IF_HAVE(int, beamWidth, 2);

		width = std::clamp(width, 1, 127);

		// Nearest number of milliseconds equating to approx 254, the max even byte value for "life".
		// This takes into account a "hardcoded" FPS of 30 and the fact that
		// lightning loses two "life" each frame.
		constexpr auto kMaxLifeSeconds = 4.233f; 
		float life = USE_IF_HAVE(float, lifetime, 1.0f);
		life = std::clamp(life, 0.0f, kMaxLifeSeconds);

		constexpr float secsPerFrame = 1.0f / (float)FPS;

		// This will put us in the range [0, 127]
		int lifeInFrames = (int)round(life / secsPerFrame);

		// Multiply by two since a) lightning loses two "life" each frame, and b) it must be
		// an even number to avoid overshooting a value of 0 and wrapping around.
		byte byteLife = lifeInFrames * 2;

		int amp = USE_IF_HAVE(int, amplitude, 20);
		byte byteAmplitude = std::clamp(amp, 1, 255);

		bool isSmooth = USE_IF_HAVE(bool, smooth, false);
		bool isDrift = USE_IF_HAVE(bool, endDrift, false);

		char flags = 0;
		if(isSmooth)
			flags |= 1;

		if(isDrift)
			flags |= 2;

		ScriptColor col = USE_IF_HAVE(ScriptColor, color, ScriptColor( 255, 255, 255 ));

		SpawnElectricity(p1, p2, byteAmplitude, col.GetR(), col.GetG(), col.GetB(), byteLife, flags, width, segs);
	}

	/// Emit a particle.
	// @function EmitParticle
	// @tparam Vec3 pos World position.
	// @tparam Vec3 vel Velocity.
	// @tparam int spriteID ID of the sprite in the sprite sequence object.
	// @tparam float gravity Specifies if the particle will fall over time. Positive values ascend, negative values descend. Recommended range: [-1000 and 1000]. __Default: 0__
	// @tparam float rotVel Rotational velocity in degrees. __Default: 0__
	// @tparam Color startColor Color at start of life. __Default: Color(255, 255, 255)__
	// @tparam Color endColor Color to fade toward. This will finish long before the end of the particle's life due to internal math. __Default: Color(255, 255, 255)__
	// @tparam Effects.BlendID blendMode Render blend mode. __TEN.Effects.BlendID.ALPHABLEND__
	// @tparam float startSize Size at start of life. __Default: 10__
	// @tparam float endSize Size at end of life. The particle will linearly shrink or grow toward this size over its lifespan. __Default: 0__
	// @tparam float life Lifespan in seconds. __Default: 2__
	// @tparam bool applyDamage Specify if the particle will harm the player on collision. __Default: false__
	// @tparam bool applyPoison Specify if the particle will poison the player on collision. __Default: false__
	// @tparam Objects.ObjID spriteSeqID ID of the sprite sequence object. __Default: Objects.ObjID.DEFAULT_SPRITES__
	// @tparam float startRot Rotation at start of life. __Default: random__
	// @usage
	// EmitParticle(
	// 	pos,
	// 	Vec3(math.random(), math.random(), math.random()),
	// 	22, -- spriteID
	// 	0, -- gravity
	// 	-2, -- rotVel
	// 	Color(255, 0, 0), -- startColor
	// 	Color(0,  255, 0), -- endColor
	// 	TEN.Effects.BlendID.ADDITIVE, -- blendMode
	// 	15, -- startSize
	// 	50, -- endSize
	// 	20, -- life
	// 	false, -- applyDamage
	// 	true, -- applyPoison
	//  Objects.ObjID.DEFAULT_SPRITES, -- spriteSeqID
	//  180 -- startRot
	//  )
	static void EmitParticle(const Vec3& pos, const Vec3& vel, int spriteID, TypeOrNil<float> gravity, TypeOrNil<float> rotVel,
							 TypeOrNil<ScriptColor> startColor, TypeOrNil<ScriptColor> endColor, TypeOrNil<BlendMode> blendMode, 
							 TypeOrNil<float> startSize, TypeOrNil<float> endSize, TypeOrNil<float> life,
							 TypeOrNil<bool> applyDamage, TypeOrNil<bool> applyPoison, TypeOrNil<GAME_OBJECT_ID> spriteSeqID, TypeOrNil<float> startRot)
	{
		constexpr auto DEFAULT_START_SIZE = 10.0f;
		constexpr auto DEFAULT_LIFE		  = 2.0f;
		constexpr auto SECS_PER_FRAME	  = 1.0f / (float)FPS;

		static const auto DEFAULT_COLOR = ScriptColor(255, 255, 255);

		auto convertedSpriteSeqID = USE_IF_HAVE(GAME_OBJECT_ID, spriteSeqID, ID_DEFAULT_SPRITES); 
		if (!CheckIfSlotExists(convertedSpriteSeqID, "EmitParticle() script function."))
			return;

		auto& part = *GetFreeParticle();

		part.on = true;
		part.SpriteSeqID = convertedSpriteSeqID;
		part.SpriteID = spriteID;

		auto convertedBlendMode = USE_IF_HAVE(BlendMode, blendMode, BlendMode::AlphaBlend);
		part.blendMode = BlendMode(std::clamp((int)convertedBlendMode, (int)BlendMode::Opaque, (int)BlendMode::AlphaBlend));

		part.x = pos.x;
		part.y = pos.y;
		part.z = pos.z;
		part.roomNumber = FindRoomNumber(Vector3i(pos.x, pos.y, pos.z));

		part.xVel = short(vel.x * 32);
		part.yVel = short(vel.y * 32);
		part.zVel = short(vel.z * 32);

		float rotAdd = USE_IF_HAVE(float, rotVel, 0.0f);
		part.rotAng = USE_IF_HAVE(float, startRot, TO_DEGREES(Random::GenerateAngle()));
		part.rotAdd = byte(ANGLE(rotAdd) >> 4);
		
		part.sSize =
		part.size = USE_IF_HAVE(float, startSize, DEFAULT_START_SIZE);
		part.dSize = USE_IF_HAVE(float, endSize, 0.0f);
		part.scalar = 2;

		part.gravity = (short)std::clamp(USE_IF_HAVE(float, gravity, 0.0f), (float)SHRT_MIN, (float)SHRT_MAX);
		part.friction = 0;
		part.maxYvel = 0;

		auto convertedStartColor = USE_IF_HAVE(ScriptColor, startColor, DEFAULT_COLOR);
		part.sR = convertedStartColor.GetR();
		part.sG = convertedStartColor.GetG();
		part.sB = convertedStartColor.GetB();

		auto convertedEndColor = USE_IF_HAVE(ScriptColor, endColor, DEFAULT_COLOR);
		part.dR = convertedEndColor.GetR();
		part.dG = convertedEndColor.GetG();
		part.dB = convertedEndColor.GetB();

		float convertedLife = std::max(0.1f, USE_IF_HAVE(float, life, DEFAULT_LIFE));
		part.life =
		part.sLife = (int)round(convertedLife / SECS_PER_FRAME);
		part.colFadeSpeed = part.life / 2;
		part.fadeToBlack = part.life / 3;

		part.flags = SP_SCALE | SP_ROTATE | SP_DEF | SP_EXPDEF;

		bool convertedApplyPoison = USE_IF_HAVE(bool, applyPoison, false);
		if (convertedApplyPoison)
			part.flags |= SP_POISON;

		bool convertedApplyDamage = USE_IF_HAVE(bool, applyDamage, false);
		if (convertedApplyDamage)
			part.flags |= SP_DAMAGE;

		// TODO: Add option to turn off wind.
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WIND, part.roomNumber))
			part.flags |= SP_WIND;
	}
	
/***Emit a shockwave, similar to that seen when a harpy projectile hits something.
	@function EmitShockwave
	@tparam Vec3 pos Origin position
	@tparam int innerRadius (default 0) Initial inner radius of the shockwave circle - 128 will be approx a click, 512 approx a block
	@tparam int outerRadius (default 128) Initial outer radius of the shockwave circle
	@tparam Color color (default Color(255, 255, 255))
	@tparam float lifetime (default 1.0) Lifetime in seconds (max 8.5 because of inner maths weirdness)
	@tparam int speed (default 50) Initial speed of the shockwave's expansion (the shockwave will always slow as it goes)
	@tparam int angle (default 0) Angle about the X axis - a value of 90 will cause the shockwave to be entirely vertical
	@tparam bool hurtsLara (default false) If true, the shockwave will hurt Lara, with the damage being relative to the shockwave's current speed
*/
	static void EmitShockwave(Vec3 pos, TypeOrNil<int> innerRadius, TypeOrNil<int> outerRadius, TypeOrNil<ScriptColor> col,
							  TypeOrNil<float> lifetime, TypeOrNil<int> speed, TypeOrNil<int> angle, TypeOrNil<bool> hurtPlayer)
	{
		constexpr auto LIFE_IN_SECONDS_MAX = 8.5f;
		constexpr auto SECONDS_PER_FRAME   = 1 / (float)FPS;

		auto pose = Pose(Vector3i(pos.x, pos.y, pos.z));

		int innerRad = USE_IF_HAVE(int, innerRadius, 0);
		int outerRad = USE_IF_HAVE(int, outerRadius, 128);

		auto color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		int spd = USE_IF_HAVE(int, speed, 50);
		int ang = USE_IF_HAVE(int, angle, 0);
 
		float life = USE_IF_HAVE(float, lifetime, 1.0f);
		life = std::clamp(life, 0.0f, LIFE_IN_SECONDS_MAX);

		// Normalize to range [0, 255].
		int lifeInFrames = (int)round(life / SECONDS_PER_FRAME);

		bool doDamage = USE_IF_HAVE(bool, hurtPlayer, false);

		TriggerShockwave(
			&pose, innerRad, outerRad, spd,
			color.GetR(), color.GetG(), color.GetB(),
			lifeInFrames, EulerAngles(ANGLE(ang), 0.0f, 0.0f),
			(short)doDamage, true, false, false, (int)ShockwaveStyle::Normal);
	}

/***Emit dynamic light that lasts for a single frame.
 * If you want a light that sticks around, you must call this each frame.
@function EmitLight
@tparam Vec3 pos position of the light
@tparam[opt] Color color light color (default Color(255, 255, 255))
@tparam[opt] int radius measured in "clicks" or 256 world units (default 20)
@tparam[opt] bool shadows determines whether light should generate dynamic shadows for applicable moveables (default is false)
@tparam[opt] string name if provided, engine will interpolate this light for high framerate mode (be careful not to use same name for different lights)
*/
	static void EmitLight(Vec3 pos, TypeOrNil<ScriptColor> col, TypeOrNil<int> radius, TypeOrNil<bool> castShadows, TypeOrNil<std::string> name)
	{
		auto color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		int rad = (float)(USE_IF_HAVE(int, radius, 20) * BLOCK(0.25f));
		SpawnDynamicPointLight(pos.ToVector3(), color, rad, USE_IF_HAVE(bool, castShadows, false), GetHash(USE_IF_HAVE(std::string, name, std::string())));
	}

/***Emit dynamic directional spotlight that lasts for a single frame.
* If you want a light that sticks around, you must call this each frame.
@function EmitSpotLight
@tparam Vec3 pos position of the light
@tparam Vec3 dir direction, or a point to which spotlight should be directed to
@tparam[opt] Color color (default Color(255, 255, 255))
@tparam[opt] int radius overall radius at the endpoint of a light cone, measured in "clicks" or 256 world units (default 10)
@tparam[opt] int falloff radius, at which light starts to fade out, measured in "clicks" (default 5)
@tparam[opt] int distance distance, at which light cone fades out, measured in "clicks" (default 20)
@tparam[opt] bool shadows determines whether light should generate dynamic shadows for applicable moveables (default is false)
@tparam[opt] string name if provided, engine will interpolate this light for high framerate mode (be careful not to use same name for different lights)
*/
	static void EmitSpotLight(Vec3 pos, Vec3 dir, TypeOrNil<ScriptColor> col, TypeOrNil<int> radius, TypeOrNil<int> falloff, TypeOrNil<int> distance, TypeOrNil<bool> castShadows, TypeOrNil<std::string> name)
	{
		auto color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		int rad =	  (float)(USE_IF_HAVE(int, radius,   10) * BLOCK(0.25f));
		int fallOff = (float)(USE_IF_HAVE(int, falloff,   5) * BLOCK(0.25f));
		int dist =	  (float)(USE_IF_HAVE(int, distance, 20) * BLOCK(0.25f));
		SpawnDynamicSpotLight(pos.ToVector3(), dir.ToVector3(), color, rad, fallOff, dist, USE_IF_HAVE(bool, castShadows, false), GetHash(USE_IF_HAVE(std::string, name, std::string())));
	}

/***Emit blood.
@function EmitBlood
@tparam Vec3 pos
@tparam int count (default 1) "amount" of blood. Higher numbers won't add more blood but will make it more "flickery", with higher numbers turning it into a kind of red orb.
*/
	static void EmitBlood(Vec3 pos, TypeOrNil<int> num)
	{
		TriggerBlood(pos.x, pos.y, pos.z, -1, USE_IF_HAVE(int, num, 1));
	}

/***Emit fire for one frame. Will not hurt Lara. Call this each frame if you want a continuous fire.
@function EmitFire
@tparam Vec3 pos
@tparam float size (default 1.0)
*/
	static void EmitFire(Vec3 pos, TypeOrNil<float> size)
	{
		AddFire(pos.x, pos.y, pos.z, FindRoomNumber(Vector3i(pos.x, pos.y, pos.z)), USE_IF_HAVE(float, size, 1));
	}

/***Make an explosion. Does not hurt Lara
@function MakeExplosion 
@tparam Vec3 pos
@tparam float size (default 512.0) this will not be the size of the sprites, but rather the distance between the origin and any additional sprites
@tparam bool shockwave (default false) if true, create a very faint white shockwave which will not hurt Lara
*/
	static void MakeExplosion(Vec3 pos, TypeOrNil<float> size, TypeOrNil<bool> shockwave)
	{
		TriggerExplosion(Vector3(pos.x, pos.y, pos.z), USE_IF_HAVE(float, size, 512.0f), true, false, USE_IF_HAVE(bool, shockwave, false), FindRoomNumber(Vector3i(pos.x, pos.y, pos.z)));
	}

/***Make an earthquake
@function MakeEarthquake 
@tparam int strength (default 100) How strong should the earthquake be? Increasing this value also increases the lifespan of the earthquake.
*/
	static void Earthquake(TypeOrNil<int> strength)
	{
		int str = USE_IF_HAVE(int, strength, 100);
		Camera.bounce = -str;
	}

	/// Get the wind vector for the current game frame.
	// This represents the 3D displacement applied by the engine on things like particles affected by wind.
	// @function GetWind()
	// @treturn Vec3 Wind vector.
	static Vec3 GetWind()
	{
		return Vec3(Weather.Wind());
	}

	void Register(sol::state* state, sol::table& parent) 
	{
		auto tableEffects = sol::table(state->lua_state(), sol::create);
		parent.set(ScriptReserved_Effects, tableEffects);

		tableEffects.set_function(ScriptReserved_EmitLightningArc, &EmitLightningArc);
		tableEffects.set_function(ScriptReserved_EmitParticle, &EmitParticle);
		tableEffects.set_function(ScriptReserved_EmitShockwave, &EmitShockwave);
		tableEffects.set_function(ScriptReserved_EmitLight, &EmitLight);
		tableEffects.set_function(ScriptReserved_EmitSpotLight, &EmitSpotLight);
		tableEffects.set_function(ScriptReserved_EmitBlood, &EmitBlood);
		tableEffects.set_function(ScriptReserved_MakeExplosion, &MakeExplosion);
		tableEffects.set_function(ScriptReserved_EmitFire, &EmitFire);
		tableEffects.set_function(ScriptReserved_MakeEarthquake, &Earthquake);
		tableEffects.set_function(ScriptReserved_GetWind, &GetWind);

		auto handler = LuaHandler{ state };
		handler.MakeReadOnlyTable(tableEffects, ScriptReserved_BlendID, BLEND_IDS);
		handler.MakeReadOnlyTable(tableEffects, ScriptReserved_EffectID, EFFECT_IDS);
	}
}
