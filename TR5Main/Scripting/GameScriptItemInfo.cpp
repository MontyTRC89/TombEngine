#include "framework.h"
#include "ScriptAssert.h"
#include "GameScriptItemInfo.h"
#include "ScriptUtil.h"
#include "Game/items.h"
#include "Objects/objectslist.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "GameScriptPosition.h"
#include "GameScriptRotation.h"
#include "Specific/trmath.h"

/***
Represents any object inside the game world.
Examples include statics, enemies, doors,
pickups, and Lara herself.

@entityclass ItemInfo
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ "ItemInfo" };

static auto index_error = index_error_maker(GameScriptItemInfo, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(GameScriptItemInfo, LUA_CLASS_NAME);

GameScriptItemInfo::GameScriptItemInfo(short num, bool temp) : m_item{ &g_Level.Items[num] }, m_num{ num }, m_initialised{ false }, m_temporary{ temp }
{};

GameScriptItemInfo::GameScriptItemInfo(GameScriptItemInfo&& other) noexcept : 
	m_item { std::exchange(other.m_item, nullptr) },
	m_num{ std::exchange(other.m_num, NO_ITEM) },
	m_initialised{ std::exchange(other.m_initialised, false) },
	m_temporary{ std::exchange(other.m_temporary, false) }
{};

// todo.. how to check if item is killed outside of script?
GameScriptItemInfo::~GameScriptItemInfo() {
	// todo.. see if there's a better default state than -1
	if (m_temporary && (m_num > NO_ITEM))
	{
		s_callbackRemoveName(m_item->LuaName);
		KillItem(m_num);
	}
}


/*** If you create items with this you NEED to give a position, room, 
and object number, and then call InitialiseItem before it will work.
	@function ItemInfo.new
*/

/*** Like above, but the returned variable controls the 
lifetime of the object (it will be destroyed when the variable goes
out of scope).
	@function ItemInfo.newTemporary
*/
template <bool temp> std::unique_ptr<GameScriptItemInfo> CreateEmpty()
{
	short num = CreateItem();	
	ITEM_INFO * item = &g_Level.Items[num];
	return std::make_unique<GameScriptItemInfo>(num, temp);
}

/*** For more information on each parameter, see the
associated getters and setters. If you do not know what to set for these,
most can just be set to zero (see usage). See also the overload which
takes no arguments.
	@function ItemInfo.new
	@tparam ObjID object ID
	@tparam string name Lua name of the item
	@tparam Position position position in level
	@tparam Rotation rotation rotation about x, y, and z axes
	@tparam int room room ID item is in
	@tparam int ActiveState current animation state
	@tparam int RequiredState required animation state
	@tparam int TargetState goal animation state
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
		ObjID.PISTOLS_ITEM, -- object id
		"test", -- name
		Position.new(18907, 0, 21201),
		Rotation.new(0,0,0),
		0, -- room
		0, -- ActiveState
		0, -- RequiredState
		0, -- TargetState
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

/*** Like the above, but the returned variable controls the 
lifetime of the object (it will be destroyed when the variable goes
out of scope).
	@function ItemInfo.newTemporary
	@param see_above same as above function
*/

template <bool temp> static std::unique_ptr<GameScriptItemInfo> Create(
	GAME_OBJECT_ID objID,
	std::string name,
	GameScriptPosition pos,
	GameScriptRotation rot,
	short room,
	int ActiveState,
	int RequiredState,
	int TargetState,
	int animNumber,
	int frameNumber,
	short hp,
	short ocb,
	sol::as_table_t<std::array<short, 8>> flags,
	byte aiBits,
	short status,
	bool active,
	bool hitStatus
)
{
	short num = CreateItem();
	auto ptr = std::make_unique<GameScriptItemInfo>(num, temp);

	ITEM_INFO* item = &g_Level.Items[num];
	ptr->SetPos(pos);
	ptr->SetRot(rot);
	ptr->SetRoom(room);
	ptr->SetObjectID(objID);
	InitialiseItem(num);

	ptr->SetName(name);
	ptr->SetCurrentAnimState(ActiveState);
	ptr->SetRequiredAnimState(RequiredState);
	ptr->SetGoalAnimState(TargetState);
	ptr->SetAnimNumber(animNumber);
	ptr->SetFrameNumber(frameNumber);
	ptr->SetHP(hp);
	ptr->SetOCB(ocb);
	ptr->SetItemFlags(flags);
	ptr->SetAIBits(aiBits);
	ptr->SetStatus(status);
	ptr->SetActive(active);
	ptr->SetHitStatus(hitStatus);

	return ptr;
}

