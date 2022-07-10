#include "framework.h"
#include "ReservedScriptNames.h"
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

	static void AddLightningArc(Vec3 src, Vec3 dest, ScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
	{
		Vector3Int p1;
		p1.x = src.x;
		p1.y = src.y;
		p1.z = src.z;

		Vector3Int p2;
		p2.x = dest.x;
		p2.y = dest.y;
		p2.z = dest.z;

		TriggerLightning(&p1, &p2, amplitude, color.GetR(), color.GetG(), color.GetB(), lifetime, flags, beamWidth, segments);
	}

	static void AddParticle(int spriteIndex, Vec3 pos, Vec3 velocity, int gravity, float rot, 
							ScriptColor startColor, ScriptColor endColor, int blendMode, 
							int startSize, int endSize, int lifetime, 
							bool damage, bool poison)
	{
		if (!Objects[ID_DEFAULT_SPRITES].loaded)
		{
			TENLog("Can't spawn a particle because sprites are not loaded for this level.", LogLevel::Error);
			return;
		}

		auto* s = GetFreeParticle();

		s->on = true;

		s->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + spriteIndex;

		s->sR = startColor.GetR();
		s->sG = startColor.GetG();
		s->sB = startColor.GetB();
		s->dR = endColor.GetR();
		s->dG = endColor.GetG();
		s->dB = endColor.GetB();

		s->blendMode = BLEND_MODES(std::clamp(blendMode, int(BLEND_MODES::BLENDMODE_OPAQUE), int(BLEND_MODES::BLENDMODE_ALPHABLEND)));

		s->x = pos.x;
		s->y = pos.y;
		s->z = pos.z;
		s->roomNumber = FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z));

		s->life = s->sLife = lifetime;
		s->colFadeSpeed = lifetime / 2;
		s->fadeToBlack = lifetime / 3;

		s->xVel = short(velocity.x << 5);
		s->yVel = short(velocity.y << 5);
		s->zVel = short(velocity.z << 5);

		s->sSize = s->size = float(startSize);
		s->dSize = float(endSize);
		s->scalar = 2;

		s->flags = SP_SCALE | SP_ROTATE | SP_DEF | SP_EXPDEF;

		if (poison)
			s->flags |= SP_POISON;

		if (damage)
			s->flags |= SP_DAMAGE;

		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WIND, s->roomNumber))
			s->flags |= SP_WIND;

		s->rotAng = (GetRandomControl() & 0x0FFF); 
		s->rotAdd = byte(ANGLE(rot) >> 4);

		s->friction = 0;
		s->maxYvel  = 0;
		s->gravity  = gravity;
	}

	static void AddShockwave(Vec3 pos, int innerRadius, int outerRadius, ScriptColor color, int lifetime, int speed, int angle, int flags)
	{
		PHD_3DPOS p;
		p.Position.x = pos.x;
		p.Position.y = pos.y;
		p.Position.z = pos.z;

		TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, FROM_DEGREES(angle), flags);
	}

	static void AddLight(Vec3 pos, ScriptColor color, int radius)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, radius, color.GetR(), color.GetG(), color.GetB());
	}

	static void AddBlood(Vec3 pos, int num)
	{
		TriggerBlood(pos.x, pos.y, pos.z, -1, num);
	}

	static void AddFireFlame(Vec3 pos, float size)
	{
		AddFire(pos.x, pos.y, pos.z, FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z)), size, 0);
	}

	static void AddExplosion(Vec3 pos, sol::optional<float> size, sol::optional<bool> shockwave)
	{
		TriggerExplosion(Vector3(pos.x, pos.y, pos.z), size.value_or(512.0f), true, false, shockwave.value_or(false), FindRoomNumber(Vector3Int(pos.x, pos.y, pos.z)));
	}

	static void Earthquake(int strength)
	{
		Camera.bounce = -strength;
	}

	static void Vibrate(float strength, sol::optional<float> time)
	{
		Rumble(strength, time.value_or(0.3f), RumbleMode::Both);
	}

	static void FlashScreen(ScriptColor color, sol::optional<float> speed)
	{
		Weather.Flash(color.GetR(), color.GetG(), color.GetB(), speed.value_or(0.5f) / float(FPS));
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

		///Emit a lightning arc.
		//@function AddLightningArc
		//@tparam Vec3 src
		//@tparam Vec3 dest
		//@tparam ScriptColor color
		//@tparam int lifetime
		//@tparam int amplitude
		//@tparam int beamWidth
		//@tparam int segments
		//@tparam int flags
		table_misc.set_function(ScriptReserved_AddLightningArc, &AddLightningArc);

		///Emit a particle.
		//@function AddLightningArc
		//@tparam Vec3 pos
		//@tparam Vec3 velocity
		//@tparam int gravity
		//@tparam float rot
		//@tparam ScriptColor startColor
		//@tparam ScriptColor endColor
		//@tparam int blendMode
		//@tparam int startSize
		//@tparam int endSize
		//@tparam int lifetime
		//@tparam bool damage
		//@tparam bool poison
		table_misc.set_function(ScriptReserved_AddParticle, &AddParticle);

		///Emit a shockwave.
		//@function AddShockwave
		//@tparam Vec3 pos
		//@tparam int innerRadius
		//@tparam int outerRadius
		//@tparam ScriptColor color
		//@tparam int lifetime
		//@tparam int speed
		//@tparam int angle
		//@tparam int flags
		table_misc.set_function(ScriptReserved_AddShockwave, &AddShockwave);

		///Emit dynamic light.
		//@function AddLight
		//@tparam Vec3 pos
		//@tparam ScriptColor color
		//@tparam int radius
		table_misc.set_function(ScriptReserved_AddLight, &AddLight);

		///Emit blood.
		//@function AddFire
		//@tparam Vec3 pos
		//@tparam int size
		table_misc.set_function(ScriptReserved_AddFire, &AddFireFlame);

		///Emit explosion.
		//@function AddExplosion
		//@tparam Vec3 pos
		//@tparam float size
		//@tparam bool shockwave (default: false)
		table_misc.set_function(ScriptReserved_AddExplosion, &AddExplosion);

		///Emit blood.
		//@function AddBlood
		//@tparam Vec3 pos
		//@tparam int amount
		table_misc.set_function(ScriptReserved_AddBlood, &AddBlood);

		///Do earthquake.
		//@function Earthquake
		//@tparam int strength
		table_misc.set_function(ScriptReserved_Earthquake, &Earthquake);

		///Vibrate gamepad, if possible.
		//@function Vibrate
		//@tparam float strength
		//@tparam float time (in seconds, default: 0.3)
		table_misc.set_function(ScriptReserved_Vibrate, &Vibrate);

		///Flash screen.
		//@function FlashScreen
		//@tparam ScriptColor color
		//@tparam float speed (default: 1 second)
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
	}
}
