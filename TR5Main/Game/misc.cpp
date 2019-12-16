#include "misc.h"

#define CHK_ANY(var, flag) (var & flag) != 0
#define CHK_NOP(var, flag) !(var & flag)

short GF(short animIndex, short frameToStart)
{
	return short(Anims[animIndex].frameBase + frameToStart);
}

short GF2(short objectID, short animIndex, short frameToStart)
{
	return short(Anims[Objects[objectID].animIndex + animIndex].frameBase + frameToStart);
}

int getLaraMask(UINT16 meshMaskFlag)
{
	if (CHK_ANY(meshMaskFlag, LARA_ONLY_LEGS))
		return (MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT));
	else if (CHK_ANY(meshMaskFlag, LARA_ONLY_ARMS))
		return (MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND) | MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND));
	else if (CHK_ANY(meshMaskFlag, LARA_ONLY_HEAD))
		return MESH_BITS(LM_HEAD);
	else if (CHK_ANY(meshMaskFlag, LARA_ONLY_TORSO))
		return MESH_BITS(LM_TORSO);
	else if (CHK_ANY(meshMaskFlag, LARA_ONLY_LEFT_ARM))
		return (MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND));
	else if (CHK_ANY(meshMaskFlag, LARA_ONLY_RIGHT_ARM))
		return (MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND));
	else if (CHK_ANY(meshMaskFlag, LARA_LEGS_TORSO_HEAD))
		return (MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT) | MESH_BITS(LM_HEAD) | MESH_BITS(LM_TORSO));
	else if (CHK_ANY(meshMaskFlag, LARA_LEGS_TORSO_HEAD_ARMS))
		return (MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT) | MESH_BITS(LM_HEAD) | MESH_BITS(LM_TORSO) | MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND) | MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND));
}