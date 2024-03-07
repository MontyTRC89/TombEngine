#include "framework.h"
#include "Objects/Effects/ember_emitter.h"

#include "Game/effects/effects.h"
#include "Game/Setup.h"

	void EmberEmitterControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TriggerActive(item))
			return;

		unsigned char r = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.x * UCHAR_MAX), 0, UCHAR_MAX);
		unsigned char g = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.y * UCHAR_MAX), 0, UCHAR_MAX);
		unsigned char b = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.z * UCHAR_MAX), 0, UCHAR_MAX);

		if (item->TriggerFlags)

		{
			if (!item->ItemFlags[2])
			{
				int div = item->TriggerFlags % 10 << 10;
				int mod = item->TriggerFlags / 10 << 10;
				item->ItemFlags[0] = GetRandomControl() % div;
				item->ItemFlags[1] = GetRandomControl() % mod;
				item->ItemFlags[2] = (GetRandomControl() & 0xF) + 15;
			}

			if (--item->ItemFlags[2] < 15)
			{
				auto* spark = GetFreeParticle();
				spark->on = 1;
				spark->sR = -1;
				spark->sB = 16;
				spark->sG = (GetRandomControl() & 0x1F) + 48;
				spark->dR = (GetRandomControl() & 0x3F) - 64;
				spark->dB = 0;
				spark->dG = (GetRandomControl() & 0x3F) + -128;
				spark->fadeToBlack = 4;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
				spark->blendMode = BlendMode::Additive;
				spark->life = spark->sLife = (GetRandomControl() & 3) + 24;
				spark->x = item->ItemFlags[1] + (GetRandomControl() & 0x3F) + item->Pose.Position.x - 544;
				spark->y = item->Pose.Position.y;
				spark->z = item->ItemFlags[0] + (GetRandomControl() & 0x3F) + item->Pose.Position.z - 544;
				spark->xVel = (GetRandomControl() & 0x1FF) - 256;
				spark->friction = 6;
				spark->zVel = (GetRandomControl() & 0x1FF) - 256;
				spark->rotAng = GetRandomControl() & 0xFFF;
				spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
				spark->maxYvel = 0;
				spark->yVel = -512 - (GetRandomControl() & 0x3FF);
				spark->sSize = spark->size = (GetRandomControl() & 0x0F) + 32;
				spark->dSize = spark->size / 4;

				if (GetRandomControl() & 3)
				{
					spark->flags = SP_DAMAGE | SP_ROTATE | SP_DEF | SP_SCALE | SP_EXPDEF;
					spark->scalar = 3;
					spark->gravity = (GetRandomControl() & 0x3F) + 32;
				}
				else
				{
					spark->flags = SP_ROTATE | SP_DEF | SP_SCALE;
					spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST;
					spark->scalar = 1;
					spark->gravity = (GetRandomControl() & 0xF) + 64;
				}
			}
		}
		else
		{
			if (GetRandomControl() & 7)
			{
				auto* spark = GetFreeParticle();
				spark->on = 1;

				spark->sR = r;
				spark->sB = b;
				spark->sG = g;
				spark->dR = r;
				spark->dB = b;
				spark->dG = g;
				spark->fadeToBlack = 4;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
				spark->blendMode = BlendMode::Additive;
				spark->life = spark->sLife = (GetRandomControl() & 3) + 74;
				spark->x = (GetRandomControl() & 0x15) + item->Pose.Position.x;
				spark->y = item->Pose.Position.y;
				spark->z = (GetRandomControl() & 0x15) + item->Pose.Position.z;
				spark->rotAng = (GetRandomControl() - 0x4000) * 2;
				spark->yVel = -BLOCK(1.1f) - (GetRandomControl() & 0x6FF);
				spark->gravity = (GetRandomControl() & 0xF) + 64; 
				spark->xVel = (GetRandomControl() & 0x2FF) - 386;
				spark->friction = 15;
				spark->maxYvel = 0;
				spark->zVel = (GetRandomControl() & 0x2FF) - 386;
				spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST;
				spark->flags = SP_DAMAGE |SP_ROTATE | SP_DEF | SP_SCALE;
				spark->scalar = 1;
				spark->sSize = spark->size = (GetRandomControl() & 0x0F) + 32;
				spark->dSize = spark->size;
			}
		}
	}

