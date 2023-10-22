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

	void InitializePlayerStateRoutines();
	void HandlePlayerState(ItemInfo& item, CollisionInfo& coll, PlayerStateRoutineType routineType);
}
