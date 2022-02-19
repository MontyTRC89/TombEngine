#include "frameworkandsol.h"

#ifdef TEN_OPTIONAL_LUA
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
#include "ReservedScriptNames.h"

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
Moveable::~Moveable()
{
	// todo.. see if there's a better default state than -1
	if (m_temporary && (m_num > NO_ITEM))
	{
		s_callbackRemoveName(m_item->luaName);
		KillItem(m_num);
	}
}


/*** If you create items with this you NEED to give a position, room, 
and object number, and then call InitialiseItem before it will work.
	@function Moveable.New
*/

/*** Like above, but the returned variable controls the 
lifetime of the object (it will be destroyed when the variable goes
out of scope).
	@function Moveable.NewTemporary
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
	std::string const & name,
	Position const & pos,
	Rotation const & rot,
	short room,
	int animNumber,
	int frameNumber,
	short hp,
	short ocb,
	aiBitsType const & aiBits,
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
	ptr->SetAnimNumber(animNumber);
	ptr->SetFrameNumber(frameNumber);
	ptr->SetHP(hp);
	ptr->SetOCB(ocb);
	ptr->SetAIBits(aiBits);
	ptr->SetActive(active);
	ptr->SetHitStatus(hitStatus);

	return ptr;
}

void Moveable::Register(sol::table & parent)
{
	parent.new_usertype<Moveable>(LUA_CLASS_NAME,
		ScriptReserved_new, sol::overload(Create<false>, CreateEmpty<false>),
		ScriptReserved_newTemporary, sol::overload(Create<true>, CreateEmpty<true>),
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,

/// Initialise an item.
//Use this if you called new with no arguments
// @function Moveable.Init
		ScriptReserved_Init, &Moveable::Init,

/// Enable the item
// @function Moveable:EnableItem
		ScriptReserved_Enable, &Moveable::EnableItem,

/// Disable the item
// @function Moveable:DisableItem
		ScriptReserved_Disable, &Moveable::DisableItem,

/// Make the item invisible. Use EnableItem to make it visible again.
// @function Moveable:MakeInvisible
		ScriptReserved_MakeInvisible, &Moveable::MakeInvisible,

/// (int) status of object.
// possible values:
// 0 - not active
// 1 - active
// 2 - deactivated
// 3 - invisible
// @function Moveable:GetStatus
// @treturn int a number representing the status of the object
		ScriptReserved_GetStatus, &Moveable::GetStatus,

/// Retrieve the object ID
// @function Moveable:GetObjectID
// @treturn int a number representing the ID of the object
		ScriptReserved_GetObjectID, &Moveable::GetObjectID,

/// Change the object's ID. This will literally change the object.
// @function Moveable:SetObjectID
// @tparam ObjectID ID the new ID 
// @usage
// shiva = TEN.Objects.GetMoveableByName("shiva_60")
// shiva:SetObjectID(TEN.Objects.ObjID.BIGMEDI_ITEM)
		ScriptReserved_SetObjectID, &Moveable::SetObjectID,

/// Retrieve the index of the current animation.
// This corresponds to the number shown in the item's animation list in WadTool.
// @function Moveable:GetAnim
// @treturn int the index of the active animation
		ScriptReserved_GetAnimNumber, &Moveable::GetAnimNumber,

/// Set the opject's animation to the one specified by the given index.
// Performs no bounds checking. *Ensure the number given is correct, else
// the program is likely to crash with an unhelpful error message.*
// @function Moveable:SetAnim
// @tparam int index the index of the desired anim 
		ScriptReserved_SetAnimNumber, &Moveable::SetAnimNumber,

/// Retrieve frame number.
// This is the current frame of the object's active animation.
// @function Moveable:GetFrame
// @treturn int the current frame of the active animation
		ScriptReserved_GetFrameNumber, &Moveable::GetFrameNumber,

/// Set frame number.
// This will move the animation to the given frame.
// The number of frames in an animation can be seen under the heading "End frame" in
// the WadTool animation editor.
// @function Moveable:SetFrame
// @tparam int frame the new frame number
		ScriptReserved_SetFrameNumber, &Moveable::SetFrameNumber,
		
/// Get current HP (hit points/health points)
// @function Moveable:GetHP
// @treturn int the amount of HP the moveable currently has
		ScriptReserved_GetHP, &Moveable::GetHP,

/// Set current HP (hit points/health points)
// @function Moveable:SetHP
// @tparam int the amount of HP to give the moveable
		ScriptReserved_SetHP, &Moveable::SetHP,

/// Get OCB (object code bit) of the moveable
// @function Moveable:GetOCB
// @treturn int the moveable's current OCB value
		ScriptReserved_GetOCB, &Moveable::GetOCB,

/// Set OCB (object code bit) of the moveable
// @function Moveable:SetOCB
// @tparam int the new value for the moveable's OCB
		ScriptReserved_SetOCB, &Moveable::SetOCB,

/// (int) AIBits of object. Will be clamped to [0, 255]
// @mem AIBits
// 1 - none
// 2 - guard
// 3 - ambush
// 4 - patrol 1
// 5 - modify
// 6 - follow
// 7 - patrol 2
		"AIBits", sol::property(&Moveable::GetAIBits, &Moveable::SetAIBits),

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

aiBitsType Moveable::GetAIBits() const
{
	static_assert(63 == ALL_AIOBJ);

	aiBitsArray ret{};
	for (int i = 0; i < ret.size(); ++i)
	{
		uint8_t isSet = m_item->aiBits & (1 << i);
		ret[i] = static_cast<int>( isSet > 0);
	}
	return ret;
}

void Moveable::SetAIBits(aiBitsType const & bits)
{
	for (int i = 0; i < bits.value().size(); ++i)
	{
		uint8_t isSet = bits.value()[i] > 0;
		m_item->aiBits |= isSet << i;
	}
}

int Moveable::GetAnimNumber() const
{
	return m_item->animNumber - Objects[m_item->objectNumber].animIndex;
}

void Moveable::SetAnimNumber(int animNumber)
{
	//TODO fixme: we need bounds checking with an error message once it's in the level file format
	m_item->animNumber = animNumber +  Objects[m_item->objectNumber].animIndex;
}

int Moveable::GetFrameNumber() const
{
	return m_item->frameNumber - g_Level.Anims[m_item->animNumber].frameBase;
}


void Moveable::SetFrameNumber(int frameNumber)
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

short Moveable::GetStatus() const
{
	return m_item->status;
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

void Moveable::MakeInvisible()
{
	m_item->status = ITEM_INVISIBLE;
	if (m_item->active)
	{
		m_item->touchBits = 0;
		RemoveActiveItem(m_num);
		if (Objects[m_item->objectNumber].intelligent)
		{
			DisableBaddieAI(m_num);
		}
	}
}
#endif
