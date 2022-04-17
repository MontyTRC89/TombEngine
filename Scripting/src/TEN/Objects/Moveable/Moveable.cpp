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
#include "Objects/ObjectsHandler.h"
#include "ReservedScriptNames.h"

/***
Represents any object inside the game world.
Examples include traps, enemies, doors,
pickups, and Lara herself.

@tenclass Objects.Moveable
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ ScriptReserved_Moveable };

static auto index_error = index_error_maker(Moveable, LUA_CLASS_NAME);
static auto newindex_error = newindex_error_maker(Moveable, LUA_CLASS_NAME);


Moveable::Moveable(short num, bool alreadyInitialised) : m_item{ &g_Level.Items[num] }, m_num{ num }, m_initialised{ alreadyInitialised }
{
	if (alreadyInitialised)
	{
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->AddMoveableToMap(m_item, this);
	}
};

Moveable::Moveable(Moveable&& other) noexcept : 
	m_item{ std::exchange(other.m_item, nullptr) },
	m_num{ std::exchange(other.m_num, NO_ITEM) },
	m_initialised{ std::exchange(other.m_initialised, false) }
{
	if (GetValid())
	{
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->RemoveMoveableFromMap(m_item, &other);
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->AddMoveableToMap(m_item, this);
	}
};


Moveable::~Moveable()
{
	if (m_num > NO_ITEM && g_GameScriptEntities) {
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->RemoveMoveableFromMap(m_item, this);
	}
}

bool operator==(Moveable const& first, Moveable const& second)
{
	return first.m_item == second.m_item;
}

/*** For more information on each parameter, see the
associated getters and setters. If you do not know what to set for these,
most can just be set to zero (see usage).
	@function Moveable.New
	@tparam ObjID object ID
	@tparam string name Lua name of the item
	@tparam Position position position in level
	@tparam Rotation rotation rotation about x, y, and z axes (default Rotation.new(0, 0, 0))
	@tparam int room room ID item is in
	@tparam int animNumber anim number (default 0)
	@tparam int frameNumber frame number (default 0)
	@tparam int hp HP of item (default 10)
	@tparam int OCB ocb of item (default 0)
	@tparam table AIBits table with AI bits (default {0,0,0,0,0,0})
	@return reference to new Moveable object
	@usage 
	local item = Moveable.New(
		TEN.ObjID.PISTOLS_ITEM, -- object id
		"test", -- name
		Position.new(18907, 0, 21201),
		Rotation.new(0,0,0),
		0, -- room
		0, -- animNumber
		0, -- frameNumber
		10, -- HP
		0, -- OCB
		{0,0,0,0,0,0} -- aiBits
		)
	*/

#define USE_IF_HAVE(Type, ifThere, ifNotThere) \
std::holds_alternative<Type>(ifThere) ? std::get<Type>(ifThere) : ifNotThere

static std::unique_ptr<Moveable> Create(
	GAME_OBJECT_ID objID,
	std::string const & name,
	Position const & pos,
	TypeOrNil<Rotation> const & rot,
	short room,
	TypeOrNil<int> animNumber,
	TypeOrNil<int> frameNumber,
	TypeOrNil<short> hp,
	TypeOrNil<short> ocb,
	TypeOrNil<aiBitsType> const & aiBits
)
{
	short num = CreateItem();
	auto ptr = std::make_unique<Moveable>(num, false);

	if (ScriptAssert(ptr->SetName(name), "Could not set name for Moveable; returning an invalid object."))
	{

		ITEM_INFO* item = &g_Level.Items[num];
		ptr->SetPos(pos);
		ptr->SetRot(USE_IF_HAVE(Rotation, rot, Rotation{}));
		ptr->SetRoom(room);
		ptr->SetObjectID(objID);
		ptr->Init();

		ptr->SetAnimNumber(USE_IF_HAVE(int, animNumber, 0));
		ptr->SetFrameNumber(USE_IF_HAVE(int, frameNumber, 0));
		ptr->SetHP(USE_IF_HAVE(short, hp, 10));
		ptr->SetOCB(USE_IF_HAVE(short, ocb, 0));
		ptr->SetAIBits(USE_IF_HAVE(aiBitsType, aiBits, aiBitsType{}));
		item->carriedItem = NO_ITEM;

		// call this when resetting name too?
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->AddMoveableToMap(item, ptr.get());
		// add to name map too?
	}
	return ptr;
}

