#pragma once

enum class ModificationType;
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingConsumable(LaraInfo&, GAME_OBJECT_ID objectID, std::optional<int> amount = 0);
bool TryRemovingConsumable(LaraInfo&, GAME_OBJECT_ID objectID, std::optional<int> amount = 0);
bool TryModifyingConsumable(LaraInfo& lara, GAME_OBJECT_ID objectID, std::optional<int> amount, ModificationType modType);
std::optional<int> GetConsumableCount(LaraInfo&, GAME_OBJECT_ID objectID);
