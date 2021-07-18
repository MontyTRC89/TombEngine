#pragma once
#include <functional>

/***
Represents any object inside the game world.
Examples include statics, enemies, doors,
pickups, and Lara herself.

@classmod ItemInfo
@pragma nostrip
*/
namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class GameScriptPosition;
class GameScriptRotation;
struct ITEM_INFO;
enum GAME_OBJECT_ID : short;

using callbackSetName = std::function<bool(std::string const&, short itemID)>;
using callbackRemoveName = std::function<bool(std::string const&)>;

class GameScriptItemInfo
{
private:
	ITEM_INFO* m_item;
	short m_num;
	static callbackSetName s_callbackSetName;
	static callbackRemoveName s_callbackRemoveName;
public:
	GameScriptItemInfo(short num);
	~GameScriptItemInfo();
	GameScriptItemInfo& operator=(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo const& other) = delete;
	GameScriptItemInfo(GameScriptItemInfo && other) noexcept;

	static void Register(sol::state *);
	/*** If you create items with this you NEED to give a position, rotation,
	room, and object number, and then call InitialiseItem before it will work.
	@function ItemInfo.new
	*/
	static std::unique_ptr<GameScriptItemInfo> CreateEmpty();

	/*** For more information on each parameter, see the
	associated getters and setters. If you do not know what to set for these,
	most can just be set them to zero (see usage) or use the overload which
	takes no arguments.
	@function ItemInfo.new
	@tparam int object ID
	@tparam string name Lua name of the item
	@tparam Position position position in level
	@tparam Rotation rotation rotation about x, y, and z axes
	@tparam int room room ID item is in
	@tparam int currentAnimState current animation state
	@tparam int requiredAnimState required animation state
	@tparam int goalAnimState goal animation state
	@tparam int animNumber anim number
	@tparam int frameNumber frame number
	@tparam int hp HP of item
	@tparam int OCB ocb of item
	@tparam int itemFlags item flags 
	@tparam int AIBits byte with AI bits
	@tparam int status status of object
	@tparam bool active is item active or not?
	@tparam bool hitStatus hit status of object
	@return reference to new ItemInfo object
	@usage 
	local item = ItemInfo.new(
		950, -- object id. 950 is pistols
		"test", -- name
		Position.new(18907, 0, 21201),
		Rotation.new(0,0,0),
		0, -- room
		0, -- currentAnimState
		0, -- requiredAnimState
		0, -- goalAnimState
		0, -- animNumber
		0, -- frameNumber
		0, -- HP
		0, -- OCB
		{0,0,0,0,0,0,0,0}, -- itemFlags
		0, -- AIBits
		0, -- status
		false, -- active
		false, -- hitStatus
		)
	*/
	static std::unique_ptr<GameScriptItemInfo> Create(
		GAME_OBJECT_ID objID,
		std::string Name,
		GameScriptPosition pos,
		GameScriptRotation aRot,
		short room,
		short aCurrentAnimState,
		short aRequiredAnimState,
		short goalAnimState,
		short animNumber,
		short frameNumber,
		short aHp,
		short aOcb,
		sol::as_table_t<std::array<short, 8>> aFlags,
		byte aAiBits,
		short aStatus,
		bool aActive,
		bool aHitStatus
		);
	static void SetNameCallbacks(callbackSetName, callbackRemoveName);

/// (int) object ID 
//@mem objectID
	GAME_OBJECT_ID GetObjectID() const;
	void SetObjectID(GAME_OBJECT_ID id);

/// (string) unique string identifier.
// e.g. "door_back_room" or "cracked_greek_statue"
//@mem name
	std::string GetName() const;
	void SetName(std::string const &);

/// (@{Position}) position in level
//@mem pos
	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

/// (@{Rotation}) rotation represented as degree angles about X, Y, and Z axes
//@mem rot
	GameScriptRotation GetRot() const;
	void SetRot(GameScriptRotation const& rot);

/// (int) State of current animation
//@mem currentAnimState
//@todo what does this actually mean/should we expose it to the user?
	short GetCurrentAnimState() const;
	void SetCurrentAnimState(short animState);

/// (int) animation number
//@mem animNumber
	short GetAnimNumber() const;
	void SetAnimNumber(short animNumber);

/// (int) frame number
//@mem frameNumber
	short GetFrameNumber() const;
	void SetFrameNumber(short frameNumber);

/// (int) State of required animation
//@mem requiredAnimState
//@todo what does this actually mean/should we expose it to the user?
	short GetRequiredAnimState() const;
	void SetRequiredAnimState(short animState);

/// (int) State of goal animation
//@mem goalAnimState
//@todo what does this actually mean/should we expose it to the user?
	short GetGoalAnimState() const;
	void SetGoalAnimState(short animState);

/// (int) HP (hit points/health points) of object 
//@raise an exception if the object is intelligent and an invalid
//hp value is given
//@mem HP
	short GetHP() const;
	void SetHP(short hp);

/// (int) OCB (object code bit) of object
//@mem OCB
	short GetOCB() const;
	void SetOCB(short ocb);

/// (table) item flags of object (table of 8 ints)
//@mem itemFlags 
	sol::as_table_t<std::array<short, 8>>  GetItemFlags() const;
	void SetItemFlags(sol::as_table_t<std::array<short, 8>>  const & arr);

/// (int) AIBits of object. Will be clamped to [0, 255]
// @mem AIBits
	byte GetAIBits() const;
	void SetAIBits(byte bits);

/// (int) status of object.
// possible values:
// 0 - not active
// 1 - active
// 2 - deactivated
// 3 - invisible
// @mem status
	short GetStatus() const;
	void SetStatus(short status);

/// (bool) hit status of object
// @mem hitStatus
	bool GetHitStatus() const;
	void SetHitStatus(bool status);

/// (bool) whether or not the object is active 
// @mem hitStatus
	bool GetActive() const;
	void SetActive(bool active);

/// (short) room the item is in 
// @mem room
	short GetRoom() const;
	void SetRoom(short Room);

/// Enable the item
// @function ItemInfo:EnableItem
	void EnableItem();

/// Disable the item
// @function ItemInfo:DisableItem
	void DisableItem();

/// Initialise an item. Use this if you called new with no arguments
	void Init();
};
