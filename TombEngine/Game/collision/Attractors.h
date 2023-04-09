#pragma once

struct ItemInfo;

namespace TEN::Collision
{
	enum class AttractorType
	{
		Edge,
		/*VerticalPole,
		HorizontalPole,
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	class Attractor
	{
	private:
		// Members
		AttractorType Type		 = AttractorType::Edge;
		float		  Length	 = 0.0f;
		Vector3		  Point0	 = Vector3::Zero;
		Vector3		  Point1	 = Vector3::Zero;
		int			  RoomNumber = 0;

	public:
		// Constructors
		Attractor() {};
		Attractor(AttractorType type, const Vector3& point0, const Vector3& point1, int roomNumber);

		// Getters
		AttractorType GetType() const;
		float		  GetLength() const;
		Vector3		  GetPoint0() const;
		Vector3		  GetPoint1() const;
		int			  GetRoomNumber() const;

		// Setters
		void SetPoint0(const Vector3& point);
		void SetPoint1(const Vector3& point);

		// Inquirers
		bool IsEdge();

		// Helpers
		void DrawDebug(ItemInfo& item);
	};

	struct AttractorData
	{
		const Attractor* AttractorPtr = nullptr;

		Vector3 ClosestPoint = Vector3::Zero;

		float Distance		= 0.0f;
		short SlopeAngle	= 0;
		bool  IsIntersected = false;
		bool  IsInFront		= false;
	};
}
