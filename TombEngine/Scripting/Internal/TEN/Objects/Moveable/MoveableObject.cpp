#include "framework.h"

#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/effects/debris.h"
#include "Game/effects/item_fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/objectslist.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Math/Math.h"

#include "ScriptAssert.h"
#include "MoveableObject.h"
#include "ScriptUtil.h"
#include "Objects/ObjectsHandler.h"
#include "ReservedScriptNames.h"
#include "Color/Color.h"
#include "Logic/LevelFunc.h"
#include "Rotation/Rotation.h"
#include "Vec3/Vec3.h"

using namespace TEN::Effects::Items;

/***
Represents any object inside the game world.
Examples include traps, enemies, doors,
pickups, and Lara herself (see also @{Objects.LaraObject} for Lara-specific features).

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
}


Moveable::~Moveable()
{
	if (m_item && g_GameScriptEntities) 
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->RemoveMoveableFromMap(m_item, this);
}

bool operator==(Moveable const& first, Moveable const& second)
{
	return first.m_item == second.m_item;
}

/*** For more information on each parameter, see the
associated getters and setters. If you do not know what to set for these,
most can just be ignored (see usage).
	@function Moveable
	@tparam Objects.ObjID object ID
	@tparam string name Lua name of the item
	@tparam Vec3 position position in level
	@tparam[opt] Rotation rotation rotation about x, y, and z axes (default Rotation(0, 0, 0))
	@int[opt] room room ID item is in (default: calculated automatically)
	@int[opt=0] animNumber anim number
	@int[opt=0] frameNumber frame number
	@int[opt=10] hp HP of item
	@int[opt=0] OCB ocb of item (default 0)
	@tparam[opt] table AIBits table with AI bits (default {0,0,0,0,0,0})
	@treturn Moveable A new Moveable object (a wrapper around the new object)
	@usage 
	local item = Moveable(
		TEN.Objects.ObjID.PISTOLS_ITEM, -- object id
		"test", -- name
		Vec3(18907, 0, 21201)
		)
	*/
static std::unique_ptr<Moveable> Create(
	GAME_OBJECT_ID objID,
	std::string const & name,
	Vec3 const & pos,
	TypeOrNil<Rotation> const & rot,
	TypeOrNil<short> room,
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
		ItemInfo* item = &g_Level.Items[num];
		if (std::holds_alternative<short>(room))
		{
			ptr->SetPos(pos, false);
			ptr->SetRoomNumber(std::get<short>(room));
		}
		else
			ptr->SetPos(pos, true);

		ptr->SetRot(USE_IF_HAVE(Rotation, rot, Rotation{}));
		ptr->SetObjectID(objID);
		ptr->Init();

		ptr->SetAnimNumber(USE_IF_HAVE(int, animNumber, 0));
		ptr->SetFrameNumber(USE_IF_HAVE(int, frameNumber, 0));
		ptr->SetHP(USE_IF_HAVE(short, hp, 10));
		ptr->SetOCB(USE_IF_HAVE(short, ocb, 0));
		ptr->SetAIBits(USE_IF_HAVE(aiBitsType, aiBits, aiBitsType{}));
		ptr->SetColor(ScriptColor(Vector4::One));
		item->CarriedItem = NO_ITEM;

		// call this when resetting name too?
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->AddMoveableToMap(item, ptr.get());
		// add to name map too?
	}
	return ptr;
}