void GameScriptItemInfo::Register(sol::state* state)
{
	state->new_usertype<GameScriptItemInfo>(LUA_CLASS_NAME,
		"new", sol::overload(Create<false>, CreateEmpty<false>),
		"newTemporary", sol::overload(Create<true>, CreateEmpty<true>),
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

/// Initialise an item.
//Use this if you called new with no arguments
// @function ItemInfo.Init
		"Init", &GameScriptItemInfo::Init,

/// Enable the item
// @function ItemInfo:EnableItem
		"Enable", &GameScriptItemInfo::EnableItem,

/// Disable the item
// @function ItemInfo:DisableItem
		"Disable", &GameScriptItemInfo::DisableItem,

/// (@{ObjID}) object ID 
// @mem objectID
		"objectID", sol::property(&GameScriptItemInfo::GetObjectID, &GameScriptItemInfo::SetObjectID),

/*** (int) current animation state

The state number of the animation the object is currently doing.
This corresponds to "state" number shown in the animation editor of WadTool.
@mem ActiveState
*/
		"ActiveState", sol::property(&GameScriptItemInfo::GetCurrentAnimState, &GameScriptItemInfo::SetCurrentAnimState),

/// (int) State of required animation
// @mem RequiredState
		"RequiredState", sol::property(&GameScriptItemInfo::GetRequiredAnimState, &GameScriptItemInfo::SetRequiredAnimState),

/// (int) State of goal animation
// @mem TargetState
		"TargetState", sol::property(&GameScriptItemInfo::GetGoalAnimState, &GameScriptItemInfo::SetGoalAnimState),

/*** (int) animation number

The index of the animation the object is currently doing.
This corresponds to the number shown in the item's animation list in WadTool.
@mem animNumber
*/
		"animNumber", sol::property(&GameScriptItemInfo::GetAnimNumber, &GameScriptItemInfo::SetAnimNumber),

/*** (int) frame number

Current fame of the animation the object is currently doing.
The number of frames in an animation can be seen under the heading "End frame" in
the WadTool animation editor.
@mem frameNumber
*/
		"frameNumber", sol::property(&GameScriptItemInfo::GetFrameNumber, &GameScriptItemInfo::SetFrameNumber),

/// (int) HP (hit points/health points) of object 
//@raise an exception if the object is intelligent and an invalid
//hp value is given
// @mem HP
		"HP", sol::property(&GameScriptItemInfo::GetHP, &GameScriptItemInfo::SetHP),

/// (int) OCB (object code bit) of object
// @mem OCB
		"OCB", sol::property(&GameScriptItemInfo::GetOCB, &GameScriptItemInfo::SetOCB),

/// (table) item flags of object (table of 8 ints)
// @mem itemFlags 
		"itemFlags", sol::property(&GameScriptItemInfo::GetItemFlags, &GameScriptItemInfo::SetItemFlags),

/// (int) AIBits of object. Will be clamped to [0, 255]
// @mem AIBits
		"AIBits", sol::property(&GameScriptItemInfo::GetAIBits, &GameScriptItemInfo::SetAIBits),

/// (int) status of object.
// possible values:
// 0 - not active
// 1 - active
// 2 - deactivated
// 3 - invisible
// @mem status
		"status", sol::property(&GameScriptItemInfo::GetStatus, &GameScriptItemInfo::SetStatus),

/// (bool) hit status of object
// @mem hitStatus
		"hitStatus", sol::property(&GameScriptItemInfo::GetHitStatus, &GameScriptItemInfo::SetHitStatus),

/// (bool) whether or not the object is active 
// @mem active
		"active", sol::property(&GameScriptItemInfo::GetActive, &GameScriptItemInfo::SetActive),

/// (int) room the item is in 
// @mem room
		"room", sol::property(&GameScriptItemInfo::GetRoom, &GameScriptItemInfo::SetRoom),

/// (@{Position}) position in level
// @mem pos
		"pos", sol::property(&GameScriptItemInfo::GetPos, &GameScriptItemInfo::SetPos),

/// (@{Rotation}) rotation represented as degree angles about X, Y, and Z axes
// @mem rot
		"rot", sol::property(&GameScriptItemInfo::GetRot, &GameScriptItemInfo::SetRot),

/// (string) unique string identifier.
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// @mem name
		"name", sol::property(&GameScriptItemInfo::GetName, &GameScriptItemInfo::SetName)
		);
}


