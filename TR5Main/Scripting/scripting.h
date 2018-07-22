#pragma once

#include "..\Global\global.h"

typedef enum ItemFlags
{
	SIF_ACTIVE = 0,
	SIF_STATUS = 1,
	SIF_gravityStatus = 2,
	SIF_hitStatus = 3,
	SIF_COLLIDABLE = 4,
	SIF_LOOKED_AT = 5,
	SIF_DYNAMIC_LIGHT = 6,
	SIF_POISONED = 7,
	SIF_REALLY_ACTIVE = 8
};

__int16 GetItemObjectNumber(__int16 itemNumber);
__int16 GetItemCurrentAnimState(__int16 itemNumber);
void SetItemCurrentAnimState(__int16 itemNumber, __int16 animState);
__int16 GetItemGoalAnimState(__int16 itemNumber);
void SetItemGoalAnimState(__int16 itemNumber, __int16 animState);
__int16 GetItemRequiredAnimState(__int16 itemNumber);
void SetItemRequiredAnimState(__int16 itemNumber, __int16 animState);
__int16 GetItemAnimNumber(__int16 itemNumber);
void SetItemAnimNumber(__int16 itemNumber, __int16 animState);
__int16 GetItemAnimFrame(__int16 itemNumber);
void SetItemAnimFrame(__int16 itemNumber, __int16 animState);
__int16 GetItemRoomNumber(__int16 itemNumber);
void SetItemRoomNumber(__int16 itemNumber, __int16 animState);
__int16 GetItemSpeed(__int16 itemNumber);
void SetItemSpeed(__int16 itemNumber, __int16 animState);
__int16 GetItemFallSpeed(__int16 itemNumber);
void SetItemFallSpeed(__int16 itemNumber, __int16 animState);
__int16 GetItemHitPoints(__int16 itemNumber);
void SetItemHitPoints(__int16 itemNumber, __int16 animState);
__int16 GetItemFlag(__int16 itemNumber, __int16 flag);
void SetItemFlag(__int16 itemNumber, __int16 flag, bool value);
__int16 GetItemAiBits(__int16 itemNumber);
void SetItemAiBits(__int16 itemNumber, __int16 aiBits);
__int32 GetItemPositionX(__int16 itemNumber);
void SetItemPositionX(__int16 itemNumber, __int16 x);
__int32 GetItemPositionY(__int16 itemNumber);
void SetItemPositionY(__int16 itemNumber, __int16 y);
__int32 GetItemPositionZ(__int16 itemNumber);
void SetItemPositionZ(__int16 itemNumber, __int16 z);
__int32 GetItemRotationX(__int16 itemNumber);
void SetItemRotationX(__int16 itemNumber, __int16 x);
__int32 GetItemRotationY(__int16 itemNumber);
void SetItemRotationY(__int16 itemNumber, __int16 y);
__int32 GetItemRotationZ(__int16 itemNumber);
void SetItemRotationZ(__int16 itemNumber, __int16 z);