#include "frameworkandsol.h"
#include "ScriptAssert.h"
#include "Moveable.h"
#include "ScriptUtil.h"
#include "Game/items.h"
#include "Objects/objectslist.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Position/Position.h"
#include "Rotation/Rotation.h"
#include "Specific/trmath.h"

/***
Represents any object inside the game world.
Examples include statics, enemies, doors,
pickups, and Lara herself.

@tenclass Objects.Moveable
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ "Moveable" };

static auto index_error = index_error_maker(Moveable, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(Moveable, LUA_CLASS_NAME);

Moveable::Moveable(short num, bool temp) : m_item{ &g_Level.Items[num] }, m_num{ num }, m_initialised{ false }, m_temporary{ temp }
{};

Moveable::Moveable(Moveable&& other) noexcept : 
	m_item { std::exchange(other.m_item, nullptr) },
	m_num{ std::exchange(other.m_num, NO_ITEM) },
	m_initialised{ std::exchange(other.m_initialised, false) },
	m_temporary{ std::exchange(other.m_temporary, false) }
{};

// todo.. how to check if item is killed outside of script?
Moveable::~Moveable() {
	// todo.. see if there's a better default state than -1
	if (m_temporary && (m_num > NO_ITEM))
	{
		s_callbackRemoveName(m_item->luaName);
		KillItem(m_num);
	}
}


/*** If you create items with this you NEED to give a position, room, 
and object number, and then call InitialiseItem before it will work.
	@function Moveable.new
*/

/*** Like above, but the returned variable controls the 
lifetime of the object (it will be destroyed when the variable goes
out of scope).
	@function Moveable.newTemporary
*/
template <bool temp> std::unique_ptr<Moveable> CreateEmpty()
{
	short num = CreateItem();	
	ITEM_INFO * item = &g_Level.Items[num];
	return std::make_unique<Moveable>(num, temp);
}

/*** For more information on each parameter, see the
associated getters and setters. If you do not know what to set for these,
most can just be set to zero (see usage). See also the overload which
takes no arguments.
	@function Moveable.new
	@tparam ObjID object ID
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
	@return reference to new Moveable object
	@usage 
	local item = Moveable.new(
		ObjID.PISTOLS_ITEM, -- object id
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

/*** Like the above, but the returned variable controls the 
lifetime of the object (it will be destroyed when the variable goes
out of scope).
	@function Moveable.newTemporary
	@param see_above same as above function
*/