void Moveable::Register(sol::table & parent)
{
	parent.new_usertype<Moveable>(LUA_CLASS_NAME,
		sol::call_constructor, Create,
		sol::meta_function::index, index_error,
		sol::meta_function::new_index, newindex_error,
		sol::meta_function::equal_to, std::equal_to<Moveable const>(),

	ScriptReserved_Enable, &Moveable::EnableItem,

	ScriptReserved_Disable, &Moveable::DisableItem,

	ScriptReserved_MakeInvisible, &Moveable::MakeInvisible,

	ScriptReserved_SetVisible, &Moveable::SetVisible,

/// Explode item. This also kills and disables item.
// @function Moveable:Explode
	ScriptReserved_Explode, &Moveable::Explode,

/// Shatter item. This also kills and disables item.
// @function Moveable:Shatter
	ScriptReserved_Shatter, &Moveable::Shatter,

/// Set effect to moveable
// @function Moveable:SetEffect
// @tparam Effects.EffectID effect Type of effect to assign.
// @tparam float timeout time (in seconds) after which effect turns off (optional).
	ScriptReserved_SetEffect, &Moveable::SetEffect,

/// Set custom colored burn effect to moveable
// @function Moveable:SetCustomEffect
// @tparam Color Color1 color the primary color of the effect (also used for lighting).
// @tparam Color Color2 color the secondary color of the effect.
// @tparam float timeout time (in seconds) after which effect turns off (optional).
	ScriptReserved_SetCustomEffect, &Moveable::SetCustomEffect,

/// Get current moveable effect
// @function Moveable:GetEffect
// @treturn Effects.EffectID effect type currently assigned to moveable.
	ScriptReserved_GetEffect, & Moveable::GetEffect,

/// Get the status of object.
// possible values:
// <br />0 - not active 
// <br />1 - active 
// <br />2 - deactivated 
// <br />3 - invisible
// @function Moveable:GetStatus
// @treturn int a number representing the status of the object
	ScriptReserved_GetStatus, &Moveable::GetStatus,

/// Set the name of the function to be called when the moveable is shot by Lara.
// Note that this will be triggered twice when shot with both pistols at once. 
// @function Moveable:SetOnHit
// @tparam function callback function in LevelFuncs hierarchy to call when moveable is shot
	ScriptReserved_SetOnHit, &Moveable::SetOnHit,

	ScriptReserved_SetOnCollidedWithObject, &Moveable::SetOnCollidedWithObject,

	ScriptReserved_SetOnCollidedWithRoom, &Moveable::SetOnCollidedWithRoom,

/// Set the name of the function to be called when the moveable is destroyed/killed
// Note that enemy death often occurs at the end of an animation, and not at the exact moment
// the enemy's HP becomes zero.
// @function Moveable:SetOnKilled
// @tparam function callback function in LevelFuncs hierarchy to call when enemy is killed
// @usage
// LevelFuncs.baddyKilled = function(theBaddy) print("You killed a baddy!") end
// baddy:SetOnKilled(LevelFuncs.baddyKilled)
	ScriptReserved_SetOnKilled, &Moveable::SetOnKilled,

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

/// Retrieve the index of the current state.
// This corresponds to the number shown in the item's state ID field in WadTool.
// @function Moveable:GetState
// @treturn int the index of the active state
	ScriptReserved_GetStateNumber, &Moveable::GetStateNumber,

/// Set the object's state to the one specified by the given index.
// Performs no bounds checking. *Ensure the number given is correct, else
// object may end up in corrupted animation state.*
// @function Moveable:SetState
// @tparam int index the index of the desired state 
	ScriptReserved_SetStateNumber, &Moveable::SetStateNumber,

/// Retrieve the index of the current animation.
// This corresponds to the number shown in the item's animation list in WadTool.
// @function Moveable:GetAnim
// @treturn int the index of the active animation
	ScriptReserved_GetAnimNumber, &Moveable::GetAnimNumber,

/// Set the object's animation to the one specified by the given index.
// Performs no bounds checking. *Ensure the number given is correct, else
// object may end up in corrupted animation state.*
// @function Moveable:SetAnim
// @tparam int index the index of the desired anim 
	ScriptReserved_SetAnimNumber, &Moveable::SetAnimNumber,

/// Retrieve frame number.
// This is the current frame of the object's active animation.
// @function Moveable:GetFrame
// @treturn int the current frame of the active animation
	ScriptReserved_GetFrameNumber, &Moveable::GetFrameNumber,

/// Set the object's velocity to specified value.
// In most cases, only Z and Y components are used as forward and vertical velocity.
// In some cases, primarily NPCs, X component is used as side velocity.
// @function Moveable:SetVelocity
// @tparam Vec3 velocity velocity represented as vector 
	ScriptReserved_SetVelocity, &Moveable::SetVelocity,
		
/// Get the object's velocity.
// In most cases, only Z and Y components are used as forward and vertical velocity.
// In some cases, primarily NPCs, X component is used as side velocity.
// @function Moveable:GetVelocity
// @treturn Vec3 current object velocity
	ScriptReserved_GetVelocity, &Moveable::GetVelocity,

/// Set frame number.
// This will move the animation to the given frame.
// The number of frames in an animation can be seen under the heading "End frame" in
// the WadTool animation editor. If the animation has no frames, the only valid argument
// is -1.
// @function Moveable:SetFrame
// @tparam int frame the new frame number
	ScriptReserved_SetFrameNumber, &Moveable::SetFrameNumber,
		
	ScriptReserved_GetHP, &Moveable::GetHP,

	ScriptReserved_SetHP, &Moveable::SetHP,

/// Get HP definded for that object type (hit points/health points) (Read Only).
// @function Moveable:GetSlotHP
// @tparam int ID of the moveable slot type.
ScriptReserved_GetSlotHP, & Moveable::GetSlotHP,

/// Get OCB (object code bit) of the moveable
// @function Moveable:GetOCB
// @treturn int the moveable's current OCB value
	ScriptReserved_GetOCB, &Moveable::GetOCB,

/// Set OCB (object code bit) of the moveable
// @function Moveable:SetOCB
// @tparam int OCB the new value for the moveable's OCB
	ScriptReserved_SetOCB, &Moveable::SetOCB,

/// Get the value stored in ItemFlags[index]
// @function Moveable:GetItemFlags
// @tparam index: (short) index the ItemFlags, can be between 0 and 7.
// @treturn (short) the value contained in the ItemFlags[index]
	ScriptReserved_GetItemFlags, & Moveable::GetItemFlags,

/// Stores a value in ItemFlags[index]
// @function Moveable:SetItemFlags
// @tparam value: (short) value to store in the moveable's ItemFlags[index]
// @tparam index: (short) index of the ItemFlags where store the value.
	ScriptReserved_SetItemFlags, & Moveable::SetItemFlags,

	ScriptReserved_GetLocationAI, & Moveable::GetLocationAI,

	ScriptReserved_SetLocationAI, & Moveable::SetLocationAI,

/// Get the moveable's color
// @function Moveable:GetColor
// @treturn Color a copy of the moveable's color
	ScriptReserved_GetColor, &Moveable::GetColor,

/// Set the moveable's color
// @function Moveable:SetColor
// @tparam Color color the new color of the moveable 
	ScriptReserved_SetColor, &Moveable::SetColor,

	ScriptReserved_GetAIBits, &Moveable::GetAIBits, 
			
	ScriptReserved_SetAIBits, &Moveable::SetAIBits,

	ScriptReserved_GetMeshVisable, &Moveable::GetMeshVisible,
			
	ScriptReserved_SetMeshVisible, &Moveable::SetMeshVisible,
			
	ScriptReserved_ShatterMesh, &Moveable::ShatterMesh,

	ScriptReserved_GetMeshSwapped, &Moveable::GetMeshSwapped,
			
	ScriptReserved_SwapMesh, &Moveable::SwapMesh,
			
	ScriptReserved_UnswapMesh, &Moveable::UnswapMesh,

/// Get the hit status of the object
// @function Moveable:GetHitStatus
// @treturn bool true if the moveable was hit by something in the last gameplay frame, false otherwise 
	ScriptReserved_GetHitStatus, &Moveable::GetHitStatus,

/// Determine whether the moveable is active or not 
// @function Moveable:GetActive
// @treturn bool true if the moveable is active
	ScriptReserved_GetActive, &Moveable::GetActive,

	ScriptReserved_GetRoom, &Moveable::GetRoom,

	ScriptReserved_GetRoomNumber, &Moveable::GetRoomNumber,

	ScriptReserved_SetRoomNumber, &Moveable::SetRoomNumber,

	ScriptReserved_GetPosition, & Moveable::GetPos,

/// Get the object's joint position
// @function Moveable:GetJointPosition
// @tparam int index of a joint to get position
// @treturn Vec3 a copy of the moveable's position
	ScriptReserved_GetJointPosition, & Moveable::GetJointPos,

	ScriptReserved_SetPosition, & Moveable::SetPos,

/// Get the moveable's rotation
// @function Moveable:GetRotation
// @treturn Rotation a copy of the moveable's rotation
	ScriptReserved_GetRotation, &Moveable::GetRot,

/// Set the moveable's rotation
// @function Moveable:SetRotation
// @tparam Rotation rotation The moveable's new rotation
	ScriptReserved_SetRotation, &Moveable::SetRot,

/// Get the moveable's name (its unique string identifier)
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// This corresponds with the "Lua Name" field in an object's properties in Tomb Editor.
// @function Moveable:GetName
// @treturn string the moveable's name
	ScriptReserved_GetName, &Moveable::GetName,

/// Set the moveable's name (its unique string identifier)
// e.g. "door\_back\_room" or "cracked\_greek\_statue"
// It cannot be blank and cannot share a name with any existing object.
// @function Moveable:SetName
// @tparam name string the new moveable's name
// @treturn bool true if we successfully set the name, false otherwise (e.g. if another object has the name already)
	ScriptReserved_SetName, &Moveable::SetName, 

/// Test if the object is in a valid state (i.e. has not been destroyed through Lua or killed by Lara).
// @function Moveable:GetValid
// @treturn bool valid true if the object is still not destroyed
	ScriptReserved_GetValid, &Moveable::GetValid,

/// Destroy the moveable. This will mean it can no longer be used, except to re-initialise it with another object.
// @function Moveable:Destroy
	ScriptReserved_Destroy, &Moveable::Destroy,

/// Attach camera to an object.
// @function Moveable:AttachObjCamera
// @tparam int mesh of a moveable to use as a camera position
// @tparam Moveable target moveable to attach camera to
// @tparam int mesh of a target moveable to use as a camera target
	ScriptReserved_AttachObjCamera, &Moveable::AttachObjCamera,

/// Borrow animation from an object
// @function Moveable:AnimFromObject
// @tparam Objects.ObjID ObjectID to take animation and stateID from,
// @tparam int animNumber animation from object
// @tparam int stateID state from object
	ScriptReserved_AnimFromObject, &Moveable::AnimFromObject);
}

