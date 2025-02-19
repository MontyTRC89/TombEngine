#pragma once
#include <sol/sol.hpp>
#include "framework.h"
#include "Game/collision/collide_room.h"
#include "Math/Math.h"
#include "Game/collision/Point.h"
#include "Game/collision/collide_room.h"
#include "Objects/Utils/object_helper.h"
#include "Scripting/Internal/LuaHandler.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Room;
using namespace TEN::Math;

using namespace TEN::Collision::Point;

namespace TEN::Scripting::Collision
{
    void static RegisterPointCollision(sol::state& lua)
    {
        lua.new_usertype<PointCollisionData>("PointCollisionData",
            sol::constructors<PointCollisionData(), PointCollisionData(const Vector3i&, int)>(),

            // Getters
            ScriptReserved_GetColPosition, &PointCollisionData::GetPosition,
            ScriptReserved_GetColRoomNumber, &PointCollisionData::GetRoomNumber,
            ScriptReserved_GetSector, &PointCollisionData::GetSector,
            ScriptReserved_GetBottomSector, &PointCollisionData::GetBottomSector,
            ScriptReserved_GetTopSector, &PointCollisionData::GetTopSector,
            ScriptReserved_GetFloorHeight, &PointCollisionData::GetFloorHeight,
            ScriptReserved_GetCeilingHeight, &PointCollisionData::GetCeilingHeight,
            ScriptReserved_GetFloorNormal, &PointCollisionData::GetFloorNormal,
            ScriptReserved_GetCeilingNormal, &PointCollisionData::GetCeilingNormal,
            ScriptReserved_GetFloorBridgeItemNumber, &PointCollisionData::GetFloorBridgeItemNumber,
            ScriptReserved_GetCeilingBridgeItemNumber, &PointCollisionData::GetCeilingBridgeItemNumber,
            ScriptReserved_GetWaterSurfaceHeight, &PointCollisionData::GetWaterSurfaceHeight,
            ScriptReserved_GetWaterBottomHeight, &PointCollisionData::GetWaterBottomHeight,
            ScriptReserved_GetWaterTopHeight, &PointCollisionData::GetWaterTopHeight,

            // Inquirers
            ScriptReserved_IsWall, &PointCollisionData::IsWall,
            ScriptReserved_IsSteepFloor, &PointCollisionData::IsSteepFloor,
            ScriptReserved_IsSteepCeiling, &PointCollisionData::IsSteepCeiling,
            ScriptReserved_IsDiagonalFloorStep, &PointCollisionData::IsDiagonalFloorStep,
            ScriptReserved_IsDiagonalCeilingStep, &PointCollisionData::IsDiagonalCeilingStep,
            ScriptReserved_IsDiagonalFloorSplit, &PointCollisionData::IsDiagonalFloorSplit,
            ScriptReserved_IsDiagonalCeilingSplit, &PointCollisionData::IsDiagonalCeilingSplit,
            ScriptReserved_IsFlippedDiagonalFloorSplit, &PointCollisionData::IsFlippedDiagonalFloorSplit,
            ScriptReserved_IsFlippedDiagonalCeilingSplit, &PointCollisionData::IsFlippedDiagonalCeilingSplit,
            ScriptReserved_TestEnvironmentFlag, &PointCollisionData::TestEnvironmentFlag
        );

        // Register functions
        lua.set_function(ScriptReserved_GetPointCollision, sol::overload(
            static_cast<PointCollisionData(*)(const Vector3i&, int)>(&GetPointCollision),
            static_cast<PointCollisionData(*)(const Vector3i&, int, const Vector3&, float)>(&GetPointCollision),
            static_cast<PointCollisionData(*)(const Vector3i&, int, short, float, float, float, const Vector3&)>(&GetPointCollision),
            static_cast<PointCollisionData(*)(const ItemInfo&)>(&GetPointCollision),
            static_cast<PointCollisionData(*)(const ItemInfo&, const Vector3&, float)>(&GetPointCollision),
            static_cast<PointCollisionData(*)(const ItemInfo&, short, float, float, float, const Vector3&)>(&GetPointCollision)
        ));
    }
}
