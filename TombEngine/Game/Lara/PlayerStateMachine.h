#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	enum class PlayerStateRoutineType
	{
		Control,
		Collision
	};

	void InitializePlayerStateMachine();
	void HandlePlayerState(ItemInfo& item, CollisionInfo& coll, PlayerStateRoutineType routineType);
}