void GameScriptItemInfo::Init()
{
	bool cond = IsPointInRoom(m_item->Pose, m_item->RoomNumber);
	std::string err{ "Position of item \"{}\" does not match its room ID." };
	if (!ScriptAssertF(cond, err, m_item->LuaName))
	{
		ScriptWarn("Resetting to the center of the room.");
		PoseData center = GetRoomCenter(m_item->RoomNumber);
		// reset position but not rotation
		m_item->Pose.Position.x = center.Position.x;
		m_item->Pose.Position.y = center.Position.y;
		m_item->Pose.Position.z = center.Position.z;
	}
	InitialiseItem(m_num);
	m_initialised = true;
}

GAME_OBJECT_ID GameScriptItemInfo::GetObjectID() const
{
	return m_item->ObjectNumber;
}

void GameScriptItemInfo::SetObjectID(GAME_OBJECT_ID item) 
{
	m_item->ObjectNumber = item;
}


std::string GameScriptItemInfo::GetName() const
{
	return m_item->LuaName;
}

void GameScriptItemInfo::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_item->LuaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_item->LuaName = id;
	// todo add error checking
	s_callbackSetName(id, m_num);
}

GameScriptPosition GameScriptItemInfo::GetPos() const
{
	return GameScriptPosition(	m_item->Pose	);
}

void GameScriptItemInfo::SetPos(GameScriptPosition const& pos)
{
	pos.StoreInPHDPos(m_item->Pose);
}

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
GameScriptRotation GameScriptItemInfo::GetRot() const
{
	return GameScriptRotation(	int(Angle::RadToDeg(m_item->Pose.Orientation.GetX())) % 360,
								int(Angle::RadToDeg(m_item->Pose.Orientation.GetY())) % 360,
								int(Angle::RadToDeg(m_item->Pose.Orientation.GetZ())) % 360);
}

void GameScriptItemInfo::SetRot(GameScriptRotation const& rot)
{
	m_item->Pose.Orientation.SetX(Angle::DegToRad(rot.x));
	m_item->Pose.Orientation.SetY(Angle::DegToRad(rot.y));
	m_item->Pose.Orientation.SetZ(Angle::DegToRad(rot.z));
}

short GameScriptItemInfo::GetHP() const
{
	return(m_item->HitPoints);
}

void GameScriptItemInfo::SetHP(short hp)
{
	if(Objects[m_item->ObjectNumber].intelligent &&
		(hp < 0 || hp > Objects[m_item->ObjectNumber].HitPoints))
	{
		ScriptAssert(false, "Invalid HP value: " + std::to_string(hp));
		if (hp < 0)
		{
			hp = 0;
			ScriptWarn("Setting HP to 0.");
		}
		else if (hp > Objects[m_item->ObjectNumber].HitPoints)
		{
			hp = Objects[m_item->ObjectNumber].HitPoints;
			ScriptWarn("Setting HP to default value (" + std::to_string(hp) + ")");
		}
	}

	m_item->HitPoints = hp;
}

short GameScriptItemInfo::GetOCB() const
{
	return m_item->TriggerFlags;
}

void GameScriptItemInfo::SetOCB(short ocb)
{
	m_item->TriggerFlags = ocb;
}

byte GameScriptItemInfo::GetAIBits() const
{
	return m_item->AIBits;
}

void GameScriptItemInfo::SetAIBits(byte bits)
{
	m_item->AIBits = bits;
}

sol::as_table_t<std::array<short, 8>> GameScriptItemInfo::GetItemFlags() const
{	
	std::array<short, 8> ret{};
	memcpy(ret.data(), m_item->ItemFlags, sizeof(m_item->ItemFlags));
	return ret;
}

void GameScriptItemInfo::SetItemFlags(sol::as_table_t<std::array<short, 8>> const& arr)
{	
	memcpy(m_item->ItemFlags, arr.value().data(), sizeof(m_item->ItemFlags));
}

int GameScriptItemInfo::GetCurrentAnimState() const
{
	return m_item->Animation.ActiveState;
}

void GameScriptItemInfo::SetCurrentAnimState(int animState)
{
	m_item->Animation.ActiveState = animState;
}

int GameScriptItemInfo::GetRequiredAnimState() const
{
	return m_item->Animation.RequiredState;
}