template <bool temp> static std::unique_ptr<Moveable> Create(
	GAME_OBJECT_ID objID,
	std::string name,
	Position pos,
	Rotation rot,
	short room,
	short currentAnimState,
	short requiredAnimState,
	short goalAnimState,
	short animNumber,
	short frameNumber,
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
	auto ptr = std::make_unique<Moveable>(num, temp);

	ITEM_INFO* item = &g_Level.Items[num];
	ptr->SetPos(pos);
	ptr->SetRot(rot);
	ptr->SetRoom(room);
	ptr->SetObjectID(objID);
	InitialiseItem(num);

	ptr->SetName(name);
	ptr->SetCurrentAnimState(currentAnimState);
	ptr->SetRequiredAnimState(requiredAnimState);
	ptr->SetGoalAnimState(goalAnimState);
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

void Moveable::Register(sol::table & parent)
{
	parent.new_usertype<Moveable>(LUA_CLASS_NAME,
		"new", sol::overload(Create<false>, CreateEmpty<false>),
		"newTemporary", sol::overload(Create<true>, CreateEmpty<true>),
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

/// Initialise an item.
//Use this if you called new with no arguments
// @function Moveable.Init
		"Init", &Moveable::Init,

/// Enable the item
// @function Moveable:EnableItem
		"Enable", &Moveable::EnableItem,

/// Disable the item
// @function Moveable:DisableItem
		"Disable", &Moveable::DisableItem,

/// (@{ObjID}) object ID 
// @mem objectID
		"objectID", sol::property(&Moveable::GetObjectID, &Moveable::SetObjectID),

/*** (int) current animation state

The state number of the animation the object is currently doing.
This corresponds to "state" number shown in the animation editor of WadTool.
@mem currentAnimState
*/
		"currentAnimState", sol::property(&Moveable::GetCurrentAnimState, &Moveable::SetCurrentAnimState),

/// (int) State of required animation
// @mem requiredAnimState
		"requiredAnimState", sol::property(&Moveable::GetRequiredAnimState, &Moveable::SetRequiredAnimState),

/// (int) State of goal animation
// @mem goalAnimState
		"goalAnimState", sol::property(&Moveable::GetGoalAnimState, &Moveable::SetGoalAnimState),

/*** (int) animation number

The index of the animation the object is currently doing.
This corresponds to the number shown in the item's animation list in WadTool.
@mem animNumber
*/
		"animNumber", sol::property(&Moveable::GetAnimNumber, &Moveable::SetAnimNumber),

/*** (int) frame number

Current fame of the animation the object is currently doing.
The number of frames in an animation can be seen under the heading "End frame" in
the WadTool animation editor.
@mem frameNumber
*/
		"frameNumber", sol::property(&Moveable::GetFrameNumber, &Moveable::SetFrameNumber),

/// (int) HP (hit points/health points) of object 
//@raise an exception if the object is intelligent and an invalid
//hp value is given
// @mem HP
		"HP", sol::property(&Moveable::GetHP, &Moveable::SetHP),

/// (int) OCB (object code bit) of object
// @mem OCB
		"OCB", sol::property(&Moveable::GetOCB, &Moveable::SetOCB),

/// (table) item flags of object (table of 8 ints)
// @mem itemFlags 
		"itemFlags", sol::property(&Moveable::GetItemFlags, &Moveable::SetItemFlags),

/// (int) AIBits of object. Will be clamped to [0, 255]
// @mem AIBits
		"AIBits", sol::property(&Moveable::GetAIBits, &Moveable::SetAIBits),

/// (int) status of object.
// possible values:
// 0 - not active
// 1 - active
// 2 - deactivated
// 3 - invisible
// @mem status
		"status", sol::property(&Moveable::GetStatus, &Moveable::SetStatus),

/// (bool) hit status of object
// @mem hitStatus
		"hitStatus", sol::property(&Moveable::GetHitStatus, &Moveable::SetHitStatus),

/// (bool) whether or not the object is active 
// @mem active
		"active", sol::property(&Moveable::GetActive, &Moveable::SetActive),

/// (int) room the item is in 
// @mem room
		"room", sol::property(&Moveable::GetRoom, &Moveable::SetRoom),

/// (@{Position}) position in level
// @mem pos
		"pos", sol::property(&Moveable::GetPos, &Moveable::SetPos),

/// (@{Rotation}) rotation represented as degree angles about X, Y, and Z axes
// @mem rot
		"rot", sol::property(&Moveable::GetRot, &Moveable::SetRot),

/// (string) unique string identifier.
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// @mem name
		"name", sol::property(&Moveable::GetName, &Moveable::SetName)
		);
}


void Moveable::Init()
{
	bool cond = IsPointInRoom(m_item->pos, m_item->roomNumber);
	std::string err{ "Position of item \"{}\" does not match its room ID." };
	if (!ScriptAssertF(cond, err, m_item->luaName))
	{
		ScriptWarn("Resetting to the center of the room.");
		PHD_3DPOS center = GetRoomCenter(m_item->roomNumber);
		// reset position but not rotation
		m_item->pos.xPos = center.xPos;
		m_item->pos.yPos = center.yPos;
		m_item->pos.zPos = center.zPos;
	}
	InitialiseItem(m_num);
	m_initialised = true;
}

GAME_OBJECT_ID Moveable::GetObjectID() const
{
	return m_item->objectNumber;
}

void Moveable::SetObjectID(GAME_OBJECT_ID item) 
{
	m_item->objectNumber = item;
}


std::string Moveable::GetName() const
{
	return m_item->luaName;
}

void Moveable::SetName(std::string const & id) 
{
	ScriptAssert(!id.empty(), "Name cannot be blank", ERROR_MODE::TERMINATE);

	// remove the old name if we have one
	s_callbackRemoveName(m_item->luaName);

	// un-register any other objects using this name.
	// maybe we should throw an error if another object
	// already uses the name...
	s_callbackRemoveName(id);
	m_item->luaName = id;
	// todo add error checking
	s_callbackSetName(id, m_num);
}

Position Moveable::GetPos() const
{
	return Position(	m_item->pos	);
}

void Moveable::SetPos(Position const& pos)
{
	pos.StoreInPHDPos(m_item->pos);
}

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
Rotation Moveable::GetRot() const
{
	return Rotation(	int(TO_DEGREES(m_item->pos.xRot)) % 360,
								int(TO_DEGREES(m_item->pos.yRot)) % 360,
								int(TO_DEGREES(m_item->pos.zRot)) % 360);
}

void Moveable::SetRot(Rotation const& rot)
{
	m_item->pos.xRot = FROM_DEGREES(rot.x);
	m_item->pos.yRot = FROM_DEGREES(rot.y);
	m_item->pos.zRot = FROM_DEGREES(rot.z);
}

short Moveable::GetHP() const
{
	return(m_item->hitPoints);
}

void Moveable::SetHP(short hp)
{
	if(Objects[m_item->objectNumber].intelligent &&
		(hp < 0 || hp > Objects[m_item->objectNumber].hitPoints))
	{
		ScriptAssert(false, "Invalid HP value: " + std::to_string(hp));
		if (hp < 0)
		{
			hp = 0;
			ScriptWarn("Setting HP to 0.");
		}
		else if (hp > Objects[m_item->objectNumber].hitPoints)
		{
			hp = Objects[m_item->objectNumber].hitPoints;
			ScriptWarn("Setting HP to default value (" + std::to_string(hp) + ")");
		}
	}

	m_item->hitPoints = hp;
}

short Moveable::GetOCB() const
{
	return m_item->triggerFlags;
}

void Moveable::SetOCB(short ocb)
{
	m_item->triggerFlags = ocb;
}

byte Moveable::GetAIBits() const
{
	return m_item->aiBits;
}

void Moveable::SetAIBits(byte bits)
{
	m_item->aiBits = bits;
}

sol::as_table_t<std::array<short, 8>> Moveable::GetItemFlags() const
{	
	std::array<short, 8> ret{};
	memcpy(ret.data(), m_item->itemFlags, sizeof(m_item->itemFlags));
	return ret;
}

void Moveable::SetItemFlags(sol::as_table_t<std::array<short, 8>> const& arr)
{	
	memcpy(m_item->itemFlags, arr.value().data(), sizeof(m_item->itemFlags));
}

short Moveable::GetCurrentAnimState() const
{
	return m_item->currentAnimState;
}

void Moveable::SetCurrentAnimState(short animState)
{
	m_item->currentAnimState = animState;
}

short Moveable::GetRequiredAnimState() const
{
	return m_item->requiredAnimState;
}

void Moveable::SetRequiredAnimState(short animState)
{
	m_item->requiredAnimState = animState;
}

short Moveable::GetGoalAnimState() const
{
	return m_item->goalAnimState;
}

void Moveable::SetGoalAnimState(short state)
{
	m_item->goalAnimState = state;
}

short Moveable::GetAnimNumber() const
{
	return m_item->animNumber - Objects[m_item->objectNumber].animIndex;
}

void Moveable::SetAnimNumber(short animNumber)
{
	//TODO fixme: we need bounds checking with an error message once it's in the level file format
	m_item->animNumber = animNumber +  Objects[m_item->objectNumber].animIndex;
}

short Moveable::GetFrameNumber() const
{
	return m_item->frameNumber - g_Level.Anims[m_item->animNumber].frameBase;
}


void Moveable::SetFrameNumber(short frameNumber)
{
	auto const fBase = g_Level.Anims[m_item->animNumber].frameBase;
	auto const fEnd = g_Level.Anims[m_item->animNumber].frameEnd;
	auto frameCount = fEnd - fBase;
	bool cond = (frameNumber < frameCount);
	const char* err = "Invalid frame number {}; max frame count for anim {} is {}.";
	if (ScriptAssertF(cond, err, frameNumber, m_item->animNumber, frameCount))
	{
		m_item->frameNumber = frameNumber + fBase;
	}
	else
	{
		ScriptWarn("Not setting frame number.");
	}
}


short Moveable::GetStatus() const
{
	return m_item->status;
}

void Moveable::SetStatus(short status)
{
	m_item->status = status;
}

bool Moveable::GetActive() const
{
	return m_item->active;
}

void Moveable::SetActive(bool active)
{
	m_item->active = active;
}

bool Moveable::GetHitStatus() const
{
	return m_item->hitStatus;
}

void Moveable::SetHitStatus(bool hitStatus)
{
	m_item->hitStatus = hitStatus;
}

short Moveable::GetRoom() const
{
	return m_item->roomNumber;
}

void Moveable::SetRoom(short room)
{	
	const size_t nRooms = g_Level.Rooms.size();
	if (room < 0 || static_cast<size_t>(room) >= nRooms)
	{
		ScriptAssertF(false, "Invalid room number: {}. Value must be in range [0, {})", room, nRooms);
		TENLog("Room number will not be set", LogLevel::Warning, LogConfig::All);
		return;
	}

	if (!m_initialised)
		m_item->roomNumber = room;
	else
		ItemNewRoom(m_num, room);
}

void Moveable::EnableItem()
{
	if (!m_item->active)
	{
		if (Objects[m_item->objectNumber].intelligent)
		{
			if (m_item->status == ITEM_DEACTIVATED)
			{
				m_item->touchBits = 0;
				m_item->status = ITEM_ACTIVE;
				AddActiveItem(m_num);
				EnableBaddieAI(m_num, 1);
			}
			else if (m_item->status == ITEM_INVISIBLE)
			{
				m_item->touchBits = 0;
				if (EnableBaddieAI(m_num, 0))
					m_item->status = ITEM_ACTIVE;
				else
					m_item->status = ITEM_INVISIBLE;
				AddActiveItem(m_num);
			}
		}
		else
		{
			m_item->touchBits = 0;
			AddActiveItem(m_num);
			m_item->status = ITEM_ACTIVE;
		}
	}
}

void Moveable::DisableItem()
{
	if (m_item->active)
	{
		if (Objects[m_item->objectNumber].intelligent)
		{
			if (m_item->status == ITEM_ACTIVE)
			{
				m_item->touchBits = 0;
				m_item->status = ITEM_DEACTIVATED;
				RemoveActiveItem(m_num);
				DisableBaddieAI(m_num);
			}
		}
		else
		{
			m_item->touchBits = 0;
			RemoveActiveItem(m_num);
			m_item->status = ITEM_DEACTIVATED;
		}
	}
}