void Moveable::Init()
{
	bool cond = IsPointInRoom(m_item->Pose.Position, m_item->RoomNumber);
	std::string err{ "Position of item \"{}\" does not match its room ID." };
	if (!ScriptAssertF(cond, err, m_item->Name))
	{
		ScriptWarn("Resetting to the center of the room.");
		auto center = GetRoomCenter(m_item->RoomNumber);
		// reset position but not rotation
		m_item->Pose.Position = center;
	}
	InitialiseItem(m_num);
	m_initialised = true;
}

GAME_OBJECT_ID Moveable::GetObjectID() const
{
	return m_item->ObjectNumber;
}

void Moveable::SetObjectID(GAME_OBJECT_ID id) 
{
	m_item->ObjectNumber = id;
	m_item->ResetModelToDefault();
}

void SetLevelFuncCallback(TypeOrNil<LevelFunc> const & cb, std::string const & callerName, Moveable & mov, std::string & toModify)
{
	if (std::holds_alternative<LevelFunc>(cb))
	{
		toModify = std::get<LevelFunc>(cb).m_funcName;
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(mov.m_num);
	}
	else if (std::holds_alternative<sol::nil_t>(cb))
	{
		toModify = std::string{};
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryRemoveColliding(mov.m_num);
	}
	else
	{
		ScriptAssert(false, "Tried giving " + mov.m_item->Name
			+ " a non-LevelFunc object as an arg to "
			+ callerName);
	}

}

