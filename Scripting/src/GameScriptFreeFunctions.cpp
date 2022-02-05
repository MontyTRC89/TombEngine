#include "frameworkandsol.h"
#include "ReservedScriptNames.h"
#include "Sound\sound.h"
#include "GameScriptPosition.h"
#include "GameScriptColor.h"
#include "Game\effects\lightning.h"
#include "effects\tomb4fx.h"
#include "effects\effects.h"
#include "camera.h"
#include "pickup.h"
#include "ItemEnumPair.h"

/***
Scripts that will be run on game startup.
@tentable Misc 
@pragma nostrip
*/

using namespace TEN::Effects::Lightning;

namespace GameScriptFreeFunctions {
	static int FindRoomNumber(GameScriptPosition pos)
	{
		return 0;
	}

	static void AddLightningArc(GameScriptPosition src, GameScriptPosition dest, GameScriptColor color, int lifetime, int amplitude, int beamWidth, int segments, int flags)
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

	static void AddShockwave(GameScriptPosition pos, int innerRadius, int outerRadius, GameScriptColor color, int lifetime, int speed, int angle, int flags)
	{
		PHD_3DPOS p;
		p.xPos = pos.x;
		p.yPos = pos.y;
		p.zPos = pos.z;

		TriggerShockwave(&p, innerRadius, outerRadius, speed, color.GetR(), color.GetG(), color.GetB(), lifetime, FROM_DEGREES(angle), flags);
	}

	static void AddDynamicLight(GameScriptPosition pos, GameScriptColor color, int radius, int lifetime)
	{
		TriggerDynamicLight(pos.x, pos.y, pos.z, radius, color.GetR(), color.GetG(), color.GetB());
	}

	static void AddBlood(GameScriptPosition pos, int num)
	{
		TriggerBlood(pos.x, pos.y, pos.z, -1, num);
	}

	static void AddFireFlame(GameScriptPosition pos, int size)
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

	static void PlaySoundEffect(int id, GameScriptPosition p, int flags)
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
	static void InventoryAdd(ItemEnumPair slot, sol::optional<int> count)
	{
		// If 0 is passed in, then the amount added will be the default amount
		// for that pickup - i.e. the amount you would get from picking up the
		// item in-game (e.g. 1 for medipacks, 12 for flares).
		PickedUpObject(slot.m_pair.first, count.value_or(0));
	}

	static void InventoryRemove(ItemEnumPair slot, sol::optional<int> count)
	{
		// 0 is default for the same reason as in InventoryAdd.
		RemoveObjectFromInventory(slot.m_pair.first, count.value_or(0));
	}

	static int InventoryGetCount(ItemEnumPair slot)
	{
		return GetInventoryCount(slot.m_pair.first);
	}

	static void InventorySetCount(ItemEnumPair slot, int count)
	{
		// add the amount we'd need to add to get to count
		int currAmt = GetInventoryCount(slot.m_pair.first);
		InventoryAdd(slot, count - currAmt);
	}

	static void InventoryCombine(int slot1, int slot2)
	{

	}

	static void InventorySeparate(int slot)
	{

	}

	static int CalculateDistance(GameScriptPosition const& pos1, GameScriptPosition const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	static int CalculateHorizontalDistance(GameScriptPosition const& pos1, GameScriptPosition const& pos2)
	{
		auto result = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.z - pos2.z));
		return static_cast<int>(round(result));
	}

	static std::tuple<int, int> PercentToScreen(double x, double y)
	{
		auto fWidth = static_cast<double>(g_Configuration.Width);
		auto fHeight = static_cast<double>(g_Configuration.Height);
		int resX = std::round(fWidth / 100.0 * x);
		int resY = std::round(fHeight / 100.0 * y);
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


	void Register(sol::state* lua) {
		///Set and play an ambient track
		//@function SetAmbientTrack
		//@tparam string name of track (without file extension) to play
		lua->set_function(ScriptReserved_SetAmbientTrack, &SetAmbientTrack);

		/// Play an audio track
		//@function PlayAudioTrack
		//@tparam string name of track (without file extension) to play
		//@tparam bool loop if true, the track will loop; if false, it won't (default: false)
		lua->set_function(ScriptReserved_PlayAudioTrack, &PlayAudioTrack);

		///Add x of an item to the inventory.
		//A count of 0 will add the "default" amount of that item
		//(i.e. the amount the player would get from a pickup of that type).
		//For example, giving "zero" crossbow ammo would give the player
		//10 instead, whereas giving "zero" medkits would give the player 1 medkit.
		//@function GiveInvItem
		//@tparam InvItem item the item to be added
		//@tparam int count the number of items to add (default: 0)
		lua->set_function(ScriptReserved_GiveInvItem, &InventoryAdd);

		
		//Remove x of a certain item from the inventory.
		//As in @{GiveInvItem}, a count of 0 will remove the "default" amount of that item.
		//@function TakeInvItem
		//@tparam InvItem item the item to be removed
		//@tparam int count the number of items to remove (default: 0)
		
		lua->set_function(ScriptReserved_TakeInvItem, &InventoryRemove);

		
		///Get the amount the player holds of an item.
		//@function GetInvItemCount
		//@tparam InvItem item the item to check
		//@treturn int the amount of the item the player has in the inventory
		
		lua->set_function(ScriptReserved_GetInvItemCount, &InventoryGetCount);


		///Set the amount of a certain item the player has in the inventory.
		//Similar to @{GiveInvItem} but replaces with the new amount instead of adding it.
		//@function SetInvItemCount
		//@tparam @{InvItem} item the item to be set
		//@tparam int count the number of items the player will have
		lua->set_function(ScriptReserved_SetInvItemCount, &InventorySetCount);

	
		///Calculate the distance between two positions.
		//@function CalculateDistance
		//@tparam Position posA first position
		//@tparam Position posB second position
		//@treturn int the direct distance from one position to the other
		lua->set_function(ScriptReserved_CalculateDistance, &CalculateDistance);

		
		///Calculate the horizontal distance between two positions.
		//@function CalculateHorizontalDistance
		//@tparam Position posA first position
		//@tparam Position posB second position
		//@treturn int the direct distance on the XZ plane from one position to the other
		lua->set_function(ScriptReserved_CalculateHorizontalDistance, &CalculateHorizontalDistance);


		///Translate a pair of percentages to screen-space pixel coordinates.
		//To be used with @{DisplayString:SetPos} and @{DisplayString.new}.
		//@function PercentToScreen
		//@tparam float x percent value to translate to x-coordinate
		//@tparam float y percent value to translate to y-coordinate
		//@treturn int x x coordinate in pixels
		//@treturn int y y coordinate in pixels
		lua->set_function(ScriptReserved_PercentToScreen, &PercentToScreen);


		///Translate a pair of coordinates to percentages of window dimensions.
		//To be used with @{DisplayString:GetPos}.
		//@function ScreenToPercent
		//@tparam int x pixel value to translate to a percentage of the window width
		//@tparam int y pixel value to translate to a percentage of the window height
		//@treturn float x coordinate as percentage
		//@treturn float y coordinate as percentage
		lua->set_function(ScriptReserved_ScreenToPercent, &ScreenToPercent);
	}
}
