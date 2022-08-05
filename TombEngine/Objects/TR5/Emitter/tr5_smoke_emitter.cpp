#include "framework.h"
#include "tr5_smoke_emitter.h"

#include "Game/camera.h"
#include "Game/items.h"
#include "Game/effects/bubble.h"
#include "Game/effects/effects.h"
#include "Game/control/control.h"
#include "Game/control/trigger.h"
#include "Game/collision/collide_room.h"
#include "Specific/level.h"
#include "Specific/trmath.h"
#include "Objects/objectslist.h"
#include "Renderer/Renderer11Enums.h"

void InitialiseSmokeEmitter(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags == 111)
	{
		if (item->Pose.Orientation.y > 0)
		{
			if (item->Pose.Orientation.y == ANGLE(90.0f))
				item->Pose.Position.x += CLICK(2);
		}
		else if (item->Pose.Orientation.y)
		{
			if (item->Pose.Orientation.y == -ANGLE(180.0f))
				item->Pose.Position.z -= CLICK(2);
			else if (item->Pose.Orientation.y == -ANGLE(90.0f))
				item->Pose.Position.x -= CLICK(2);
		}
		else
			item->Pose.Position.z += CLICK(2);
	}
	else if (item->ObjectNumber != ID_STEAM_EMITTER)
		return;
	else if (item->TriggerFlags & 8)
	{
		item->ItemFlags[0] = item->TriggerFlags / 16;

		if (item->Pose.Orientation.y > 0)
		{
			if (item->Pose.Orientation.y == ANGLE(90.0f))
				item->Pose.Position.x += CLICK(1);
		}
		else
		{
			if (item->Pose.Orientation.y == 0)
				item->Pose.Position.z += CLICK(1);
			else if (item->Pose.Orientation.y == -ANGLE(180.0f))
				item->Pose.Position.z -= CLICK(1);
			else if (item->Pose.Orientation.y == -ANGLE(90.0f))
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
	Vector3Int pos = {};

	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	if (TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item->RoomNumber))
	{
		if (item->ItemFlags[0] || !(GetRandomControl() & 0x1F) || item->TriggerFlags == 1)
		{
			if (!(GetRandomControl() & 3) || item->ItemFlags[1])
			{
				pos.x = (GetRandomControl() & 0x3F) + item->Pose.Position.x - 32;
				pos.y = item->Pose.Position.y - (GetRandomControl() & 0x1F) - 16;
				pos.z = (GetRandomControl() & 0x3F) + item->Pose.Position.z - 32;

				if (item->TriggerFlags == 1)
					CreateBubble(&pos, item->RoomNumber, 15, 15, 0, 0, 0, 0);
				else
					CreateBubble(&pos, item->RoomNumber, 8, 7, 0, 0, 0, 0);

				if (item->ItemFlags[0])
				{
					item->ItemFlags[0]--;

					if (!item->ItemFlags[0])
						item->ItemFlags[1] = 0;
				}
			}
		}
		else if (!(GetRandomControl() & 0x1F))
			item->ItemFlags[0] = (GetRandomControl() & 3) + 4;

		return;
	}

	if (item->ObjectNumber == ID_STEAM_EMITTER && item->TriggerFlags & 8)
	{
		bool normal = false;

		if (item->ItemFlags[0])
		{
			item->ItemFlags[0]--;

			if (!item->ItemFlags[0])
				item->ItemFlags[1] = (GetRandomControl() & 0x3F) + 30;

			normal = true;

			if (item->ItemFlags[2])
				item->ItemFlags[2] -= 256;
		}
		else if (item->ItemFlags[2] < 4096)
			item->ItemFlags[2] += 256;

		if (item->ItemFlags[2])
		{
			int dx = Camera.pos.x - item->Pose.Position.x;
			int dz = Camera.pos.z - item->Pose.Position.z;

			if (dx < -SECTOR(16) || dx > SECTOR(16) || dz < -SECTOR(16) || dz > SECTOR(16))
				return;

			auto* sptr = GetFreeParticle();
			sptr->on = true;
			sptr->sR = 96;
			sptr->sG = 96;
			sptr->sB = 96;
			sptr->dR = 48;
			sptr->dG = 48;
			sptr->dB = 48;
			sptr->fadeToBlack = 6;
			sptr->colFadeSpeed = (GetRandomControl() & 3) + 6;
			sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			sptr->life = (GetRandomControl() & 7) + 16;
			sptr->sLife = sptr->life;
			sptr->x = (GetRandomControl() & 0x3F) + item->Pose.Position.x - 32;
			sptr->y = (GetRandomControl() & 0x3F) + item->Pose.Position.y - 32;
			sptr->z = (GetRandomControl() & 0x3F) + item->Pose.Position.z - 32;
			int size = item->ItemFlags[2];

			if (item->ItemFlags[2] == 4096)
				size = (GetRandomControl() & 0x7FF) + 2048;

			sptr->xVel = (short)((size * phd_sin(item->Pose.Orientation.y - 32768)) / SECTOR(1));
			sptr->yVel = -16 - (GetRandomControl() & 0xF);
			sptr->zVel = (short)((size * phd_cos(item->Pose.Orientation.y - 32768)) / SECTOR(1));
			sptr->friction = 4;
			sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

			if (!(GlobalCounter & 0x03))
				sptr->flags |= SP_DAMAGE;

			sptr->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				sptr->rotAdd = -8 - (GetRandomControl() & 7);
			else
				sptr->rotAdd = (GetRandomControl() & 7) + 8;

			sptr->scalar = 2;
			sptr->gravity = -8 - (GetRandomControl() & 0xF);
			sptr->maxYvel = -8 - (GetRandomControl() & 7);
			size = (GetRandomControl() & 0x1F) + 128;
			sptr->dSize = float(size);
			sptr->sSize = sptr->size = sptr->dSize / 2.0f;

			if (item->ItemFlags[1])
				item->ItemFlags[1]--;
			else
				item->ItemFlags[0] = item->TriggerFlags >> 4;
		}

		if (!normal)
			return;
	}

	if (!(Wibble & 0x0F) && (item->ObjectNumber != ID_STEAM_EMITTER || !(Wibble & 0x1F)))
	{
		int dx = Camera.pos.x - item->Pose.Position.x;
		int dz = Camera.pos.z - item->Pose.Position.z;

		if (dx < -SECTOR(16) || dx > SECTOR(16) || dz < -SECTOR(16) || dz > SECTOR(16))
			return;

		auto* sptr = GetFreeParticle();
		sptr->on = 1;
		sptr->sR = 0;
		sptr->sG = 0;
		sptr->sB = 0;
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;

		if (item->ObjectNumber == ID_SMOKE_EMITTER_BLACK)
		{
			sptr->dR = 96;
			sptr->dG = 96;
			sptr->dB = 96;
		}

		sptr->fadeToBlack = 16;
		sptr->colFadeSpeed = (GetRandomControl() & 3) + 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 7) + 28;

		if (item->ObjectNumber == ID_SMOKE_EMITTER_BLACK)
			sptr->blendMode = BLEND_MODES::BLENDMODE_SUBTRACTIVE;
		else
			sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

		sptr->x = (GetRandomControl() & 0x3F) + item->Pose.Position.x - 32;
		sptr->y = (GetRandomControl() & 0x3F) + item->Pose.Position.y - 32;
		sptr->z = (GetRandomControl() & 0x3F) + item->Pose.Position.z - 32;
		sptr->xVel = (GetRandomControl() & 0xFF) - 128;
		sptr->yVel = -16 - (GetRandomControl() & 0xF);
		sptr->zVel = (GetRandomControl() & 0xFF) - 128;
		sptr->friction = 3;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

		if (TestEnvironment(RoomEnvFlags::ENV_FLAG_OUTSIDE, item->RoomNumber))
			sptr->flags |= SP_WIND;

		sptr->rotAng = GetRandomControl() & 0xFFF;

		if (GetRandomControl() & 1)
			sptr->rotAdd = -8 - (GetRandomControl() & 7);
		else
			sptr->rotAdd = (GetRandomControl() & 7) + 8;

		sptr->scalar = 2;
		sptr->gravity = -8 - (GetRandomControl() & 0xF);
		sptr->maxYvel = -8 - (GetRandomControl() & 7);
		int size = (GetRandomControl() & 0x1F) + 128;
		sptr->dSize = float(size);
		sptr->sSize = sptr->size = float(size / 4);

		if (item->ObjectNumber == ID_STEAM_EMITTER)
		{
			sptr->gravity /= 2;
			sptr->yVel    /= 2;
			sptr->maxYvel /= 2;
			sptr->life  += 16;
			sptr->sLife += 16;
			sptr->dR = 32;
			sptr->dG = 32;
			sptr->dB = 32;
		}
	}
}
