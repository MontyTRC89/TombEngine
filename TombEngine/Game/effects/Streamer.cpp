#include "framework.h"
#include "Game/effects/Streamer.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
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

	std::array<std::vector<StreamerSegment>, (int)StreamerType::Count> Streamers = {};

	static StreamerSegment& GetNewStreamerSegment(StreamerType type)
	{
		constexpr auto COUNT_MAX = 256;

		// Add and return new segment.
		if (Streamers[(int)type].size() < COUNT_MAX)
			return Streamers[(int)type].emplace_back();

		// Clear and return oldest segment.
		auto& segment = Streamers[(int)type][0];
		segment = {};
		return segment;
	}

	void ClearInactiveStreamerSegments()
	{
		for (auto& streamer : Streamers)
			ClearInactiveEffects(streamer);
	}

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, float life, float fade)
	{
		constexpr auto OPACITY_MAX = 0.7f;

		auto& segment = GetNewStreamerSegment((StreamerType)type);
		int prevSegmentIndex = std::max((int)Streamers[type].size() - 2, 0);
		const auto& prevSegment = Streamers[type][prevSegmentIndex];

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
		}
		else
		{
			segment.Vertices[1] = leftVertex;
			segment.Vertices[0] = rightVertex;
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
		for (auto& streamer : Streamers)
		{
			for (int i = 0; i < streamer.size(); i++)
			{
				auto& segment = streamer[i];
				const auto& prevSegment = streamer[std::max(i - 1, 0)];

				if (segment.Opacity > 0.0f)
					segment.Opacity -= 0.1f / segment.FadeOut;

				auto leftDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(-90.0f), 0));
				auto rightDirection = Geometry::RotatePoint(segment.Direction, EulerAngles(0, ANGLE(90.0f), 0));

				/*auto leftVertex = Geometry::TranslatePoint(segment.Vertices[0], leftDirection, segment.Width);
				auto rightVertex = Geometry::TranslatePoint(segment.Vertices[1], rightDirection, segment.Width);;

				segment.Vertices[0] = leftVertex;
				segment.Vertices[1] = rightVertex;*/

				int zOffset = 0;
				float sinY = phd_sin(EulerAngles(-segment.Direction).y);
				float cosY = phd_cos(EulerAngles(-segment.Direction).y);

				switch (segment.Type)
				{
				case StreamerType::Center:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					break;

				case StreamerType::Left:
					segment.Vertices[0] -= Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					segment.Vertices[1] -= Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					break;

				case StreamerType::Right:
					segment.Vertices[1] += Vector3((zOffset * sinY) + (segment.ScaleRate * cosY), 0.0f, (zOffset * cosY) - (segment.ScaleRate * sinY));
					segment.Vertices[0] += Vector3((zOffset * sinY) + ((segment.ScaleRate / 2) * cosY), 0.0f, (zOffset * cosY) - ((segment.ScaleRate / 2) * sinY));
					break;
				}

				segment.Life -= 1.0f;
			}
		}

		ClearInactiveStreamerSegments();
	}
}
