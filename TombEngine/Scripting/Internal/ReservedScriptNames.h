#pragma once

// Tables
static constexpr char ScriptReserved_TEN[]				= "TEN";
static constexpr char ScriptReserved_Flow[]				= "Flow";
static constexpr char ScriptReserved_Logic[]			= "Logic";
static constexpr char ScriptReserved_Objects[]			= "Objects";
static constexpr char ScriptReserved_Strings[]			= "Strings";
static constexpr char ScriptReserved_Inventory[]		= "Inventory";
static constexpr char ScriptReserved_Misc[]				= "Misc";
static constexpr char ScriptReserved_Effects[]			= "Effects";

// Classes
static constexpr char ScriptReserved_Moveable[]			= "Moveable";
static constexpr char ScriptReserved_Lara[]				= "Lara";
static constexpr char ScriptReserved_Static[]			= "Static";
static constexpr char ScriptReserved_Camera[]			= "Camera";
static constexpr char ScriptReserved_Sink[]				= "Sink";
static constexpr char ScriptReserved_SoundSource[]		= "SoundSource";
static constexpr char ScriptReserved_AIObject[]			= "AIObject";
static constexpr char ScriptReserved_DisplayString[]	= "DisplayString";
static constexpr char ScriptReserved_Vec3[]				= "Vec3";
static constexpr char ScriptReserved_Rotation[]			= "Rotation";
static constexpr char ScriptReserved_LevelFunc[]		= "LevelFunc";

// Member functions
static constexpr char ScriptReserved_New[]					= "New";
static constexpr char ScriptReserved_Init[]					= "Init";
static constexpr char ScriptReserved_Enable[]				= "Enable";
static constexpr char ScriptReserved_Disable[]				= "Disable";
static constexpr char ScriptReserved_MakeInvisible[]		= "MakeInvisible";
static constexpr char ScriptReserved_Explode[]				= "Explode";
static constexpr char ScriptReserved_Shatter[]				= "Shatter";
static constexpr char ScriptReserved_SetPoison[]			= "SetPoison";
static constexpr char ScriptReserved_GetPoison[]			= "GetPoison";
static constexpr char ScriptReserved_SetAir[]				= "SetAir";
static constexpr char ScriptReserved_GetAir[]				= "GetAir";
static constexpr char ScriptReserved_GetColor[]				= "GetColor";
static constexpr char ScriptReserved_SetColor[]				= "SetColor";
static constexpr char ScriptReserved_GetPosition[]			= "GetPosition";
static constexpr char ScriptReserved_GetJointPosition[]		= "GetJointPosition";
static constexpr char ScriptReserved_SetPosition[]			= "SetPosition";
static constexpr char ScriptReserved_GetRotation[]			= "GetRotation";
static constexpr char ScriptReserved_SetRotation[]			= "SetRotation";
static constexpr char ScriptReserved_GetRotationY[]			= "GetRotationY";
static constexpr char ScriptReserved_SetRotationY[]			= "SetRotationY";
static constexpr char ScriptReserved_GetScale[]				= "GetScale";
static constexpr char ScriptReserved_SetScale[]				= "SetScale";
static constexpr char ScriptReserved_GetSolid[]				= "GetSolid";
static constexpr char ScriptReserved_SetSolid[]				= "SetSolid";
static constexpr char ScriptReserved_GetName[]				= "GetName";
static constexpr char ScriptReserved_SetName[]				= "SetName";
static constexpr char ScriptReserved_GetSlot[]				= "GetSlot";
static constexpr char ScriptReserved_SetSlot[]				= "SetSlot";
static constexpr char ScriptReserved_GetObjectID[]			= "GetObjectID";
static constexpr char ScriptReserved_SetObjectID[]			= "SetObjectID";
static constexpr char ScriptReserved_GetSoundID[]			= "GetSoundID";
static constexpr char ScriptReserved_SetSoundID[]			= "SetSoundID";
static constexpr char ScriptReserved_GetHP[]				= "GetHP";
static constexpr char ScriptReserved_SetHP[]				= "SetHP";
static constexpr char ScriptReserved_GetSlotHP[]			= "GetSlotHP";
static constexpr char ScriptReserved_GetVelocity[]			= "GetVelocity";
static constexpr char ScriptReserved_SetVelocity[]			= "SetVelocity";
static constexpr char ScriptReserved_GetFrameNumber[]		= "GetFrame";
static constexpr char ScriptReserved_SetFrameNumber[]		= "SetFrame";
static constexpr char ScriptReserved_GetAnimNumber[]		= "GetAnim";
static constexpr char ScriptReserved_SetAnimNumber[]		= "SetAnim";
static constexpr char ScriptReserved_GetStateNumber[]		= "GetState";
static constexpr char ScriptReserved_SetStateNumber[]		= "SetState";
static constexpr char ScriptReserved_GetOCB[]				= "GetOCB";
static constexpr char ScriptReserved_SetOCB[]				= "SetOCB";
static constexpr char ScriptReserved_GetStatus[]			= "GetStatus";
static constexpr char ScriptReserved_GetAIBits[]			= "GetAIBits";
static constexpr char ScriptReserved_SetAIBits[]			= "SetAIBits";
static constexpr char ScriptReserved_GetItemFlags[]			= "GetItemFlags";
static constexpr char ScriptReserved_SetItemFlags[]			= "SetItemFlags";
static constexpr char ScriptReserved_MeshIsVisible[]		= "MeshIsVisible";
static constexpr char ScriptReserved_ShowMesh[]				= "ShowMesh";
static constexpr char ScriptReserved_HideMesh[]				= "HideMesh";
static constexpr char ScriptReserved_ShatterMesh[]			= "ShatterMesh";
static constexpr char ScriptReserved_MeshIsSwapped[]		= "MeshIsSwapped";
static constexpr char ScriptReserved_SwapMesh[]				= "SwapMesh";
static constexpr char ScriptReserved_UnswapMesh[]			= "UnswapMesh";
static constexpr char ScriptReserved_GetHitStatus[]			= "GetHitStatus";
static constexpr char ScriptReserved_GetActive[]			= "GetActive";
static constexpr char ScriptReserved_GetRoom[]				= "GetRoom";
static constexpr char ScriptReserved_SetRoom[]				= "SetRoom";
static constexpr char ScriptReserved_GetStrength[]			= "GetStrength";
static constexpr char ScriptReserved_SetStrength[]			= "SetStrength";
static constexpr char ScriptReserved_GetValid[]				= "GetValid";
static constexpr char ScriptReserved_SetOnFire[]			= "SetOnFire";
static constexpr char ScriptReserved_GetOnFire[]			= "GetOnFire";
static constexpr char ScriptReserved_Destroy[]				= "Destroy";
static constexpr char ScriptReserved_GetKey[]				= "GetKey";
static constexpr char ScriptReserved_SetKey[]				= "SetKey";
static constexpr char ScriptReserved_GetOnKilled[]			= "GetOnKilled";
static constexpr char ScriptReserved_SetOnKilled[]			= "SetOnKilled";
static constexpr char ScriptReserved_GetOnHit[]				= "GetOnHit";
static constexpr char ScriptReserved_SetOnHit[]				= "SetOnHit";
static constexpr char ScriptReserved_GetOnCollidedWithObject[]	= "GetOnCollidedWithObject";
static constexpr char ScriptReserved_SetOnCollidedWithObject[]	= "SetOnCollidedWithObject";
static constexpr char ScriptReserved_GetOnCollidedWithRoom[]	= "GetOnCollidedWithRoom";
static constexpr char ScriptReserved_SetOnCollidedWithRoom[]	= "SetOnCollidedWithRoom";
static constexpr char ScriptReserved_ToLength[]					= "ToLength";

