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


	constexpr auto NUM_WAKE_SPRITES = 64;
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
		int youngestLifeIndex = 0;
		int youngestAge = 0;
		const auto& revSegment = Segments[0];
		int testnumber = 0;
	


		for (int i = 0; i < Segments.size(); i++)
		{
			auto& segment = Segments[i];

			if (!segment.On)
				continue;

			segment.Age++;
			segment.Life--; 

			if (segment.Life <= 0.0f)
			{
				segment.On = false;
				continue;
			}

			int zOffset = 0;

			float sinY = phd_sin(segment.pos.Orientation.y);
			float cosY = phd_cos(segment.pos.Orientation.y);


			segment.Vertices[2]  -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY)) ;
			segment.Vertices[1] -=  Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY)) ;
			segment.Vertices[3] -= Vector3((zOffset * sinY) + (0.2f * cosY), 0.0f, (zOffset * cosY) - (0.2f * sinY));
			segment.Vertices[0] -= Vector3((zOffset * sinY) + (0.2f * cosY), 0.0f, (zOffset * cosY) - (0.2f * sinY));

			if (segment.Opacity > 0.0f)
			segment.Opacity -= 0.1f /15 ;

		}
			
	}

	void SpawnWaveSegment(const Vector3& origin, const Vector3& target, ItemInfo* kayakItem, const Vector4& color, int Velocity)
	{

		auto& segment = GetFreeWaveSegment();
		auto& prevSegment = GetPreviousSegment();

		if (segment.On)
			return;


		


		segment.On = true;
		segment.Age = 0;
		segment.pos = kayakItem->Pose;



		segment.Direction = origin;
		segment.ScaleRate = 1.0f;
		segment.width = 1.0f;
		segment.PrevSegment = Velocity;

		int zOffset = 0;

		float sinY = phd_sin(segment.pos.Orientation.y);
		float cosY = phd_cos(segment.pos.Orientation.y);

		int x = segment.Direction.x  + (zOffset * sinY) + (segment.width * cosY);
		int z = segment.Direction.z + (zOffset * cosY) - (segment.width * sinY);

		Vector3 verticerPos = Vector3(x, origin.y, z);
	
		 x = segment.Direction.x - (zOffset * sinY) + (segment.width * cosY);
		 z = segment.Direction.z - (zOffset * cosY) - (segment.width * sinY);

		Vector3 vertice1Pos = Vector3(x, origin.y, z);
		


					segment.Vertices[0] = origin;// startVector;
					segment.Vertices[1] = verticerPos;// 

					//segment.Vertices[3] = origin;// startVector;
					//segment.Vertices[2] = vertice1Pos;// 
					segment.Vertices[3] = prevSegment.Vertices[0];
					segment.Vertices[2] = prevSegment.Vertices[1];

					segment.On = true;

					segment.RoomNumber = kayakItem->RoomNumber;

					segment.Opacity = 0.5f;
					segment.Life = 84;

		
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
