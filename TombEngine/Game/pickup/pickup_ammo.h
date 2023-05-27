#pragma once

enum class ModificationType;
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingAmmo(LaraInfo&, GAME_OBJECT_ID objectID, std::optional<int> amount = std::nullopt);
bool TryRemovingAmmo(LaraInfo&, GAME_OBJECT_ID objectID, std::optional<int> amount = std::nullopt);
bool TryModifyingAmmo(LaraInfo& lara, GAME_OBJECT_ID objectID, std::optional<int> amount, ModificationType modType);
std::optional<int> GetAmmoCount(LaraInfo&, GAME_OBJECT_ID objectID);
