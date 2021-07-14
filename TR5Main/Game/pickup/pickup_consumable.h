#pragma once
enum GAME_OBJECT_ID : short;
struct LaraInfo;
bool TryAddConsumable(LaraInfo&, GAME_OBJECT_ID id, int amt = 0);
bool TryRemoveConsumable(LaraInfo&, GAME_OBJECT_ID id, int amt = 0);
std::optional<int> GetConsumableCount(LaraInfo&, GAME_OBJECT_ID id);