short Moveable::GetIndex() const
{
	return m_num;
}

void Moveable::SetOnHit(TypeOrNil<LevelFunc> const & cb)
{
	SetLevelFuncCallback(cb, ScriptReserved_SetOnHit, *this, m_item->Callbacks.OnHit);
}

void Moveable::SetOnKilled(TypeOrNil<LevelFunc> const & cb)
{
	SetLevelFuncCallback(cb, ScriptReserved_SetOnKilled, *this, m_item->Callbacks.OnKilled);
}

/// Set the function to be called when this moveable collides with another moveable
// @function Moveable:SetOnCollidedWithObject
// @tparam function func callback function to be called (must be in LevelFuncs hierarchy). This function can take two arguments; these will store the two @{Moveable} taking part in the collision.
// @usage
// LevelFuncs.objCollided = function(obj1, obj2)
//     print(obj1:GetName() .. " collided with " .. obj2:GetName())
// end
// baddy:SetOnCollidedWithObject(LevelFuncs.objCollided)
void Moveable::SetOnCollidedWithObject(TypeOrNil<LevelFunc> const & cb)
{
	SetLevelFuncCallback(cb, ScriptReserved_SetOnCollidedWithObject, *this, m_item->Callbacks.OnObjectCollided);
}

/// Set the function called when this moveable collides with room geometry (e.g. a wall or floor). This function can take an argument that holds the @{Moveable} that collided with geometry.
// @function Moveable:SetOnCollidedWithRoom
// @tparam function func callback function to be called (must be in LevelFuncs hierarchy)
// @usage
// LevelFuncs.roomCollided = function(obj)
//     print(obj:GetName() .. " collided with room geometry")
// end
// baddy:SetOnCollidedWithRoom(LevelFuncs.roomCollided)
void Moveable::SetOnCollidedWithRoom(TypeOrNil<LevelFunc> const & cb)
{
	SetLevelFuncCallback(cb, ScriptReserved_SetOnCollidedWithRoom, *this, m_item->Callbacks.OnRoomCollided);
}

