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


	Vector3 RotatePoint(const Vector3& point, const EulerAngles& rotation)
	{
		auto rotMatrix = rotation.ToRotationMatrix();
		return Vector3::Transform(point, rotMatrix);
	}

	void KayakUpdateWakeFX()
	{


		for (int i = 0; i < Segments.size(); i++)
		{
			auto& segment = Segments[i];

			auto& prevSegment = Segments[i];

			
				
		
			

			if (!segment.On)
				continue;

			segment.Age++;
			segment.Life--; // NOTE: Life.
			if (segment.Life <= 0.0f)
			{
				segment.On = false;
				continue;
			}

			segment.ScaleRate += 1.0f;

			//auto lDirection = RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
			//segment.Vertices[0] += lDirection * segment.ScaleRate; // Or maybe v1, also make sure ScaleRate was defined in the spawn function...

			//auto rDirection = RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));
			//segment.Vertices[1] += rDirection * segment.ScaleRate;



		//TODO: Update opacity.

		}
			
	}

	void SpawnWaveSegment(const Vector3& origin, const Vector3& target, ItemInfo* kayakItem, const Vector4& color, const Vector3& width)
	{




		int segmentLenght = 47;



		//NOTE: Width
		auto direction = (  origin - target);
		direction.Normalize();


		auto lRotMatrix = EulerAngles(0, ANGLE(-90.0f), 0).ToRotationMatrix();
		auto rRotMatrix = EulerAngles(0, ANGLE(90.0f), 0).ToRotationMatrix();

		auto lDirection = RotatePoint(direction,  EulerAngles(0, ANGLE(-90.0f), 0));
		auto rDirection = RotatePoint(direction, EulerAngles(0, ANGLE(90.0f), 0));


	

			auto& segment = GetFreeWaveSegment();
			auto& prevSegment = GetPreviousSegment();

					if (segment.On)
						return;

					//NOTE: set the first two vertices

					segment.Direction = direction;
					segment.On = true;
					segment.Age = 0;

					segment.Vertices[0] = origin ;// startVector;
					segment.Vertices[1] = origin +Vector3(47.0f, 0.0f, 0.0f);// 

					//g_Renderer.AddDebugSphere(lDirection, 32, Vector4(color), RENDERER_DEBUG_PAGE::NO_PAGE);

					// NOTE: move the other two vertices to the position of the previous segment

					segment.Vertices[3] = prevSegment.Vertices[0];
					segment.Vertices[2] = prevSegment.Vertices[1];

					segment.On = true;

					segment.RoomNumber = kayakItem->RoomNumber;

					segment.Opacity = 0.2f;
					segment.Life = 64;

		
	}

	void ClearWaveSegment()
	{
//not needed TODO: delete later 
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
		int youngestLifeIndex = 0;
		int youngestAge = 0;

		for (int i = 0; i < Segments.size(); i++)
		{
			

			if (youngestAge < Segments[i].Life)
			{
				youngestAge = Segments[i].Life;
				youngestLifeIndex = i;
			}


			if (!Segments[i].On)
			{


				if (i > 0)				
					return Segments[i - 1];

			}

		}

		return Segments[youngestLifeIndex];
	}


}
