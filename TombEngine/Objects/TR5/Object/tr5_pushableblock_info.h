#pragma once

struct PushableInfo
{
	int height;				// height for collision, also in floor procedure
	int weight;
	int stackLimit;
	int moveX;				// used for pushable movement code
	int moveZ;				// used for pushable movement code
	short linkedIndex;		// using itemFlags[1] for now
	short gravity;			// fall acceleration
	short loopSound;		// looped sound index for movement
	short stopSound;		// ending sound index
	short fallSound;		// sound on hitting floor (if dropped)
	short climb;			// not used for now
	bool canFall;			// OCB 32
	bool hasFloorCeiling;			// has floor and ceiling procedures (OCB 64)
	bool disablePull;		// OCB 128
	bool disablePush;		// OCB 256
	bool disableW;			// OCB 512 (W+E)
	bool disableE;			// OCB 512 (W+E)
	bool disableN;			// OCB 1024 (N+S)
	bool disableS;			// OCB 1024 (N+S)
};
