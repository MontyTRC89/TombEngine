#include "framework.h"
#include "ReservedScriptNames.h"
#include "ScriptUtil.h"
#include "Vec3/Vec3.h"
#include "Color/Color.h"
#include "Game/camera.h"
#include "Game/spotcam.h"
#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/effects/lightning.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/effects/explosion.h"
#include "Game/effects/spark.h"
#include "Game/effects/weather.h"
#include "Sound/sound.h"
#include "Specific/configuration.h"
#include "Specific/input.h"
#include "Specific/setup.h"

#include "BlendIDs.h"
#include "LuaHandler.h"

/***
Functions that don't fit in the other modules.
@tentable Misc 
@pragma nostrip
*/

using namespace TEN::Input;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Spark;

namespace Misc 
{
	[[nodiscard]] static bool HasLineOfSight(short roomNumber1, Vec3 pos1, Vec3 pos2)
	{
		GameVector vec1, vec2;
		pos1.StoreInGameVector(vec1);
		vec1.roomNumber = roomNumber1;
		pos2.StoreInGameVector(vec2);
		return LOS(&vec1, &vec2);
	}

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
		Vector3Int p1;
		p1.x = src.x;
		p1.y = src.y;
		p1.z = src.z;

		Vector3Int p2;
		p2.x = dest.x;
		p2.y = dest.y;
		p2.z = dest.z;

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

