#include "framework.h"
#include "Objects/Effects/EmberEmitter.h"

#include "Game/effects/effects.h"
#include "Game/Setup.h"

	void EmberEmitterControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		unsigned char r;
		unsigned char g;
		unsigned char b;

		if (!TriggerActive(item))
			return;

		//For a grey color, all color values have to be the same. Else it gives some variation to the color. Grey with one color different results to a multicolor emitter.
		if  (item->Model.Color.x == item->Model.Color.y && item->Model.Color.y == item->Model.Color.z)
		{
			r = g = b = item->Model.Color.x * UCHAR_MAX;
		}
		else
		{
			r = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.x * UCHAR_MAX), 0, UCHAR_MAX);
			g = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.y * UCHAR_MAX), 0, UCHAR_MAX);
			b = std::clamp(Random::GenerateInt(-32, 32) + int(item->Model.Color.z * UCHAR_MAX), 0, UCHAR_MAX);
		}

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
				spark->fadeToBlack = 4;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 4;

				//Only The color black (0, 0, 0) has a subtractive blending mode.
				if (!item->Model.Color.x && !item->Model.Color.y && !item->Model.Color.z)
				{
					spark->sR = spark->sB = spark->sG = spark->dR = spark->dB = spark->dG = 255;
					spark->blendMode = BlendMode::Subtractive;
				}
				else
				{
					spark->sR = r;
					spark->sB = b;
					spark->sG = g;
					spark->dR = r;
					spark->dB = b;
					spark->dG = g;
					spark->blendMode = BlendMode::Additive;
				}

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

