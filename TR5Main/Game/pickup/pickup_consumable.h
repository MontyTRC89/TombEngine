#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingConsumable(LaraInfo&, GAME_OBJECT_ID objectID, int amount = 0);
bool TryRemovingConsumable(LaraInfo&, GAME_OBJECT_ID objectID, int amount = 0);
std::optional<int> GetConsumableCount(LaraInfo&, GAME_OBJECT_ID objectID);
