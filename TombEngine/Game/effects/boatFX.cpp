#include "framework.h"
#include "Game/effects/boatFX.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
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
	enum WaveDirection
	{
		WAVE_DIRECTION_CENTRAL,
		WAVE_DIRECTION_LEFT,
		WAVE_DIRECTION_RIGHT
	};

	constexpr auto NUM_WAKE_WITDH = 32;
	
	WaveSegment Segments[NUM_WAKE_SPRITES][3];
	
	void DoWakeEffect(ItemInfo* Item, int xOffset, int zOffset, int waveDirection, bool OnWater, float width, int life, float fade)
	{
		float sinY = phd_sin(Item->Pose.Orientation.y);
		float cosY = phd_cos(Item->Pose.Orientation.y);

		int x = Item->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = Item->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, Item->Pose.Position.y, z, Item->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, Item->Pose.Position.y, z, probedRoomNumber);

		if (OnWater)
		{
			if (waterHeight != NO_HEIGHT)
			{
				int offset1 = -128;
				int offset2 = 0;

				int s = phd_sin(Item->Pose.Orientation.y);
				int c = phd_cos(Item->Pose.Orientation.y);
				int x1 = (Item->Pose.Position.x + ((offset2 * s + offset1 * c) >> W2V_SHIFT));
				int y1 = Item->Pose.Position.y;
				int z1 = (Item->Pose.Position.z + ((offset2 * c - offset1 * s) >> W2V_SHIFT));

				Vector3 pos1 = Vector3(x, Item->Pose.Position.y, z);

				SpawnWaveSegment(pos1, Item, waveDirection, width, life, fade);
			}
		}
		//TODO: Make code for UPV with included Y axis
	}

	void KayakUpdateWakeFX()
	{

		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				
				WaveSegment* segment = &Segments[i][j];

				

			
				
				
	
				if (!segment->On)
					continue;

			
			

				if (segment->Life <= 0.0f)
				{
					segment->On = false;
					continue;
				}
			
			
				WaveSegment* prevSegment = &Segments[segment->PreviousID][j];
				
				


				int zOffset = 0;

				float sinY = phd_sin(segment->Orientation.y);
				float cosY = phd_cos(segment->Orientation.y);

				if (segment->Opacity > 0.0f)
					segment->Opacity -= 0.1f / segment->FadeOut;// / segment->Age;//15

				

				switch (segment->StreamerID)
				{

				case WAVE_DIRECTION_LEFT:

					segment->Vertices[2] = prevSegment->Vertices[1];// -Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[1] -=  Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[3] = prevSegment->Vertices[0];// -Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
					segment->Vertices[0] -=   Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));

					break;

				case WAVE_DIRECTION_RIGHT:
					segment->Vertices[2] = prevSegment->Vertices[1];//Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
					segment->Vertices[1] += Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
					segment->Vertices[3] = prevSegment->Vertices[0]; Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[0] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					break;

				case WAVE_DIRECTION_CENTRAL:
					segment->Vertices[2] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[1] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[3] -= Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[0] -= Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					break;
				}

				segment->Life--;
			}
		}			
	}

	void SpawnWaveSegment(const Vector3& origin, ItemInfo* Item, int waveDirection, float width, int life, float fade)
	{

		
		auto& segment = GetFreeWaveSegment(waveDirection);

		if (segment.On)
			return;

		int pvSegment = GetPreviousSegment(waveDirection);

		WaveSegment* prevSegment = &Segments[pvSegment][waveDirection];

	

		segment.On = true;
		segment.PreviousID = pvSegment;
		segment.StreamerID = waveDirection;
		segment.Direction = origin;
		segment.Orientation = Item->Pose.Orientation;
		segment.ScaleRate = 1.0f * width;
		segment.width = 1.0f ;
		segment.FadeOut = fade;

		int zOffset = 0;

		float sinY = phd_sin(Item->Pose.Orientation.y);
		float cosY = phd_cos(Item->Pose.Orientation.y);

		int x = segment.Direction.x + (zOffset * sinY) - (segment.width * cosY);
		int z = segment.Direction.z + (zOffset * cosY) + (segment.width * sinY);
		Vector3 verticelPos = Vector3(x, origin.y, z);

		x = segment.Direction.x + (zOffset * sinY) + (segment.width * cosY);
		z = segment.Direction.z + (zOffset * cosY) - (segment.width * sinY);
		Vector3 verticerPos = Vector3(x, origin.y, z);

		segment.Vertices[0] = verticelPos;
		segment.Vertices[1] = verticerPos;
		//segment.Vertices[3] = prevSegment->Vertices[0];
		//segment.Vertices[2] = prevSegment->Vertices[1];
		segment.RoomNumber = Item->RoomNumber;
		segment.Opacity = 0.5f;
		segment.Life = life;
		segment.Age =  (Item->Animation.Velocity.z / 2);
		  
	}

	WaveSegment& GetFreeWaveSegment(int waveDirection)
	{
		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			if (!Segments[i][waveDirection].On)
				return Segments[i][waveDirection];
		}
		
		return Segments[0][waveDirection];
	}

	int GetPreviousSegment(int waveDirection)
	{
		int youngestLifeIndex = 0;
		int youngestAge = 0;

		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			if ( Segments[i][waveDirection].Life > youngestAge )
			{
				youngestAge = Segments[i][waveDirection].Life;
				youngestLifeIndex = i;
			}
		}

		return  youngestLifeIndex;
	}



}
