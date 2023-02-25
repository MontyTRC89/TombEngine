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

using TEN::Renderer::g_Renderer;

namespace TEN::Effects::BOATFX
{
	enum WaveDirection
	{
		WAVE_DIRECTION_CENTRAL,
		WAVE_DIRECTION_LEFT,
		WAVE_DIRECTION_RIGHT
	};
	
	WaveSegment Segments[NUM_WAKE_SPRITES][NUM_WAKE_DIRECTION];
	
	void DoWakeEffect(ItemInfo* Item, int xOffset, int yOffset, int zOffset, int waveDirection, bool OnWater, float width, int life, float fade)
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
				Vector3 pos1 = Vector3(x, yOffset, z);

				SpawnWaveSegment(pos1, Item, waveDirection, width, life, fade);
			}
		}
		else
		{
			Vector3 pos1 = Vector3(xOffset, yOffset, zOffset);

			SpawnWaveSegment(pos1, Item, waveDirection, width, life, fade);
		}
	}

	void UpdateWakeFX()
	{
		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			for (int j = 0; j < NUM_WAKE_DIRECTION; j++)
			{
				WaveSegment* segment = &Segments[i][j];

				if (!segment->On)
					continue;

				WaveSegment* prevSegment = &Segments[segment->PreviousID][j];

				if (segment->Opacity > 0.0f)
					segment->Opacity -= 0.1f / segment->FadeOut;

				if (segment->Life <= 0.0f )
				{
					segment->On = false;
					continue;
				}

				int zOffset = 0;

				float sinY = phd_sin(segment->Orientation.y);
				float cosY = phd_cos(segment->Orientation.y);

				switch (segment->StreamerID)
				{
					case WAVE_DIRECTION_LEFT:

						segment->Vertices[0] -= Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
						segment->Vertices[1] -= Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
						segment->Vertices[2] = prevSegment->Vertices[1];
						segment->Vertices[3] = prevSegment->Vertices[0];

						break;

					case WAVE_DIRECTION_RIGHT:

						segment->Vertices[1] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
						segment->Vertices[0] += Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
						segment->Vertices[2] = prevSegment->Vertices[1];
						segment->Vertices[3] = prevSegment->Vertices[0];

						break;

					case WAVE_DIRECTION_CENTRAL:

						segment->Vertices[1] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
						segment->Vertices[0] -= Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
						segment->Vertices[2] = prevSegment->Vertices[1];
						segment->Vertices[3] = prevSegment->Vertices[0];

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

		if (waveDirection == WAVE_DIRECTION_LEFT)
		{
			segment.Vertices[0] = verticelPos;
			segment.Vertices[1] = verticerPos;
			segment.Vertices[3] = prevSegment->Vertices[0];
			segment.Vertices[2] = prevSegment->Vertices[1];
		}
		else
		{
			segment.Vertices[1] = verticelPos;
			segment.Vertices[0] = verticerPos;
			segment.Vertices[3] = prevSegment->Vertices[0];
			segment.Vertices[2] = prevSegment->Vertices[1];
		}

		segment.Opacity = 0.7f;

		segment.Life = life;		  
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

	int GetPreviousSegment(int waveDirection) //TODO: if there is any segment on the water left, if lara stops and the velocity immidiatelly starts, segment 0 combines with the left segment and stretches
	{
		int youngestLifeIndex = 0;
		int youngestAge = 0;

		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			if (Segments[i][waveDirection].Life > youngestAge && Segments[i][waveDirection].On)
			{
				youngestAge = Segments[i][waveDirection].Life;
				youngestLifeIndex = i;				
			}
		}

		return youngestLifeIndex;
	}

}
