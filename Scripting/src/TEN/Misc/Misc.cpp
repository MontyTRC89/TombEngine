#include "frameworkandsol.h"
#include "ReservedScriptNames.h"
#include "Sound\sound.h"
#include "Position/Position.h"
#include "Color/Color.h"
#include "Game\effects\lightning.h"
#include "effects\tomb4fx.h"
#include "effects\effects.h"
#include "control/los.h"
#include "Specific/configuration.h"
#include "camera.h"

/***
Scripts that will be run on game startup.
@tentable Misc 
@pragma nostrip
*/

using namespace TEN::Effects::Lightning;

namespace Misc {
	[[nodiscard]] static bool HasLineOfSight(short roomNumber1, Position pos1, Position pos2)
	{
		GAME_VECTOR vec1, vec2;
		pos1.StoreInGameVector(vec1);
		vec1.roomNumber = roomNumber1;
		pos2.StoreInGameVector(vec2);
		return LOS(&vec1, &vec2);
	}

	static int FindRoomNumber(Position pos)
	{
		return 0;
	}

	static void AddLightningArc(Position src, Position dest, ScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
	{
		PHD_VECTOR p1;
		p1.x = src.x;
		p1.y = src.y;
		p1.z = src.z;

		PHD_VECTOR p2;
		p2.x = dest.x;
		p2.y = dest.y;
		p2.z = dest.z;

		TriggerLightning(&p1, &p2, amplitude, color.GetR(), color.GetG(), color.GetB(), lifetime, flags, beamWidth, segments);
	}

	static void AddShockwave(Position pos, int innerRadius, int outerRadius, ScriptColor color, int lifetime, int speed, int angle, int flags)
	{
		PHD_3DPOS p;
		p.xPos = pos.x;
		p.yPos = pos.y;
		p.zPos = pos.z;

		TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, FROM_DEGREES(angle), flags);
	}

	static void AddDynamicLight(Position pos, ScriptColor color, int radius, int lifetime)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, radius, color.GetR(), color.GetG(), color.GetB());
	}

	static void AddBlood(Position pos, int num)
	{
		TriggerBlood(pos.x, pos.y, pos.z, -1, num);
	}

	static void AddFireFlame(Position pos, int size)
	{
		AddFire(pos.x, pos.y, pos.z, size, FindRoomNumber(pos), true);
	}

	static void Earthquake(int strength)
	{
		Camera.bounce = -strength;
	}

	static void PlayAudioTrack(std::string const& trackName, sol::optional<bool> looped)
	{
		auto mode = looped.value_or(false) ? SOUNDTRACK_PLAYTYPE::OneShot : SOUNDTRACK_PLAYTYPE::BGM;
		PlaySoundTrack(trackName, mode);
	}

	static void PlaySoundEffect(int id, Position p, int flags)
	{
		PHD_3DPOS pos;

		pos.xPos = p.x;
		pos.yPos = p.y;
		pos.zPos = p.z;
		pos.xRot = 0;
		pos.yRot = 0;
		pos.zRot = 0;

		SoundEffect(id, &pos, flags);
	}

	static void PlaySoundEffect(int id, int flags)
	{
		SoundEffect(id, NULL, flags);
	}

	static void SetAmbientTrack(std::string const& trackName)
	{
		PlaySoundTrack(trackName, SOUNDTRACK_PLAYTYPE::BGM);
	}

	static int CalculateDistance(Position const& pos1, Position const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	static int CalculateHorizontalDistance(Position const& pos1, Position const& pos2)
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

		///Set and play an ambient track
		//@function SetAmbientTrack
		//@tparam string name of track (without file extension) to play
		table_misc.set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		/// Play an audio track
		//@function PlayAudioTrack
		//@tparam string name of track (without file extension) to play
		//@tparam bool loop if true, the track will loop; if false, it won't (default: false)
		table_misc.set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);
	
		///Calculate the distance between two positions.
		//@function CalculateDistance
		//@tparam Position posA first position
		//@tparam Position posB second position
		//@treturn int the direct distance from one position to the other
		table_misc.set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

		
		///Calculate the horizontal distance between two positions.
		//@function CalculateHorizontalDistance
		//@tparam Position posA first position
		//@tparam Position posB second position
		//@treturn int the direct distance on the XZ plane from one position to the other
		table_misc.set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);


		///Translate a pair of percentages to screen-space pixel coordinates.
		//To be used with @{Strings.DisplayString:SetPosition} and @{Strings.DisplayString.new}.
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
		//@tparam Position pos1 first position
		//@tparam Position pos2 second position
		//@treturn bool is there a direct line of sight between the two positions?
		//@usage
		//local flamePlinthPos = flamePlinth:GetPosition() + Position(0, flamePlinthHeight, 0);
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
