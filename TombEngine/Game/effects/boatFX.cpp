#include "framework.h"
#include "Game/effects/boatFX.h"

#include "Game/animation.h"
#include "Game/effects/bubble.h"
#include "Game/effects/drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Math/Math.h"

using std::vector;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::BOATFX
{
	constexpr auto NUM_WAKE_SPRITES = 32;
	constexpr auto NUM_WAKE_WITDH = 32;
	
	//int LightningRandomSeed = 0x0D371F947;
	//float FloatSinCosTable[8192];
	Vector3i WakeFXPos[6];
	short WakeFXBuffer[1024];
	
	std::vector<WAKE_PTS> WakePts;

	/*void InitialiseFloatSinCosTable()
	{
		for (int i = 0; i < 8192; i++)
		{
			FloatSinCosTable[i] = sin(i * 0.000095873802f);
		}
	}*/

	void KayakUpdateWakeFX()
	{
		for (int i = 0; i < WakePts.size(); i++)
		{
			WAKE_PTS* waves = &WakePts[i];

			if (waves->life > 0)
			{
				waves->life -= 1.0f; // NOTE: Life tracked in frame time.
				if (waves->life <= 0.0f)
				{
					waves->IsActive = false;
					continue;
				}
			}
		}

		if (WakePts.size() > 0)
		{
			WakePts.erase(
				std::remove_if(WakePts.begin(), WakePts.end(),
					[](const WAKE_PTS& o) { return o.life == 0; }),
				WakePts.end());
		}
	}

	WAKE_PTS* TriggerWakeFX(const Vector3& origin, const Vector3& target, Vector4* color, char length)
	{
		WAKE_PTS waves;


		auto normal = target - origin;
		normal.Normalize();
		float length4 = Vector3::Distance(origin, target);
		float spacing = 64;//with
		float offset = 250.0f;



		//auto offset = Vector3(0.0f, width / 2, 0.0f);
		auto basePos = (origin + ((length / 2) * normal));

		waves.IsActive = true;
		waves.life = 64;
		waves.pos1 = basePos ;
		waves.pos2 = target;
		waves.Normal = normal;
		waves.length = length;
		waves.width = NUM_WAKE_WITDH;


		WakePts.push_back(waves);
		return &WakePts[WakePts.size() - 1];
	}



	void CalcSpline(Vector3i* pos, short* buffer, WAKE_PTS* wave)
	{


		buffer += 4;

		
			int segments = 32 ;

			int dx = (pos[1].x - pos->x) / segments;
			int dy = (pos[1].y - pos->y) / segments;
			int dz = (pos[1].z - pos->z) / segments;

			int x = dx +  pos->x;
			int y = dy +   pos->y;
			int z = dz +  pos->z;

			if (segments > 0)
			{
				for (int i = segments; i > 0; i--)
				{
					buffer[0] = x;
					buffer[1] = y;
					buffer[2] = z;

					x += dx  ;
					y += dy ;
					z += dz ;

					buffer += 4;
				}
			}
		

	}




}