		TriggerLightning(&p1, &p2, byteAmplitude, col.GetR(), col.GetG(), col.GetB(), byteLife, flags, width, segs);
	}

	/*** Emit a particle.
	 See the sprite editor in WadTool for DEFAULT_SPRITES to see a list of sprite indices.
	@function EmitParticle
	@tparam Vec3 pos
	@tparam Vec3 velocity
	@tparam int spriteIndex an index of a sprite in DEFAULT_SPRITES object.
	@tparam int gravity (default 0) Specifies whether particle will fall (positive values) or ascend (negative values) over time. Clamped to [-32768, 32767], but values between -1000 and 1000 are recommended; values too high or too low (e.g. under -2000 or above 2000) will cause the velocity of the particle to "wrap around" and switch directions.
	@tparam float rot (default 0) specifies a speed with which it will rotate (0 = no rotation, negative = anticlockwise rotation, positive = clockwise rotation).
	@tparam Color startColor (default Color(255, 255, 255)) color at start of life
	@tparam Color endColor (default Color(255, 255, 255)) color to fade to - at the time of writing this fade will finish long before the end of the particle's life due to internal maths
	@tparam BlendID blendMode (default TEN.Misc.BlendID.ALPHABLEND) How will we blend this with its surroundings?
	@tparam int startSize (default 10) Size on spawn. A value of 15 is approximately the size of Lara's head.
	@tparam int endSize (default 0) Size on death - the particle will linearly shrink or grow to this size during its lifespan
	@tparam float lifetime (default 2) Lifespan in seconds 
	@tparam bool damage (default false) specifies whether particle can damage Lara (does a very small amount of damage, like the small lava emitters in TR1)
	@tparam bool poison (default false) specifies whether particle can poison Lara
	@usage
	EmitParticle(
		yourPositionVarHere,
		Vec3(math.random(), math.random(), math.random()),
		22, -- spriteIndex
		0, -- gravity
		-2, -- rot
		Color(255, 0, 0), -- startColor
		Color(0,  255, 0), -- endColor
		TEN.Misc.BlendID.ADDITIVE, -- blendMode
		15, -- startSize
		50, -- endSize
		20, -- lifetime
		false, -- damage
		true -- poison
		)
	*/
	static void EmitParticle(Vec3 pos, Vec3 velocity, int spriteIndex, TypeOrNil<int> gravity, TypeOrNil<float> rot, 
							TypeOrNil<ScriptColor> startColor, TypeOrNil<ScriptColor> endColor, TypeOrNil<BLEND_MODES> blendMode, 
							TypeOrNil<int> startSize, TypeOrNil<int> endSize, TypeOrNil<float> lifetime, 
							TypeOrNil<bool> damage, TypeOrNil<bool> poison)
	{
		if (!Objects[ID_DEFAULT_SPRITES].loaded)
		{
			TENLog("Can't spawn a particle because sprites are not loaded for this level.", LogLevel::Error);
			return;
		}

		int grav = USE_IF_HAVE(int, gravity, 0);

		grav = std::clamp(grav, -32768, 32767);

		auto* s = GetFreeParticle();

		s->on = true;

		s->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + spriteIndex;

		ScriptColor sCol = USE_IF_HAVE(ScriptColor, startColor, ScriptColor( 255, 255, 255 ));
		ScriptColor eCol = USE_IF_HAVE(ScriptColor, endColor, ScriptColor( 255, 255, 255 ));

		s->sR = sCol.GetR();
		s->sG = sCol.GetG();
		s->sB = sCol.GetB();

		s->dR = eCol.GetR();
		s->dG = eCol.GetG();
		s->dB = eCol.GetB();

		//there is no blend mode 7
		BLEND_MODES bMode = USE_IF_HAVE(BLEND_MODES, blendMode, BLENDMODE_ALPHABLEND);
		s->blendMode = BLEND_MODES(std::clamp(int(bMode), int(BLEND_MODES::BLENDMODE_OPAQUE), int(BLEND_MODES::BLENDMODE_ALPHABLEND)));

		s->x = pos.x;
		s->y = pos.y;
		s->z = pos.z;
		s->roomNumber = FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z));
		constexpr float secsPerFrame = 1.0f / (float)FPS;

		float life = USE_IF_HAVE(float, lifetime, 2.0f);
		life = std::max(0.0f, life);
		int lifeInFrames = (int)round(life / secsPerFrame);

		s->life = s->sLife = lifeInFrames;
		s->colFadeSpeed = lifeInFrames / 2;
		s->fadeToBlack = lifeInFrames / 3;

		s->xVel = short(velocity.x << 5);
		s->yVel = short(velocity.y << 5);
		s->zVel = short(velocity.z << 5);

		int sSize = USE_IF_HAVE(int, startSize, 10);
		int eSize = USE_IF_HAVE(int, endSize, 0);

		s->sSize = s->size = float(sSize);
		s->dSize = float(eSize);

		s->scalar = 2;

		s->flags = SP_SCALE | SP_ROTATE | SP_DEF | SP_EXPDEF;

		bool applyPoison = USE_IF_HAVE(bool, poison, false);
		bool applyDamage = USE_IF_HAVE(bool, damage, false);

		if (applyPoison)
			s->flags |= SP_POISON;

		if (applyDamage)
			s->flags |= SP_DAMAGE;

		//todo add option to turn off wind?
		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WIND, s->roomNumber))
			s->flags |= SP_WIND;

		float rotAdd = USE_IF_HAVE(float, rot, 0.0f);

		s->rotAng = (GetRandomControl() & 0x0FFF); 
		s->rotAdd = byte(ANGLE(rotAdd) >> 4);

		s->friction = 0;
		s->maxYvel  = 0;

		s->gravity  = grav;
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
	static void EmitShockwave(Vec3 pos, TypeOrNil<int> innerRadius, TypeOrNil<int> outerRadius, TypeOrNil<ScriptColor> col, TypeOrNil<float> lifetime, TypeOrNil<int> speed, TypeOrNil<int> angle, TypeOrNil<bool> hurtsLara)
	{
		PHD_3DPOS p;
		p.Position.x = pos.x;
		p.Position.y = pos.y;
		p.Position.z = pos.z;

		int iRad = USE_IF_HAVE(int, innerRadius, 0);
		int oRad = USE_IF_HAVE(int, outerRadius, 128);

		ScriptColor color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));

		int spd = USE_IF_HAVE(int, speed, 50);

		int ang = USE_IF_HAVE(int, angle, 0);

		constexpr auto kMaxLifeSeconds = 8.5f; 
		float life = USE_IF_HAVE(float, lifetime, 1.0f);
		life = std::clamp(life, 0.0f, kMaxLifeSeconds);

		constexpr float secsPerFrame = 1.0f / (float)FPS;

		// This will put us in the range [0, 255]
		int lifeInFrames = (int)round(life / secsPerFrame);

		bool damage = USE_IF_HAVE(bool, hurtsLara, false);

		TriggerShockwave(&p, iRad, oRad, spd, color.GetR(), color.GetG(), color.GetB(), lifeInFrames, FROM_DEGREES(ang), short(damage));
	}

/***Emit dynamic light that lasts for a single frame.
 * If you want a light that sticks around, you must call this each frame.
@function EmitLight
@tparam Vec3 pos
@tparam Color color (default Color(255, 255, 255))
@tparam int radius (default 20) corresponds loosely to both intensity and range
*/
	static void EmitLight(Vec3 pos, TypeOrNil<ScriptColor> col, TypeOrNil<int> radius)
	{
		ScriptColor color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		int rad = USE_IF_HAVE(int, radius, 20);
		TriggerDynamicLight(pos.x, pos.y, pos.z, rad, color.GetR(), color.GetG(), color.GetB());
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

		AddFire(pos.x, pos.y, pos.z, FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z)), USE_IF_HAVE(float, size, 1), 0);
	}

