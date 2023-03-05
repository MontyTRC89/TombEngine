#include "framework.h"
#include "Game/effects/Streamer.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Math/Math.h"

namespace TEN::Effects::Streamer
{
	void Streamer::AddSegment()
	{

	}

	void StreamerModule::AddStreamer()
	{

	}

	void StreamerController::GrowStreamer(int entityNumber, int tag, const Vector3& pos, const AxisAngle& orient, float life, float scaleRate, float width, float fadeAlpha)
	{

	}

	void StreamerController::Update()
	{

	}

	void StreamerController::Draw() const
	{

	}

	void StreamerController::Clear()
	{
		*this = {};
	}

	StreamerController StreamerEffect = {};

	//------------------------

	StreamerSegment Segments[STREAMER_SEGMENT_COUNT_MAX][(int)StreamerType::Count];

	static StreamerSegment& GetFreeStreamerSegment(StreamerType type)
	{
		for (auto& segment : Segments)
		{
			if (segment[(int)type].On)
				continue;

			return segment[(int)type];
		}

		return Segments[0][(int)type];
	}

	// TODO: If there is any segment on the water left, if lara stops and the velocity immidiatelly starts,
	// segment 0 combines with the left segment and stretches.
	static int GetPreviousStreamerSegmentIndex(StreamerType type)
	{
		int youngestStreamerIndex = 0;
		int youngestAge = 0;

		int index = 0;
		for (auto& segment : Segments)
		{
			if (segment[(int)type].Life > youngestAge &&
				segment[(int)type].On)
			{
				youngestAge = segment[(int)type].Life;
				youngestStreamerIndex = index;
			}

			index++;
		}

		return youngestStreamerIndex;
	}

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, float life, float fade)
	{
		constexpr auto OPACITY_MAX = 0.7f;

		auto& segment = GetFreeStreamerSegment((StreamerType)type);

		if (segment.On)
			return;

		int prevSegmentIndex = GetPreviousStreamerSegmentIndex((StreamerType)type);
		const auto& prevSegment = Segments[prevSegmentIndex][type];

		segment.On = true;
		segment.PreviousIndex = prevSegmentIndex;
		segment.Type = (StreamerType)type;
		segment.Direction = -EulerAngles(0, item->Pose.Orientation.y, 0).ToDirection();
		segment.Life = life;
		segment.Opacity = OPACITY_MAX;
		segment.Width = 1.0f;
		segment.ScaleRate = 1.0f * width;
		segment.FadeOut = fade;

		auto leftDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
		auto rightDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));

		auto leftVertex = Geometry::TranslatePoint(pos, leftDirection, segment.Width);
		auto rightVertex = Geometry::TranslatePoint(pos, rightDirection, segment.Width);;

		if ((StreamerType)type == StreamerType::Left)
		{
			segment.Vertices[0] = leftVertex;
			segment.Vertices[1] = rightVertex;
			segment.Vertices[3] = prevSegment.Vertices[0];
			segment.Vertices[2] = prevSegment.Vertices[1];
		}
		else
		{
			segment.Vertices[1] = leftVertex;
			segment.Vertices[0] = rightVertex;
			segment.Vertices[3] = prevSegment.Vertices[0];
			segment.Vertices[2] = prevSegment.Vertices[1];
		}
	}

	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int type, bool isOnWater, float width, float life, float fade)
	{
		float sinY = phd_sin(item->Pose.Orientation.y);
		float cosY = phd_cos(item->Pose.Orientation.y);

		//auto collPos = Geometry::TranslatePoint(item->Pose.Position, item->Pose.Orientation.y, )
		int x = item->Pose.Position.x + (zOffset * sinY) + (xOffset * cosY);
		int z = item->Pose.Position.z + (zOffset * cosY) - (xOffset * sinY);

		int probedRoomNumber = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).RoomNumber;
		int waterHeight = GetWaterHeight(x, item->Pose.Position.y, z, probedRoomNumber);

		if (isOnWater)
		{
			if (waterHeight != NO_HEIGHT)
			{
				auto pos = Vector3(x, yOffset, z);
				SpawnStreamerSegment(pos, item, type, width, life, fade);
			}
		}
		else
		{
			auto pos = Vector3(xOffset, yOffset, zOffset);
			SpawnStreamerSegment(pos, item, type, width, life, fade);
		}
	}

	void UpdateStreamers()
	{
		for (int i = 0; i < STREAMER_SEGMENT_COUNT_MAX; i++)
		{
			for (int j = 0; j < (int)StreamerType::Count; j++)
			{
				auto& segment = Segments[i][j];

				if (!segment.On)
					continue;

				auto* prevSegment = &Segments[segment.PreviousIndex][j];

				if (segment.Opacity > 0.0f)
					segment.Opacity -= 0.1f / segment.FadeOut;

				if (segment.Life <= 0.0f )
				{
					segment.On = false;
					continue;
				}

				auto leftDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
				auto rightDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));

				/*auto leftVertex = Geometry::TranslatePoint(segment.Vertices[0], leftDirection, segment.Width);
				auto rightVertex = Geometry::TranslatePoint(segment.Vertices[1], rightDirection, segment.Width);;

				segment.Vertices[0] = leftVertex;
				segment.Vertices[1] = rightVertex;
				segment.Vertices[2] = prevSegment->Vertices[1];
				segment.Vertices[3] = prevSegment->Vertices[0];*/

				int zOffset = 0;
				float sinY = phd_sin(EulerAngles(-segment.Direction).y);
				float cosY = phd_cos(EulerAngles(-segment.Direction).y);

				switch (segment.Type)
				{
				case StreamerType::Center:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[2] = prevSegment->Vertices[1];
					segment.Vertices[3] = prevSegment->Vertices[0];
					break;

				case StreamerType::Left:
					segment.Vertices[0] -= Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					segment.Vertices[1] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[2] = prevSegment->Vertices[1];
					segment.Vertices[3] = prevSegment->Vertices[0];
					break;

				case StreamerType::Right:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] += Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					segment.Vertices[2] = prevSegment->Vertices[1];
					segment.Vertices[3] = prevSegment->Vertices[0];
					break;
				}

				segment.Life -= 1.0f;				
			}
		}			
	}
}