// Flow Functions
static constexpr char ScriptReserved_AddLevel[]					= "AddLevel";
static constexpr char ScriptReserved_GetLevel[]					= "GetLevel";
static constexpr char ScriptReserved_GetCurrentLevel[]			= "GetCurrentLevel";
static constexpr char ScriptReserved_SetIntroImagePath[]		= "SetIntroImagePath";
static constexpr char ScriptReserved_SetTitleScreenImagePath[]	= "SetTitleScreenImagePath";
static constexpr char ScriptReserved_SetFarView[]				= "SetFarView";
static constexpr char ScriptReserved_SetSettings[]				= "SetSettings";
static constexpr char ScriptReserved_SetAnimations[]			= "SetAnimations";
static constexpr char ScriptReserved_EndLevel[]					= "EndLevel";
static constexpr char ScriptReserved_GetSecretCount[]			= "GetSecretCount";
static constexpr char ScriptReserved_SetSecretCount[]			= "SetSecretCount";
static constexpr char ScriptReserved_SetTotalSecretCount[]		= "SetTotalSecretCount";
static constexpr char ScriptReserved_AddSecret[]				= "AddSecret";
static constexpr char ScriptReserved_EnableFlyCheat[]			= "EnableFlyCheat";

// Flow Functions
static constexpr char ScriptReserved_SetStrings[]			= "SetStrings";
static constexpr char ScriptReserved_GetString[]			= "GetString";
static constexpr char ScriptReserved_SetLanguageNames[]		= "SetLanguageNames";

// Flow Tables
static constexpr char ScriptReserved_WeatherType[]		= "WeatherType";
static constexpr char ScriptReserved_LaraType[]			= "LaraType";
static constexpr char ScriptReserved_RotationAxis[]		= "RotationAxis";
static constexpr char ScriptReserved_ItemAction[]		= "ItemAction";
static constexpr char ScriptReserved_ErrorMode[]		= "ErrorMode";
static constexpr char ScriptReserved_InventoryItem[]	= "InventoryItem";