void Moveable::Register(sol::table & parent)
{
	parent.new_usertype<Moveable>(LUA_CLASS_NAME,
		ScriptReserved_new, Create,
		ScriptReserved_newTemporary, Create,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,
		sol::meta_function::equal_to, std::equal_to<Moveable const>(),

/// Enable the item
// @function Moveable:EnableItem
		ScriptReserved_Enable, &Moveable::EnableItem,

/// Disable the item
// @function Moveable:DisableItem
		ScriptReserved_Disable, &Moveable::DisableItem,

/// Make the item invisible. Use EnableItem to make it visible again.
// @function Moveable:MakeInvisible
		ScriptReserved_MakeInvisible, &Moveable::MakeInvisible,

/// Get the status of object.
// possible values:
// 0 - not active
// 1 - active
// 2 - deactivated
// 3 - invisible
// @function Moveable:GetStatus
// @treturn int a number representing the status of the object
		ScriptReserved_GetStatus, &Moveable::GetStatus,
		
/// Set the name of the function to be called when the moveable is shot by Lara
// Note that this will be triggered twice when shot with both pistols at once. 
// @function Moveable:SetOnHit
// @tparam string name of callback function to be called
		ScriptReserved_SetOnHit, &Moveable::SetOnHit,

/// Get the name of the function called when this moveable is shot
// @function Moveable:GetOnHit
// @treturn string name of the function
		ScriptReserved_GetOnHit, &Moveable::GetOnHit,

/// Set the name of the function called when this moveable collides with another moveable
// @function Moveable:SetOnCollidedWithObject
// @tparam string name of callback function to be called
		ScriptReserved_SetOnCollidedWithObject, &Moveable::SetOnCollidedWithObject,

/// Get the name of the function called when this moveable collides with another moveable
// @function Moveable:GetOnCollidedWithObject
// @treturn string name of the function
		ScriptReserved_GetOnCollidedWithObject, &Moveable::GetOnCollidedWithObject,

/// Set the name of the function to be called when the moveable is destroyed/killed
// @function Moveable:SetOnKilled
// @tparam string callback name of function to be called
// @usage
// LevelFuncs.baddyKilled = function(theBaddy) print("You killed a baddy!") end
// baddy:SetOnKilled("baddyKilled")
		ScriptReserved_SetOnKilled, &Moveable::SetOnKilled,

/// Get the name of the function called when this moveable is killed
// @function Moveable:GetOnKilled
// @treturn string name of the function
		ScriptReserved_GetOnKilled, &Moveable::GetOnKilled,

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
// the WadTool animation editor. If the animation has no frames, the only valid argument
// is -1.
// @function Moveable:SetFrame
// @tparam int frame the new frame number
		ScriptReserved_SetFrameNumber, &Moveable::SetFrameNumber,
		
/// Get current HP (hit points/health points)
// @function Moveable:GetHP
// @treturn int the amount of HP the moveable currently has
		ScriptReserved_GetHP, &Moveable::GetHP,

/// Set current HP (hit points/health points)
// @function Moveable:SetHP
// @tparam int HP the amount of HP to give the moveable
		ScriptReserved_SetHP, &Moveable::SetHP,

/// Get OCB (object code bit) of the moveable
// @function Moveable:GetOCB
// @treturn int the moveable's current OCB value
		ScriptReserved_GetOCB, &Moveable::GetOCB,

/// Set OCB (object code bit) of the moveable
// @function Moveable:SetOCB
// @tparam int OCB the new value for the moveable's OCB
		ScriptReserved_SetOCB, &Moveable::SetOCB,

/// Get AIBits of object
// This will return a table with six values, each corresponding to
// an active behaviour. If the object is in a certain AI mode, the table will
// have a *1* in the corresponding cell. Otherwise, the cell will hold
// a *0*.
// 1 - guard
// 2 - ambush
// 3 - patrol 1
// 4 - modify
// 5 - follow
// 6 - patrol 2
// @function Moveable:GetAIBits
// @treturn table a table of AI bits
		ScriptReserved_GetAIBits, &Moveable::GetAIBits, 
			
/// Set AIBits of object
// Use this to force a moveable into a certain AI mode or modes, as if a certain nullmesh
// (or more than one) had suddenly spawned beneath their feet.
// @function Moveable:SetAIBits
// @tparam table bits the table of AI bits
// @usage 
// local sas = TEN.Objects.GetMoveableByName("sas_enemy")
// sas:SetAIBits({1, 0, 0, 0, 0, 0})
		ScriptReserved_SetAIBits, &Moveable::SetAIBits,

/// Get the hit status of the object
// @function Moveable:GetHitStatus
// @treturn bool true if the moveable was hit by something in the last gameplay frame, false otherwise 
		ScriptReserved_GetHitStatus, &Moveable::GetHitStatus,

/// Determine whether the moveable is active or not 
// @function Moveable:GetActive
// @treturn bool true if the moveable is active
		ScriptReserved_GetActive, &Moveable::GetActive,

/// Get the current room of the object
// @function Moveable:GetRoom
// @treturn int number representing the current room of the object
		ScriptReserved_GetRoom, &Moveable::GetRoom,

/// Set room of object 
// This is used in conjunction with SetPosition to teleport an item to a new room.
// @function Moveable:SetRoom
// @tparam int ID the ID of the new room 
// @usage 
// local sas = TEN.Objects.GetMoveableByName("sas_enemy")
// sas:SetRoom(destinationRoom)
// sas:SetPosition(destinationPosition)
	ScriptReserved_SetRoom, &Moveable::SetRoom,


/// Get the object's position
// @function Moveable:GetPosition
// @treturn Position a copy of the moveable's position
	ScriptReserved_GetPosition, & Moveable::GetPos,

/// Set the moveable's position
// If you are moving a moveable whose behaviour involves knowledge of room geometry,
// (e.g. a BADDY1, which uses it for pathfinding), then you *must* use this in conjunction
// with @{Moveable:SetRoom}. Otherwise, said moveable will not behave correctly.
// @function Moveable:SetPosition
// @tparam Position position the new position of the moveable 
	ScriptReserved_SetPosition, & Moveable::SetPos,

/// Get the moveable's rotation
// @function GetRotation
// @treturn Rotation a copy of the moveable's rotation
	ScriptReserved_GetRotation, &Moveable::GetRot,

/// Set the moveable's rotation
// @function SetRotation
// @tparam Rotation The moveable's new rotation
	ScriptReserved_SetRotation, &Moveable::SetRot,

/// Set the moveable's name (its unique string identifier)
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// This corresponds with the "Lua Name" field in an object's properties in Tomb Editor.
// @function GetName
// @treturn string the moveable's name
	ScriptReserved_GetName, &Moveable::GetName,

/// Set the moveable's name (its unique string identifier)
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// It cannot be blank and cannot share a name with any existing object.
// @function SetName
// @tparam name string the new moveable's name
// @treturn bool true if we successfully set the name, false otherwise (e.g. if another object has the name already)
	ScriptReserved_SetName, &Moveable::SetName, 

/// Test if the object is in a valid state (i.e. has not been destroyed through Lua or killed by Lara).
// @function GetValid
// @treturn valid bool true if the object is still not destroyed
	ScriptReserved_GetValid, &Moveable::GetValid,

/// Destroy the moveable. This will mean it can no longer be used, except to re-initialise it with another object.
// @function Destroy
	ScriptReserved_Destroy, &Moveable::Destroy);
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

void Moveable::SetObjectID(GAME_OBJECT_ID id) 
{
	m_item->objectNumber = id;
}


void Moveable::SetOnHit(std::string const & cbName)
{
	m_item->luaCallbackOnHitName = cbName;
}

void Moveable::SetOnKilled(std::string const & cbName)
{
	m_item->luaCallbackOnKilledName = cbName;
}

void Moveable::SetOnCollidedWithObject(std::string const & cbName)
{
	m_item->luaCallbackOnCollidedWithObjectName = cbName;

	if(cbName.empty())
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryRemoveColliding(m_num);
	else
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(m_num);
}

std::string Moveable::GetOnHit() const
{
	return m_item->luaCallbackOnHitName;
}

std::string Moveable::GetOnKilled() const
{
	return m_item->luaCallbackOnKilledName;
}

std::string Moveable::GetOnCollidedWithObject() const
{
	return m_item->luaCallbackOnCollidedWithObjectName;
}

std::string Moveable::GetName() const
{
	return m_item->luaName;
}

bool Moveable::SetName(std::string const & id) 
{
	if (!ScriptAssert(!id.empty(), "Name cannot be blank. Not setting name."))
	{
		return false;
	}

	if (s_callbackSetName(id, m_num))
	{
		// remove the old name if we have one
		if (id != m_item->luaName)
		{
			s_callbackRemoveName(m_item->luaName);
			m_item->luaName = id;
		}
	}
	else
	{
		ScriptAssertF(false, "Could not add name {} - does an object with this name already exist?", id);
		TENLog("Name will not be set", LogLevel::Warning, LogConfig::All);

		return false;
	}
	return true;
}

Position Moveable::GetPos() const
{
	return Position(m_item->pos	);
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
	return {
		static_cast<int>(TO_DEGREES(m_item->pos.xRot)) % 360,
		static_cast<int>(TO_DEGREES(m_item->pos.yRot)) % 360,
		static_cast<int>(TO_DEGREES(m_item->pos.zRot)) % 360
	};
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
	for (size_t i = 0; i < ret.size(); ++i)
	{
		uint8_t isSet = m_item->aiBits & (1 << i);
		ret[i] = static_cast<int>( isSet > 0);
	}
	return ret;
}

void Moveable::SetAIBits(aiBitsType const & bits)
{
	for (size_t i = 0; i < bits.value().size(); ++i)
	{
		m_item->aiBits &= ~(1 << i);
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
	bool cond = frameNumber < frameCount;
	const char* err = "Invalid frame number {}; max frame number for anim {} is {}.";
	if (ScriptAssertF(cond, err, frameNumber, m_item->animNumber, frameCount-1))
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

		// Try add colliding in case the item went from invisible -> activated
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(m_num);
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
		// Try add colliding in case the item went from invisible -> deactivated
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(m_num);
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
	dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryRemoveColliding(m_num);
}

void Moveable::Invalidate()
{
	m_item = nullptr;
	m_num = NO_ITEM;
	m_initialised = false;
}

bool Moveable::GetValid() const
{
	return m_item != nullptr;
}

void Moveable::Destroy()
{
	if (m_num > NO_ITEM) {
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->RemoveMoveableFromMap(m_item, this);
		s_callbackRemoveName(m_item->luaName);
		KillItem(m_num);
	}

	Invalidate();
}

#endif
