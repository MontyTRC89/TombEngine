#include "framework.h"
#include "tr5_smoke_emitter.h"
#include "Objects/objectslist.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/trmath.h"
#include "Renderer/Renderer11Enums.h"

void InitialiseSmokeEmitter(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags == 111)
	{
		if (item->Pose.Orientation.y > 0)
		{
			if (item->Pose.Orientation.y == Angle::DegToRad(90.0f))
				item->Pose.Position.x += CLICK(2);
		}
		else if (item->Pose.Orientation.y)
		{
			if (item->Pose.Orientation.y == Angle::DegToRad(-180.0f))
				item->Pose.Position.z -= CLICK(2);
			else if (item->Pose.Orientation.y == Angle::DegToRad(-90.0f))
				item->Pose.Position.x -= CLICK(2);
		}
		else
			item->Pose.Position.z += CLICK(2);
	}
	else if (item->ObjectNumber != ID_SMOKE_EMITTER)
		return;
	else if (item->TriggerFlags & 8)
	{
		item->ItemFlags[0] = item->TriggerFlags / 16;

		if (item->Pose.Orientation.y > 0)
		{
			if (item->Pose.Orientation.y == Angle::DegToRad(90.0f))
				item->Pose.Position.x += CLICK(1);
		}
		else
		{
			if (item->Pose.Orientation.y == 0)
				item->Pose.Position.z += CLICK(1);
			else if (item->Pose.Orientation.y == Angle::DegToRad(-180.0f))
				item->Pose.Position.z -= CLICK(1);
			else if (item->Pose.Orientation.y == Angle::DegToRad(-90.0f))
				item->Pose.Position.x -= CLICK(1);
		}

		if ((signed short)(item->TriggerFlags / 16) <= 0)
		{
			item->ItemFlags[2] = 4096;
			item->TriggerFlags |= 4;
		}
	}
	else if (g_Level.Rooms[item->RoomNumber].flags & 1 && item->TriggerFlags == 1)
	{
		item->ItemFlags[0] = 20;
		item->ItemFlags[1] = 1;
	}
}