// Functions
static constexpr char ScriptReserved_ShowString[]					= "ShowString";
static constexpr char ScriptReserved_HideString[]					= "HideString";
static constexpr char ScriptReserved_SetAmbientTrack[]				= "SetAmbientTrack";
static constexpr char ScriptReserved_PlayAudioTrack[]				= "PlayAudioTrack";
static constexpr char ScriptReserved_StopAudioTrack[]				= "StopAudioTrack";
static constexpr char ScriptReserved_StopAudioTracks[]				= "StopAudioTracks";
static constexpr char ScriptReserved_PlaySound[]					= "PlaySound";
static constexpr char ScriptReserved_GiveInvItem[]					= "GiveItem";
static constexpr char ScriptReserved_TakeInvItem[]					= "TakeItem";
static constexpr char ScriptReserved_GetInvItemCount[]				= "GetItemCount";
static constexpr char ScriptReserved_SetInvItemCount[]				= "SetItemCount";
static constexpr char ScriptReserved_GetMoveableByName[]			= "GetMoveableByName";
static constexpr char ScriptReserved_GetStaticByName[]				= "GetStaticByName";
static constexpr char ScriptReserved_GetMoveablesBySlot[]			= "GetMoveablesBySlot";
static constexpr char ScriptReserved_GetStaticsBySlot[]				= "GetStaticsBySlot";
static constexpr char ScriptReserved_GetCameraByName[]				= "GetCameraByName";
static constexpr char ScriptReserved_GetSinkByName[]				= "GetSinkByName";
static constexpr char ScriptReserved_GetAIObjectByName[]			= "GetAIObjectByName";
static constexpr char ScriptReserved_GetSoundSourceByName[]			= "GetSoundSourceByName";
static constexpr char ScriptReserved_CalculateDistance[]			= "CalculateDistance";
static constexpr char ScriptReserved_CalculateHorizontalDistance[]	= "CalculateHorizontalDistance";
static constexpr char ScriptReserved_ScreenToPercent[]				= "ScreenToPercent";
static constexpr char ScriptReserved_PercentToScreen[]				= "PercentToScreen";
static constexpr char ScriptReserved_HasLineOfSight[]				= "HasLineOfSight";

static constexpr char ScriptReserved_AddCallback[]					= "AddCallback";
static constexpr char ScriptReserved_RemoveCallback[]				= "RemoveCallback";

static constexpr char ScriptReserved_EmitParticle[]					= "EmitParticle";
static constexpr char ScriptReserved_EmitLightningArc[]				= "EmitLightningArc";
static constexpr char ScriptReserved_EmitShockwave[]				= "EmitShockwave";
static constexpr char ScriptReserved_EmitLight[]					= "EmitLight";
static constexpr char ScriptReserved_EmitBlood[]					= "EmitBlood";
static constexpr char ScriptReserved_EmitFire[]						= "EmitFire";
static constexpr char ScriptReserved_MakeExplosion[]				= "MakeExplosion";
static constexpr char ScriptReserved_MakeEarthquake[]				= "MakeEarthquake";
static constexpr char ScriptReserved_Vibrate[]						= "Vibrate";
static constexpr char ScriptReserved_FlashScreen[]					= "FlashScreen";
static constexpr char ScriptReserved_FadeIn[]						= "FadeIn";
static constexpr char ScriptReserved_FadeOut[]						= "FadeOut";
static constexpr char ScriptReserved_FadeOutComplete[]				= "FadeOutComplete";
static constexpr char ScriptReserved_SetCineBars[]					= "SetCineBars";
static constexpr char ScriptReserved_SetFOV[]						= "SetFOV";
static constexpr char ScriptReserved_GetFOV[]						= "GetFOV";

static constexpr char ScriptReserved_KeyIsHeld[]					= "KeyIsHeld";
static constexpr char ScriptReserved_KeyIsHit[]						= "KeyIsHit";
static constexpr char ScriptReserved_KeyPush[]						= "KeyPush";
static constexpr char ScriptReserved_KeyClear[]						= "KeyClear";

static constexpr char ScriptReserved_FlipMap[]						= "FlipMap";
static constexpr char ScriptReserved_PlayFlyBy[]					= "PlayFlyBy";

// Enums
static constexpr char ScriptReserved_ObjID[]					= "ObjID";
static constexpr char ScriptReserved_BlendID[]					= "BlendID";
static constexpr char ScriptReserved_DisplayStringOption[]		= "DisplayStringOption";
static constexpr char ScriptReserved_CallbackPoint[]			= "CallbackPoint";

static constexpr char ScriptReserved_LevelVars[]	= "LevelVars";
static constexpr char ScriptReserved_GameVars[]		= "GameVars";
static constexpr char ScriptReserved_LevelFuncs[]	= "LevelFuncs";
static constexpr char ScriptReserved_Engine[]		= "Engine";

// Internal
static constexpr char ScriptReserved_LaraObject[]				= "LaraObject";
