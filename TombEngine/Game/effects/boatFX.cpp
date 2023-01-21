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
using std::array;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::BOATFX
{


	constexpr auto NUM_WAKE_SPRITES = 32;
	constexpr auto NUM_WAKE_WITDH = 32;
	

	std::array<WaveSegment, NUM_WAKE_SPRITES> Segments;
	std::array<Wave, 1> Waves;


	void KayakUpdateWakeFX()
	{


		for (int i = 0; i < Segments.size(); i++)
		{
			auto& segment = Segments[i];

			auto& prevSegment = Segments[i];

			
				
		
			

			if (!segment.On)
				continue;

			segment.Age++;
			segment.Life--; // NOTE: Life tracked in frame time.
			if (segment.Life <= 0.0f)
			{
				segment.On = false;
				continue;
			}

		

			segment.ScaleRate += 1.0f;

			//wave.Segments[i].Vertices[0] = wave.Segments[i - 1].Vertices[0];
			//wave.Segments[i].Vertices[1] = wave.Segments[i - 1].Vertices[1];
		//segment.Segments[i].ScaleRate

		// Update opacity.
		//segment.Opacity = Lerp(1.0f, 0.0f, 1.0f - (segment.Life / 16));
		//segment.Color.w = segment.Opacity;
		}
			
	}

	void SpawnWaveSegment(const Vector3& origin, const Vector3& target, int roomNumber, const Vector4& color)
	{
		Vector3 startVector = origin;
		Vector3 targetVector = target;
		Vector3 lineVector = targetVector - startVector;

		lineVector.Normalize();;// * 64;
		//lineVector = lineVector * 64;
		targetVector = startVector + lineVector;
		//g_Renderer.AddLine3D(startVector, targetVector, Vector4(255, 255, 255, 1));

		//g_Renderer.AddLine3D(basePos, targetPos, Vector4(color));
		


			auto& segment = GetFreeWaveSegment();
			auto& prevSegment = GetPreviousSegment();


					if (segment.On)
						return;


					
					segment.On = true;
					segment.Age = 0;
					segment.Vertices[0] = startVector  ;
					segment.Vertices[1] = startVector + Vector3(47.0f, 0.0f, 0.0f);

						//(g_Renderer.AddDebugSphere(wave.Segments[i].Vertices[3], 32, Vector4(color), RENDERER_DEBUG_PAGE::NO_PAGE);

					

					segment.Vertices[3] = prevSegment.Vertices[0];
					segment.Vertices[2] = prevSegment.Vertices[1];// +Vector3(47.0f, 0.0f, 0.0f);

					segment.On = true;

					segment.RoomNumber = roomNumber;

					segment.Opacity = 0.2f;
					segment.Life = 64;
					//segment.Segments[i].ScaleRate =

					/*for (int i = 0; i < 32; i++)
					{

						wave.Segments[i].On = true;
						wave.Segments[i].Vertices[0] = startVector;
						wave.Segments[i].Vertices[1] = startVector + Vector3(47.0f, 0.0f, 0.0f);

						//(g_Renderer.AddDebugSphere(wave.Segments[i].Vertices[3], 32, Vector4(color), RENDERER_DEBUG_PAGE::NO_PAGE);

						wave.Segments[i].Vertices[3] = targetVector;
						wave.Segments[i].Vertices[2] = targetVector + Vector3(47.0f, 0.0f, 0.0f);

						wave.Segments[i].On = true;

						wave.Segments[i].RoomNumber = roomNumber;

						wave.Segments[i].Opacity = 1.0f;
						wave.Segments[i].Life = 16;
					}*/



				
			
		
	
		
	}

	void ClearWaveSegment()
	{
		//for (auto& segment : Waves)
		//	segment = {};
	}

	WaveSegment& GetFreeWaveSegment()
	{

		for (int i = 0; i < Segments.size(); i++)
			{
				if (!Segments[i].On)
					return Segments[i];
			}
		

		return Segments[0];
	}

	WaveSegment& GetPreviousSegment()
	{
		int oldestLifeIndex = 0;
		int youngestAge = 0;

		for (int i = 0; i < Segments.size(); i++)
		{
			

			if (youngestAge < Segments[i].Life)
			{
				youngestAge = Segments[i].Life;
				oldestLifeIndex = i;
			}


			if (!Segments[i].On)
			{


				if (i > 0)				
					return Segments[i - 1];
				//else
				//	return Segments[oldestLifeIndex];
					
				


			}

		}


		return Segments[oldestLifeIndex];
	}


}
