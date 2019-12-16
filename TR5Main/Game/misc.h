#pragma once
#include "../Global/global.h"

#define cutseq_givelara_pistols ((void(__cdecl*)()) 0x00422680)
#define cutseq_removelara_pistols ((void(__cdecl*)()) 0x004226B0)
#define cutseq_givelara_hk ((void(__cdecl*)()) 0x004226E0)
#define cutseq_removelara_hk ((void(__cdecl*)()) 0x00422700)

enum LARA_MESH_MASK
{
	LARA_ONLY_LEGS = (0 << 1),
	LARA_ONLY_ARMS = (1 << 1),
	LARA_ONLY_TORSO = (2 << 1),
	LARA_ONLY_HEAD = (4 << 1),
	LARA_ONLY_LEFT_ARM = (8 << 1),
	LARA_ONLY_RIGHT_ARM = (16 << 1),
	LARA_LEGS_TORSO_HEAD = (32 << 1),
	LARA_LEGS_TORSO_HEAD_ARMS = (64 << 1)
};

short GF(short animIndex, short frameToStart); // for lara
short GF2(short objectID, short animIndex, short frameToStart); // for entity

int getLaraMask(UINT16 meshMaskFlag);