#include "framework.h"
#include "Game/effects/boatFX.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Math/Math.h"

namespace TEN::Effects::Streamer
{
	WaveSegment Segments[NUM_WAKE_SPRITES][NUM_WAKE_DIRECTION];
	
	void DoWakeEffect(ItemInfo* item, int xOffset, int yOffset, int zOffset, int waveDirection, bool isOnWater, float width, int life, float fade)
	{
		float sinY = phd_sin(item->Pose.Orientation.y);
		float cosY = phd_cos(item->Pose.Orientation.y);

		int x = item->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = item->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, item->Pose.Position.y, z, probedRoomNumber);

		if (isOnWater)
		{
			if (waterHeight != NO_HEIGHT)
			{
				auto pos = Vector3(x, yOffset, z);
				SpawnWaveSegment(pos, item, waveDirection, width, life, fade);
			}
		}
		else
		{
			auto pos = Vector3(xOffset, yOffset, zOffset);
			SpawnWaveSegment(pos, item, waveDirection, width, life, fade);
		}
	}

	void UpdateStreamers()
	{
		for (int i = 0; i < NUM_WAKE_SPRITES; i++)
		{
			for (int j = 0; j < NUM_WAKE_DIRECTION; j++)
			{
				auto* segment = &Segments[i][j];

				if (!segment->On)
					continue;

				auto* prevSegment = &Segments[segment->PreviousID][j];

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
				case (int)WaveDirection::WAVE_DIRECTION_LEFT:
					segment->Vertices[0] -= Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
					segment->Vertices[1] -= Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[2] = prevSegment->Vertices[1];
					segment->Vertices[3] = prevSegment->Vertices[0];
					break;

				case (int)WaveDirection::WAVE_DIRECTION_RIGHT:
					segment->Vertices[1] += Vector3((zOffset * sinY) + (segment->ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment->ScaleRate * sinY));
					segment->Vertices[0] += Vector3((zOffset * sinY) + ((segment->ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment->ScaleRate / 2) * sinY));
					segment->Vertices[2] = prevSegment->Vertices[1];
					segment->Vertices[3] = prevSegment->Vertices[0];
					break;

				case (int)WaveDirection::WAVE_DIRECTION_CENTRAL:
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

		auto* prevSegment = &Segments[pvSegment][waveDirection];

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
		auto verticelPos = Vector3(x, origin.y, z);

		x = segment.Direction.x + (zOffset * sinY) + (segment.width * cosY);
		z = segment.Direction.z + (zOffset * cosY) - (segment.width * sinY);
		auto verticerPos = Vector3(x, origin.y, z);

		if (waveDirection == (int)WaveDirection::WAVE_DIRECTION_LEFT)
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

	// TODO: If there is any segment on the water left, if lara stops and the velocity immidiatelly starts,
	// segment 0 combines with the left segment and stretches.
	int GetPreviousSegment(int waveDirection)
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