std::string Moveable::GetName() const
{
	return m_item->Name;
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
		if (id != m_item->Name)
		{
			if(!m_item->Name.empty())
				s_callbackRemoveName(m_item->Name);
			m_item->Name = id;
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

/// Get the object's position
// @function Moveable:GetPosition
// @treturn Vec3 a copy of the moveable's position
Vec3 Moveable::GetPos() const
{
	return Vec3(m_item->Pose);
}

/// Set the moveable's position
// If you are moving a moveable whose behaviour involves knowledge of room geometry,
// (e.g. a BADDY1, which uses it for pathfinding), then the second argument should
// be true (or omitted, as true is the default). Otherwise, said moveable will not behave correctly.
// @function Moveable:SetPosition
// @tparam Vec3 position the new position of the moveable 
// @bool[opt] updateRoom Will room changes be automatically detected? Set to false if you are using overlapping rooms (default: true)
void Moveable::SetPos(Vec3 const& pos, sol::optional<bool> updateRoom)
{
	auto oldPos = m_item->Pose.Position.ToVector3();
	pos.StoreInPHDPos(m_item->Pose);

	bool willUpdate = !updateRoom.has_value() || updateRoom.value();

	if (m_initialised && willUpdate)
	{
		bool roomUpdated = false;

		if (m_item->IsLara())
			roomUpdated = UpdateLaraRoom(m_item, pos.y);
		else
			roomUpdated = UpdateItemRoom(m_item->Index);

		// In case direct portal room update didn't happen, and distance between old and new
		// points is significant, do a predictive room update.

		if (!roomUpdated && 
			(willUpdate || Vector3::Distance(oldPos, m_item->Pose.Position.ToVector3()) > BLOCK(1)))
		{
			int potentialNewRoom = FindRoomNumber(m_item->Pose.Position, m_item->RoomNumber);
			if (potentialNewRoom != m_item->RoomNumber)
				SetRoomNumber(potentialNewRoom);
		}
	}
}

Vec3 Moveable::GetJointPos(int jointIndex) const
{
	auto result = GetJointPosition(m_item, jointIndex);
	return Vec3(result.x, result.y, result.z);
}

// This does not guarantee that the returned value will be identical
// to a value written in via SetRot - only that the angle measures
// will be mathematically equal
// (e.g. 90 degrees = -270 degrees = 450 degrees)
Rotation Moveable::GetRot() const
{
	return 
	{
		TO_DEGREES(m_item->Pose.Orientation.x),
		TO_DEGREES(m_item->Pose.Orientation.y),
		TO_DEGREES(m_item->Pose.Orientation.z)
	};
}

void Moveable::SetRot(Rotation const& rot)
{
	m_item->Pose.Orientation.x = ANGLE(rot.x);
	m_item->Pose.Orientation.y = ANGLE(rot.y);
	m_item->Pose.Orientation.z = ANGLE(rot.z);
}

/// Get current HP (hit points/health points)
// @function Moveable:GetHP
// @treturn int the amount of HP the moveable currently has
short Moveable::GetHP() const
{
	return m_item->HitPoints;
}

/// Set current HP (hit points/health points)
// Clamped to [0, 32767] for "intelligent" entities (i.e. anything with AI); clamped to [-32767, 32767] otherwise.
// @function Moveable:SetHP
// @tparam int HP the amount of HP to give the moveable
void Moveable::SetHP(short hp)
{
	if(Objects[m_item->ObjectNumber].intelligent && hp < 0)
	{
		if (hp != NOT_TARGETABLE)
		{
			ScriptAssert(false, "Invalid HP value: " + std::to_string(hp));
			ScriptWarn("Setting HP to 0.");
			hp = 0;
		}
	}

	m_item->HitPoints = hp;
}

short Moveable::GetSlotHP() const
{
	return Objects[m_item->ObjectNumber].HitPoints;
}

short Moveable::GetOCB() const
{
	return m_item->TriggerFlags;
}

void Moveable::SetOCB(short ocb)
{
	m_item->TriggerFlags = ocb;
}

void Moveable::SetEffect(EffectType effectType, sol::optional<float> timeout)
{
	int realTimeout = timeout.has_value() ? int(timeout.value() * FPS) : -1;

	switch (effectType)
	{
	case EffectType::None:
		m_item->Effect.Type = EffectType::None;
		break;

	case EffectType::Smoke:
		ItemSmoke(m_item, realTimeout);
		break;

	case EffectType::Fire:
		ItemBurn(m_item, realTimeout);
		break;

	case EffectType::Sparks:
		ItemElectricBurn(m_item, realTimeout);
		break;

	case EffectType::ElectricIgnite:
		ItemBlueElectricBurn(m_item, realTimeout);
		break;

	case EffectType::RedIgnite:
		ItemRedLaserBurn(m_item, realTimeout);
		break;

	case EffectType::Custom:
		ScriptWarn("CUSTOM effect type requires additional setup. Use SetCustomEffect command instead.");
	}
}

void Moveable::SetCustomEffect(const ScriptColor& col1, const ScriptColor& col2, sol::optional<float> timeout)
{
	int realTimeout = timeout.has_value() ? int(timeout.value() * FPS) : -1;
	Vector3 color1 = Vector3(col1.GetR() * (1.f / 255.f), col1.GetG() * (1.f / 255.f), col1.GetB() * (1.f / 255.f));
	Vector3 color2 = Vector3(col2.GetR() * (1.f / 255.f), col2.GetG() * (1.f / 255.f), col2.GetB() * (1.f / 255.f));
	ItemCustomBurn(m_item, color1, color2, realTimeout);
}

EffectType Moveable::GetEffect() const
{
	return m_item->Effect.Type;
}

short Moveable::GetItemFlags(int index) const
{
	return m_item->ItemFlags[index];
}

void Moveable::SetItemFlags(short value, int index)
{
	m_item->ItemFlags[index] = value;
}

/// Get the location value stored in the Enemy AI
// @function Moveable:GetLocationAI
// @treturn (short) the value contained in the LocationAI of the creature.
short Moveable::GetLocationAI() const
{
	if (m_item->IsCreature())
	{
		auto creature = (CreatureInfo*)m_item->Data;
		return creature->LocationAI;
	}
	TENLog("Trying to get LocationAI value from a non creature moveable. Value does not exist so it's returning 0.", LogLevel::Error);
	return 0;
}

/// Updates the location in the enemy AI with the given value.
// @function Moveable:SetLocationAI
// @tparam value: (short) value to store.
void Moveable::SetLocationAI(short value)
{
	if (m_item->IsCreature())
	{
		auto creature = (CreatureInfo*)m_item->Data;
		creature->LocationAI = value;
	}
	else
		TENLog("Trying to set a value in nonexisting variable. Non creature moveable hasn't got LocationAI.", LogLevel::Error);
}

ScriptColor Moveable::GetColor() const
{
	return ScriptColor{ m_item->Model.Color };
}

void Moveable::SetColor(const ScriptColor& color)
{
	m_item->Model.Color = color;
}

/// Get AIBits of object
// This will return a table with six values, each corresponding to
// an active behaviour. If the object is in a certain AI mode, the table will
// have a *1* in the corresponding cell. Otherwise, the cell will hold
// a *0*.
//
// <br />1 - guard
// <br />2 - ambush
// <br />3 - patrol 1
// <br />4 - modify
// <br />5 - follow
// <br />6 - patrol 2
// @function Moveable:GetAIBits
// @treturn table a table of AI bits
aiBitsType Moveable::GetAIBits() const
{
	static_assert(63 == ALL_AIOBJ);

	aiBitsArray ret{};
	for (size_t i = 0; i < ret.size(); ++i)
	{
		uint8_t isSet = m_item->AIBits & (1 << i);
		ret[i] = static_cast<int>( isSet > 0);
	}
	return ret;
}

/// Set AIBits of object
// Use this to force a moveable into a certain AI mode or modes, as if a certain nullmesh
// (or more than one) had suddenly spawned beneath their feet.
// @function Moveable:SetAIBits
// @tparam table bits the table of AI bits
// @usage 
// local sas = TEN.Objects.GetMoveableByName("sas_enemy")
// sas:SetAIBits({1, 0, 0, 0, 0, 0})
void Moveable::SetAIBits(aiBitsType const & bits)
{
	for (size_t i = 0; i < bits.value().size(); ++i)
	{
		m_item->AIBits &= ~(1 << i);
		uint8_t isSet = bits.value()[i] > 0;
		m_item->AIBits |= isSet << i;
	}
}

int Moveable::GetStateNumber() const
{
	return m_item->Animation.ActiveState;
}

void Moveable::SetStateNumber(int stateNumber)
{
	m_item->Animation.TargetState = stateNumber;
}

int Moveable::GetAnimNumber() const
{
	return m_item->Animation.AnimNumber - Objects[m_item->ObjectNumber].animIndex;
}

void Moveable::SetAnimNumber(int animNumber)
{
	SetAnimation(m_item, animNumber);
}

int Moveable::GetFrameNumber() const
{
	return m_item->Animation.FrameNumber - g_Level.Anims[m_item->Animation.AnimNumber].frameBase;
}

Vec3 Moveable::GetVelocity() const
{
	return Vec3(int(round(m_item->Animation.Velocity.x)),
				int(round(m_item->Animation.Velocity.y)),
				int(round(m_item->Animation.Velocity.z)));
}

void Moveable::SetVelocity(Vec3 velocity)
{
	if (m_item->IsCreature())
		ScriptWarn("Attempt to set velocity to a creature. In may not work, as velocity is overridden by AI.");

	m_item->Animation.Velocity = Vector3(velocity.x, velocity.y, velocity.z);
}

void Moveable::SetFrameNumber(int frameNumber)
{
	auto const fBase = g_Level.Anims[m_item->Animation.AnimNumber].frameBase;
	auto const fEnd = g_Level.Anims[m_item->Animation.AnimNumber].frameEnd;
	auto frameCount = fEnd - fBase;
	bool cond = frameNumber < frameCount;
	const char* err = "Invalid frame number {}; max frame number for anim {} is {}.";
	if (ScriptAssertF(cond, err, frameNumber, m_item->Animation.AnimNumber, frameCount-1))
	{
		m_item->Animation.FrameNumber = frameNumber + fBase;
	}
	else
	{
		ScriptWarn("Not setting frame number.");
	}
}

bool Moveable::GetActive() const
{
	return m_item->Active;
}

void Moveable::SetActive(bool active)
{
	m_item->Active = active;
}

bool Moveable::GetHitStatus() const
{
	return m_item->HitStatus;
}

/// Get the current room of the object
// @function Moveable:GetRoom
// @treturn Room current room of the object
std::unique_ptr<Room> Moveable::GetRoom() const
{
	return std::make_unique<Room>(g_Level.Rooms[m_item->RoomNumber]);
}

/// Get the current room number of the object
// @function Moveable:GetRoomNumber
// @treturn int number representing the current room of the object
int Moveable::GetRoomNumber() const
{
	return m_item->RoomNumber;
}

/// Set room number of object 
// Use this if you are not using SetPosition's automatic room update - for example, when dealing with overlapping rooms.
// @function Moveable:SetRoomNumber
// @tparam int ID the ID of the new room 
// @usage 
// local sas = TEN.Objects.GetMoveableByName("sas_enemy")
// sas:SetRoomNumber(destinationRoom)
// sas:SetPosition(destinationPosition, false)
void Moveable::SetRoomNumber(short room)
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
	{
		ItemNewRoom(m_num, room);

		// HACK: For Lara, we need to manually force Location.roomNumber to new one,
		// or else camera won't be updated properly.

		if (m_item->IsLara())
			m_item->Location.roomNumber = room;
	}
}

short Moveable::GetStatus() const
{
	return m_item->Status;
}

/// Get state of specified mesh visibility of object
// Returns true if specified mesh is visible on an object, and false
// if it is not visible.
// @function Moveable:GetMeshVisible
// @int index index of a mesh
// @treturn bool visibility status
bool Moveable::GetMeshVisible(int meshId) const
{
	if (!MeshExists(meshId))
		return false;

	return m_item->MeshBits.Test(meshId);
}

/// Makes specified mesh visible or invisible
// Use this to show or hide a specified mesh of an object.
// @function Moveable:SetMeshVisible
// @int index index of a mesh
// @bool visible true if you want the mesh to be visible, false otherwise
void Moveable::SetMeshVisible(int meshId, bool visible)
{
	if (!MeshExists(meshId))
		return;

	if (visible)
		m_item->MeshBits.Set(meshId);
	else
		m_item->MeshBits.Clear(meshId);
}

/// Shatters specified mesh and makes it invisible
// Note that you can re-enable mesh later by using SetMeshVisible().
// @function Moveable:ShatterMesh
// @int index index of a mesh
void Moveable::ShatterMesh(int meshId)
{
	if (!MeshExists(meshId))
		return;

	ExplodeItemNode(m_item, meshId, 0, 128);
}

/// Get state of specified mesh swap of object
// Returns true if specified mesh is swapped on an object, and false
// if it is not swapped.
// @function Moveable:GetMeshSwapped
// @int index index of a mesh
// @treturn bool mesh swap status
bool Moveable::GetMeshSwapped(int meshId) const
{
	if (!MeshExists(meshId))
		return false;

	return m_item->Model.MeshIndex[meshId] == m_item->Model.BaseMesh + meshId;
}

/// Set state of specified mesh swap of object
// Use this to swap specified mesh of an object.
// @function Moveable:SwapMesh
// @int index index of a mesh
// @int slotIndex index of a slot to get meshswap from
// @int[opt] swapIndex index of a mesh from meshswap slot to use
void Moveable::SwapMesh(int meshId, int swapSlotId, sol::optional<int> swapMeshIndex)
{
	if (!MeshExists(meshId))
		return;

	if (!swapMeshIndex.has_value())
		 swapMeshIndex = meshId;


	if (swapSlotId <= -1 || swapSlotId >= ID_NUMBER_OBJECTS)
	{
		TENLog("Specified meshswap slot ID is incorrect!", LogLevel::Error);
		return;
	}

	if (!Objects[swapSlotId].loaded)
	{
		TENLog("Object in specified meshswap slot doesn't exist in level!", LogLevel::Error);
		return;
	}

	if (swapMeshIndex.value() >= Objects[swapSlotId].nmeshes)
	{
		TENLog("Specified meshswap index does not exist in meshswap slot!", LogLevel::Error);
		return;
	}

	m_item->Model.MeshIndex[meshId] = Objects[swapSlotId].meshIndex + swapMeshIndex.value();
}

/// Set state of specified mesh swap of object
// Use this to bring back original unswapped mesh
// @function Moveable:UnswapMesh
// @int index index of a mesh to unswap
void Moveable::UnswapMesh(int meshId)
{
	if (!MeshExists(meshId))
		return;

	m_item->Model.MeshIndex[meshId] = m_item->Model.BaseMesh + meshId;
}

/// Enable the item, as if a trigger for it had been stepped on.
// @function Moveable:Enable
void Moveable::EnableItem()
{
	if (m_num == NO_ITEM)
		return;

	bool wasInvisible = false;
	if (m_item->Status == ITEM_INVISIBLE)
		wasInvisible = true;

	m_item->Flags |= CODE_BITS;
	Trigger(m_num);

	// Try add colliding in case the item went from invisible -> activated
	if (wasInvisible)
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(m_num);
}

/// Disable the item, as if an antitrigger for it had been stepped on (i.e. it will close an open door or extinguish a flame emitter).
// Note that this will not trigger an OnKilled callback.
// @function Moveable:Disable
void Moveable::DisableItem()
{
	if (m_num == NO_ITEM)
		return;

	Antitrigger(m_num);

	if (m_num > NO_ITEM && (m_item->Status == ITEM_INVISIBLE))
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryRemoveColliding(m_num);
}

void Moveable::Explode()
{
	if (!m_item->Active && !m_item->IsLara())
		return;

	CreatureDie(m_num, true);
}

void Moveable::Shatter()
{
	if (!m_item->Active && !m_item->IsLara())
		return;

	m_item->Flags |= IFLAG_KILLED | IFLAG_INVISIBLE;
	for (int i = 0; i < Objects[m_item->ObjectNumber].nmeshes; i++)
		ExplodeItemNode(m_item, i, 0, 128);

	CreatureDie(m_num, false);
}

/// Make the item invisible. Alias for `Moveable:SetVisible(false)`.
// @function Moveable:MakeInvisible
void Moveable::MakeInvisible()
{
	SetVisible(false);
}
/// Set the item's visibility. __An invisible item will have collision turned off, as if it no longer exists in the game world__.
// @bool visible true if the caller should become visible, false if it should become invisible
// @function Moveable:SetVisible
void Moveable::SetVisible(bool visible)
{
	if (!visible)
	{
		if (Objects[m_item->ObjectNumber].intelligent)
			DisableItem();
		else
			RemoveActiveItem(m_num, false);

		m_item->Status = ITEM_INVISIBLE;

		if (m_num > NO_ITEM)
			dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryRemoveColliding(m_num);
	}
	else
	{
		if (Objects[m_item->ObjectNumber].intelligent)
		{
			if(!(m_item->Flags & IFLAG_KILLED))
				EnableItem();
			else
				m_item->Status = ITEM_ACTIVE;
		}
		else
		{
			AddActiveItem(m_num);
			m_item->Status = ITEM_ACTIVE;
		}

		if (m_num > NO_ITEM)
			dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->TryAddColliding(m_num);
	}
}

void Moveable::Invalidate()
{
	// keep m_item as it is so that we can properly remove it from the moveables set when
	// its destructor is called
	m_num = NO_ITEM;
	m_initialised = false;
}

bool Moveable::GetValid() const
{
	return m_num > NO_ITEM;
}

void Moveable::Destroy()
{
	if (m_num > NO_ITEM) 
	{
		dynamic_cast<ObjectsHandler*>(g_GameScriptEntities)->RemoveMoveableFromMap(m_item, this);
		s_callbackRemoveName(m_item->Name);
		KillItem(m_num);
	}

	Invalidate();
}

bool Moveable::MeshExists(int index) const
{
	if (index < 0 || index >= Objects[m_item->ObjectNumber].nmeshes)
	{
		ScriptAssertF(false, "Mesh index {} does not exist in moveable '{}'", index, m_item->Name);
		return false;
	}

	return true;
}

//Attach camera and camera target to a mesh of an object.
void Moveable::AttachObjCamera(short camMeshId, Moveable& mov, short targetMeshId)
{
	if ((m_item->Active || m_item->IsLara()) && (mov.m_item->Active || mov.m_item->IsLara()))
	{
		ObjCamera(m_item, camMeshId, mov.m_item, targetMeshId, true);
	}
}

//Borrow an animtaion and state id from an object.
void Moveable::AnimFromObject(GAME_OBJECT_ID object, int animNumber, int stateID)
{
	m_item->Animation.AnimNumber = Objects[object].animIndex + animNumber;
	m_item->Animation.ActiveState = stateID;
	m_item->Animation.FrameNumber = g_Level.Anims[m_item->Animation.AnimNumber].frameBase;
	AnimateItem(m_item);
}