void GameScriptItemInfo::SetRequiredAnimState(short animState)
{
	m_item->Animation.RequiredState = animState;
}

int GameScriptItemInfo::GetGoalAnimState() const
{
	return m_item->Animation.TargetState;
}

void GameScriptItemInfo::SetGoalAnimState(int state)
{
	m_item->Animation.TargetState = state;
}

int GameScriptItemInfo::GetAnimNumber() const
{
	return m_item->Animation.AnimNumber - Objects[m_item->ObjectNumber].animIndex;
}

void GameScriptItemInfo::SetAnimNumber(int animNumber)
{
	//TODO fixme: we need bounds checking with an error message once it's in the level file format
	m_item->Animation.AnimNumber = animNumber +  Objects[m_item->ObjectNumber].animIndex;
}

int GameScriptItemInfo::GetFrameNumber() const
{
	return m_item->Animation.FrameNumber - g_Level.Anims[m_item->Animation.AnimNumber].frameBase;
}

void GameScriptItemInfo::SetFrameNumber(int frameNumber)
{
	auto const fBase = g_Level.Anims[m_item->Animation.AnimNumber].frameBase;
	auto const fEnd = g_Level.Anims[m_item->Animation.AnimNumber].frameEnd;
	auto frameCount = fEnd - fBase;
	bool cond = (frameNumber < frameCount);
	const char* err = "Invalid frame number {}; max frame count for anim {} is {}.";

	if (ScriptAssertF(cond, err, frameNumber, m_item->Animation.AnimNumber, frameCount))
		m_item->Animation.FrameNumber = frameNumber + fBase;
	else
		ScriptWarn("Not setting frame number.");
}

short GameScriptItemInfo::GetStatus() const
{
	return m_item->Status;
}

void GameScriptItemInfo::SetStatus(short status)
{
	m_item->Status = status;
}

bool GameScriptItemInfo::GetActive() const
{
	return m_item->Active;
}

void GameScriptItemInfo::SetActive(bool active)
{
	m_item->Active = active;
}

bool GameScriptItemInfo::GetHitStatus() const
{
	return m_item->HitStatus;
}

void GameScriptItemInfo::SetHitStatus(bool hitStatus)
{
	m_item->HitStatus = hitStatus;
}

short GameScriptItemInfo::GetRoom() const
{
	return m_item->RoomNumber;
}

void GameScriptItemInfo::SetRoom(short room)
{	
	const size_t nRooms = g_Level.Rooms.size();
	if (room < 0 || static_cast<size_t>(room) >= nRooms)
	{
		ScriptAssertF(false, "Invalid room number: {}. Value must be in range [0, {})", room, nRooms);
		TENLog("Room number will not be set", LogLevel::Warning, LogConfig::All);
		return;
	}

	if (!m_initialised)
		m_item->RoomNumber = room;
	else
		ItemNewRoom(m_num, room);
}

void GameScriptItemInfo::EnableItem()
{
	if (!m_item->Active)
	{
		if (Objects[m_item->ObjectNumber].intelligent)
		{
			if (m_item->Status == ITEM_DEACTIVATED)
			{
				m_item->TouchBits = 0;
				m_item->Status = ITEM_ACTIVE;
				AddActiveItem(m_num);
				EnableBaddieAI(m_num, 1);
			}
			else if (m_item->Status == ITEM_INVISIBLE)
			{
				m_item->TouchBits = 0;
				if (EnableBaddieAI(m_num, 0))
					m_item->Status = ITEM_ACTIVE;
				else
					m_item->Status = ITEM_INVISIBLE;
				AddActiveItem(m_num);
			}
		}
		else
		{
			m_item->TouchBits = 0;
			AddActiveItem(m_num);
			m_item->Status = ITEM_ACTIVE;
		}
	}
}

void GameScriptItemInfo::DisableItem()
{
	if (m_item->Active)
	{
		if (Objects[m_item->ObjectNumber].intelligent)
		{
			if (m_item->Status == ITEM_ACTIVE)
			{
				m_item->TouchBits = 0;
				m_item->Status = ITEM_DEACTIVATED;
				RemoveActiveItem(m_num);
				DisableEntityAI(m_num);
			}
		}
		else
		{
			m_item->TouchBits = 0;
			RemoveActiveItem(m_num);
			m_item->Status = ITEM_DEACTIVATED;
		}
	}
}