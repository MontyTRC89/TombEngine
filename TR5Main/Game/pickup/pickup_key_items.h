#pragma once
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddKeyItem(LaraInfo& lara, GAME_OBJECT_ID obj, int count);
bool TryRemoveKeyItem(LaraInfo& lara, GAME_OBJECT_ID obj, int count);
std::optional<int> GetKeyItemCount(LaraInfo& lara, GAME_OBJECT_ID obj);

