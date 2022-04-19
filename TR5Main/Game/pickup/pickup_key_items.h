#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingKeyItem(LaraInfo& lara, GAME_OBJECT_ID objectID, int count);
bool TryRemovingKeyItem(LaraInfo& lara, GAME_OBJECT_ID objectID, int count);
std::optional<int> GetKeyItemCount(LaraInfo& lara, GAME_OBJECT_ID objectID);