void SmokeEmitterControl(short itemNumber)
{
	/*auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	if (item->objectNumber != ID_SMOKE_EMITTER)
		goto LABEL_61;

	if (g_Level.Rooms[item->roomNumber].flags & 1)
	{
		if (item->itemFlags[0] || !(GetRandomControl() & 0x1F) || item->triggerFlags == 1)
		{
			if (!(GetRandomControl() & 3) || item->itemFlags[1])
			{
				Vector3Int pos;
				pos.x = (GetRandomControl() & 0x3F) + item->pos.Position.x - 32;
				pos.y = item->pos.Position.y - (GetRandomControl() & 0x1F) - 16;
				pos.z = (GetRandomControl() & 0x3F) + item->pos.Position.z - 32;

				if (item->triggerFlags == 1)
				{
					CreateBubble(&pos, item->roomNumber, 15, 15, 0, 0, 0, 0);
				}
				else
				{
					CreateBubble(&pos, item->roomNumber, 8, 7, 0, 0, 0, 0);
				}

				if (item->itemFlags[0])
				{
					item->itemFlags[0]--;
					if (!item->itemFlags[0])
						item->itemFlags[1] = 0;
				}
			}
		}
		else
		{
			if (!(GetRandomControl() & 0x1F))
			{
				item->itemFlags[0] = (GetRandomControl() & 3) + 4;
			}
		}

		return;
	}

	if (!(item->triggerFlags & 8) || item->triggerFlags == 111)
		goto LABEL_61;

	if (item->triggerFlags & 4)
	{
		if (GlobalCounter & 1)
			return;
	}
	else
	{
		if (item->itemFlags[0])
		{
			item->itemFlags[0]--;
			if (!item->itemFlags[0])
				item->itemFlags[1] = (GetRandomControl() & 0x3F) + 30;

			v41 = 1;
			if (item->itemFlags[2])
				item->itemFlags[2] -= 256;
			if (!item->itemFlags[2])
				goto LABEL_61;
		}
		else
		{
			if (item->itemFlags[2] < 4096)
				item->itemFlags[2] += 256;
		}
	}

	int dx = LaraItem->pos.Position.x - item->pos.Position.x;
	int dz = LaraItem->pos.Position.z - item->pos.Position.z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		auto* spark = GetFreeParticle();

		spark->on = 1;
		spark->dR = 48;
		spark->dG = 48;
		spark->dB = 48;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->x = (GetRandomControl() & 0x3F) + item->pos.Position.x - 32;
		spark->y = (GetRandomControl() & 0x3F) + item->pos.Position.y - 32;
		spark->z = (GetRandomControl() & 0x3F) + item->pos.Position.z - 32;

		int flags = item->itemFlags[2];
		if (flags == 4096)
		{
			if (item->triggerFlags & 4)
				flags = (GetRandomControl() & 0xFFF) + 256;
			else
				flags = (GetRandomControl() & 0x7FF) + 2048;
		}

		if (item->triggerFlags >= 0)
		{
			spark->xVel = flags * sin(item->pos.Orientation.y - Angle::DegToRad(180)) >> W2V_SHIFT;
			spark->yVel = -16 - (GetRandomControl() & 0xF);
			spark->zVel = flags * cos(item->pos.Orientation.y - Angle::DegToRad(180)) >> W2V_SHIFT;
		}
		else
		{
			v17 = GetRandomControl();
			v18 = v17;
			LOWORD(v17) = item->pos.Orientation.y;
			f = (v18 & 0x7F) + 2048;
			spark->Xvel = v15 * 4 * rcossin_tbl[((v17 + 20480) >> 3) & 0x1FFE] >> 14;
			spark->Yvel = -128 - (unsigned __int8)GetRandomControl();
			spark->Zvel = v15 * 4 * rcossin_tbl[((((unsigned __int16)item->pos.Orientation.y + 20480) >> 3) & 0x1FFE) + 1] >> 14;
		}
		spark->Flags = 538;
		if (!(GlobalCounter & 3) && !(item->triggerFlags & 4))
			spark->Flags = 1562;
		spark->RotAng = GetRandomControl() & 0xFFF;
		spark->RotAdd = GetRandomControl() & 1 ? -8 - (GetRandomControl() & 7) : (GetRandomControl() & 7) + 8;
		spark->Gravity = -8 - (GetRandomControl() & 0xF);
		spark->MaxYvel = -8 - (GetRandomControl() & 7);
		v20 = (GetRandomControl() & 0x1F) + 128;
		if (item->triggerFlags & 4)
		{
			spark->sB = 0;
			spark->sG = 0;
			spark->sR = 0;
			spark->ColFadeSpeed = 2;
			spark->FadeToBlack = 2;
			v21 = (GetRandomControl() & 3) + 16;
			spark->Life = v21;
			spark->sLife = v21;
			if (item->triggerFlags >= 0)
			{
				spark->Xvel *= 4;
				spark->Zvel *= 4;
				spark->Scalar = 3;
				spark->Friction = 4;
			}
			else
			{
				spark->Scalar = 1;
				spark->Friction = 51;
				v15 >>= 1;
			}
			v22 = v15 * v20 >> 10;
			if (v22 > 255)
				v22 = 255;
			spark->dSize = v22;
			spark->sSize = v22 >> 2;
			spark->Size = v22 >> 2;
		}
		else
		{
			spark->sR = 96;
			spark->sG = 96;
			spark->sB = 96;
			spark->fadeToBlack = 6;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 6;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 8;
			spark->friction = 4 - (item->triggerFlags & 4);
			v25 = (((item->triggerFlags & 0xFF) >> 2) & 1) + 2;
			spark->dSize = v20;
			spark->scalar = (((item->triggerFlags & 0xFF) >> 2) & 1) + 2;
			spark->sSize = v20 >> 1;
			spark->Size = v20 >> 1;
			v26 = item->itemFlags[1];
			if (v26)
				item->itemFlags[1] = v26 - 1;
			else
				item->itemFlags[0] = item->triggerFlags >> 4;
		}
		LOBYTE(v4) = v41;
		if (v41)
		{
		LABEL_61:
			LOBYTE(v4) = wibble;
			if (!(wibble & 0xF) && (item->objectNumber != 365 || !(wibble & 0x1F)))
			{
				auto* spark = GetFreeParticle();

				spark->on = 1;
				spark->sR = 0;
				spark->sG = 0;
				spark->sB = 0;
				spark->dR = 64;
				spark->dG = 64;
				spark->dB = 64;
				spark->fadeToBlack = 16;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
				spark->life = spark->sLife = (GetRandomControl() & 7) + 28;
				if (item->objectNumber == ID_SMOKE_EMITTER_WHITE)
					spark->blendMode = BLEND_MODES::BLENDMODE_SUBTRACTIVE;
				else
					spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
				spark->x = (GetRandomControl() & 0x3F) + item->pos.Position.x - 32;
				spark->y = (GetRandomControl() & 0x3F) + item->pos.Position.y - 32;
				spark->z = (GetRandomControl() & 0x3F) + item->pos.Position.z - 32;
				if (item->triggerFlags == 111)
				{
					spark->xVel = 512 * sin(item->pos.Orientation.y - Angle::DegToRad(180)) >> W2V_SHIFT;
					spark->zVel = 512 * cos(item->pos.Orientation.y - Angle::DegToRad(180)) >> W2V_SHIFT;
					spark->friction = 5;
				}
				else
				{
					spark->xVel = (byte)GetRandomControl() - 128;
					spark->zVel = (byte)GetRandomControl() - 128;
					spark->friction = 3;
				}
				v33 = GetRandomControl();
				spark->Flags = 538;
				spark->Yvel = -16 - (v33 & 0xF);
				if (room[item->roomNumber].flags & 8)
					spark->Flags = 794;
				spark->RotAng = GetRandomControl() & 0xFFF;
				if (GetRandomControl() & 1)
					spark->RotAdd = -8 - (GetRandomControl() & 7);
				else
					spark->RotAdd = (GetRandomControl() & 7) + 8;
				spark->Scalar = 2;
				spark->Gravity = -8 - (GetRandomControl() & 0xF);
				spark->MaxYvel = -8 - (GetRandomControl() & 7);
				v4 = (GetRandomControl() & 0x1F) + 128;
				spark->dSize = v4;
				spark->sSize = v4 >> 2;
				spark->Size = v4 >> 2;
				if (item->objectNumber == 365)
				{
					v34 = spark->MaxYvel;
					v35 = spark->Life;
					v36 = spark->sLife + 16;
					spark->Gravity >>= 1;
					spark->Yvel >>= 1;
					spark->MaxYvel = v34 >> 1;
					spark->Life = v35 + 16;
					LOBYTE(v4) = 32;
					spark->sLife = v36;
					spark->dR = 32;
					spark->dG = 32;
					spark->dB = 32;
				}
			}
			return v4;
		}
	}
	return v4;*/
}