/***Make an explosion. Does not hurt Lara
@function MakeExplosion 
@tparam Vec3 pos
@tparam float size (default 512.0) this will not be the size of the sprites, but rather the distance between the origin and any additional sprites
@tparam bool shockwave (default false) if true, create a very faint white shockwave which will not hurt Lara
*/
	static void MakeExplosion(Vec3 pos, TypeOrNil<float> size, TypeOrNil<bool> shockwave)
	{
		TriggerExplosion(Vector3(pos.x, pos.y, pos.z), USE_IF_HAVE(float, size, 512.0f), true, false, USE_IF_HAVE(bool, shockwave, false), FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z)));
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

/***Flash screen.
@function FlashScreen
@tparam Color color (default Color(255, 255, 255))
@tparam float speed (default 1.0). Speed in "amount" per second. A value of 1 will make the flash take one second. Clamped to [0.005, 1.0]
*/
	static void FlashScreen(TypeOrNil<ScriptColor> col, TypeOrNil<float> speed)
	{
		ScriptColor color = USE_IF_HAVE(ScriptColor, col, ScriptColor(255, 255, 255));
		Weather.Flash(color.GetR(), color.GetG(), color.GetB(), (USE_IF_HAVE(float, speed, 1.0))/ float(FPS));
	}

	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}


	static void FadeIn(sol::optional<float> speed)
	{
		SetScreenFadeIn(speed.value_or(1.0f) / float(FPS));
	}

	static void FadeOut(sol::optional<float> speed)
	{
		SetScreenFadeOut(speed.value_or(1.0f) / float(FPS));
	}

	static void SetCineBars(float height, sol::optional<float> speed)
	{
		SetCinematicBars(height / float(REFERENCE_RES_HEIGHT), speed.value_or(1.0f) / float(FPS));
	}

	static void SetFOV(float angle)
	{
		AlterFOV(ANGLE(std::clamp(abs(angle), 10.0f, 170.0f)));
	}

	static void PlayAudioTrack(std::string const& trackName, sol::optional<bool> looped)
	{
		auto mode = looped.value_or(false) ? SoundTrackType::OneShot : SoundTrackType::BGM;
		PlaySoundTrack(trackName, mode);
	}

	static void PlaySoundEffect(int id, sol::optional<Vec3> p)
	{
		SoundEffect(id, p.has_value() ? &PHD_3DPOS(p.value().x, p.value().y, p.value().z) : nullptr, SoundEnvironment::Always);
	}

	static void SetAmbientTrack(std::string const& trackName)
	{
		PlaySoundTrack(trackName, SoundTrackType::BGM);
	}

	static bool KeyHeld(int actionIndex)
	{
		return (TrInput & (1 << actionIndex)) != 0;
	}

	static bool KeyHit(int actionIndex)
	{
		return (DbInput & (1 << actionIndex)) != 0;
	}

	static void KeyPush(int actionIndex)
	{
		TrInput |= (1 << actionIndex);
	}

	static void KeyClear(int actionIndex)
	{
		TrInput &= ~(1 << actionIndex);
	}

	static int CalculateDistance(Vec3 const& pos1, Vec3 const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	static int CalculateHorizontalDistance(Vec3 const& pos1, Vec3 const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	static std::tuple<int, int> PercentToScreen(double x, double y)
	{
		auto fWidth = static_cast<double>(g_Configuration.Width);
		auto fHeight = static_cast<double>(g_Configuration.Height);
		int resX = static_cast<int>(std::round(fWidth / 100.0 * x));
		int resY = static_cast<int>(std::round(fHeight / 100.0 * y));
		//todo this still assumes a resolution of 800/600. account for this somehow
		return std::make_tuple(resX, resY);
	}

	static std::tuple<double, double> ScreenToPercent(int x, int y)
	{
		auto fWidth = static_cast<double>(g_Configuration.Width);
		auto fHeight = static_cast<double>(g_Configuration.Height);
		double resX = x / fWidth * 100.0;
		double resY = y / fHeight * 100.0;
		return std::make_tuple(resX, resY);
	}


	void Register(sol::state * state, sol::table & parent) {
		sol::table table_misc{ state->lua_state(), sol::create };
		parent.set(ScriptReserved_Misc, table_misc);

		table_misc.set_function(ScriptReserved_EmitLightningArc, &EmitLightningArc);
		table_misc.set_function(ScriptReserved_EmitParticle, &EmitParticle);
		table_misc.set_function(ScriptReserved_EmitShockwave, &EmitShockwave);
		table_misc.set_function(ScriptReserved_EmitLight, &EmitLight);
		table_misc.set_function(ScriptReserved_EmitBlood, &EmitBlood);

		table_misc.set_function(ScriptReserved_MakeExplosion, &MakeExplosion);

		table_misc.set_function(ScriptReserved_EmitFire, &EmitFire);

		///Do earthquake.
		//@function Earthquake
		//@tparam int strength
		table_misc.set_function(ScriptReserved_MakeEarthquake, &Earthquake);

		///Vibrate gamepad, if possible.
		//@function Vibrate
		//@tparam float strength
		//@tparam float time (in seconds, default: 0.3)
		table_misc.set_function(ScriptReserved_Vibrate, &Vibrate);

		table_misc.set_function(ScriptReserved_FlashScreen, &FlashScreen);

		///Do a fade-in.
		//@function FadeIn
		//@tparam float speed (default: 1 second)
		table_misc.set_function(ScriptReserved_FadeIn, &FadeIn);

		///Do a fade-out.
		//@function FadeIn
		//@tparam float speed (default: 1 second)
		table_misc.set_function(ScriptReserved_FadeOut, &FadeOut);

		///Set cinematic bars.
		//@function SetCineBars
		//@tparam float height on the range 0-800
		//@tparam float speed (default: 1 second)
		table_misc.set_function(ScriptReserved_SetCineBars, &SetCineBars);

		///Set field of view.
		//@function SetFOV
		//@tparam float angle on the range 10-170 degrees
		table_misc.set_function(ScriptReserved_SetFOV, &SetFOV);

		///Set and play an ambient track
		//@function SetAmbientTrack
		//@tparam string name of track (without file extension) to play
		table_misc.set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		/// Play an audio track
		//@function PlayAudioTrack
		//@tparam string name of track (without file extension) to play
		//@tparam bool loop if true, the track will loop; if false, it won't (default: false)
		table_misc.set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);

		/// Play sound effect
		//@function PlaySound
		//@tparam int sound ID to play
		//@tparam Vec3 position
		table_misc.set_function(ScriptReserved_PlaySound, &PlaySoundEffect);

		/// Check if particular action key is held
		//@function KeyHeld
		//@tparam int action mapping index to check
		table_misc.set_function(ScriptReserved_KeyHeld, &KeyHeld);

		/// Check if particular action key was hit (once)
		//@function KeyHit
		//@tparam int action mapping index to check
		table_misc.set_function(ScriptReserved_KeyHit, &KeyHit);

		/// Emulate pushing of a certain action key
		//@function KeyPush
		//@tparam int action mapping index to push
		table_misc.set_function(ScriptReserved_KeyPush, &KeyPush);

		/// Clears particular input from action key
		//@function KeyClear
		//@tparam int action mapping index to clear
		table_misc.set_function(ScriptReserved_KeyClear, &KeyClear);
	
		///Calculate the distance between two positions.
		//@function CalculateDistance
		//@tparam Vec3 posA first position
		//@tparam Vec3 posB second position
		//@treturn int the direct distance from one position to the other
		table_misc.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

		///Calculate the horizontal distance between two positions.
		//@function CalculateHorizontalDistance
		//@tparam Vec3 posA first position
		//@tparam Vec3 posB second position
		//@treturn int the direct distance on the XZ plane from one position to the other
		table_misc.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);

		///Translate a pair of percentages to screen-space pixel coordinates.
		//To be used with @{Strings.DisplayString:SetPosition} and @{Strings.DisplayString}.
		//@function PercentToScreen
		//@tparam float x percent value to translate to x-coordinate
		//@tparam float y percent value to translate to y-coordinate
		//@treturn int x coordinate in pixels
		//@treturn int y coordinate in pixels
		table_misc.set_function(ScriptReserved_PercentToScreen, &PercentToScreen);

		///Determine if there's a line of sight between two points.
		//
		//i.e. if we run a direct line from one position to another
		//will any geometry get in the way?
		//
		//Note: if you use this with Moveable:GetPosition to test if (for example)
		//two creatures can see one another, you might have to do some extra adjustments.
		//
		//This is because the "position" for most objects refers to its base, i.e., the floor.
		//As a solution, you can increase the y-coordinate of this position to correspond to roughly where the
		//eyes of the creatures would be.
		//@function HasLineOfSight
		//@tparam float room1 ID of the room where the first position is
		//@tparam Vec3 pos1 first position
		//@tparam Vec3 pos2 second position
		//@treturn bool is there a direct line of sight between the two positions?
		//@usage
		//local flamePlinthPos = flamePlinth:GetPosition() + Vec3(0, flamePlinthHeight, 0);
		//print(Misc.HasLineOfSight(enemyHead:GetRoom(), enemyHead:GetPosition(), flamePlinthPos))
		table_misc.set_function(ScriptReserved_HasLineOfSight, &HasLineOfSight);


		///Translate a pair of coordinates to percentages of window dimensions.
		//To be used with @{Strings.DisplayString:GetPosition}.
		//@function ScreenToPercent
		//@tparam int x pixel value to translate to a percentage of the window width
		//@tparam int y pixel value to translate to a percentage of the window height
		//@treturn float x coordinate as percentage
		//@treturn float y coordinate as percentage
		table_misc.set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);

		LuaHandler handler{ state };
		handler.MakeReadOnlyTable(table_misc, ScriptReserved_BlendID, kBlendIDs);
	}
}
