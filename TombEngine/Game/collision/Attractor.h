#pragma once

struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractor
{
	class Attractor;

	enum class AttractorType
	{
		Edge,
		/*VerticalPole, // """""""""Polerope"""""""""
		HorizontalPole, // TODO: AOD pipe shimmy + pipe crawl.
		SwingPole,
		ZipLine,
		Tightrope,
		Pinnacle*/
	};

	class Attractor
	{
	private:
		// Members
		AttractorType		 _type			 = AttractorType::Edge;
		std::vector<Vector3> _points		 = {};
		int					 _roomNumber	 = 0;
		std::vector<float>	 _segmentLengths = {};
		float				 _length		 = 0.0f;
		BoundingBox			 _box			 = BoundingBox();

		// TODO: std::set doesn't work.
		//std::set<int> _attachedPlayerItemNumebrs = {};
		int _attachedPlayerItemNumber = -1;

	public:
		// Constructors
		Attractor() {}; // Debug only.
		Attractor(AttractorType type, const std::vector<Vector3>& points, int roomNumber);

		// Destructors
		~Attractor();

		// Getters
		AttractorType				GetType() const;
		const std::vector<Vector3>& GetPoints() const;
		int							GetRoomNumber() const;
		const std::vector<float>&	GetSegmentLengths() const;
		float						GetLength() const;
		const BoundingBox&			GetBox() const;

		// Inquirers
		bool IsLooped() const;

		// Utilities
		Vector3		 GetIntersectionAtChainDistance(float chainDist) const;
		unsigned int GetSegmentIDAtChainDistance(float chainDist) const;

		void Update(const std::vector<Vector3>& points, int roomNumber);
		void AttachPlayer(ItemInfo& playerItem);
		void DetachPlayer(ItemInfo& playerItem);
		void DetachAllPlayers();

		void DrawDebug(unsigned int segmentID) const;

	private:
		// Helpers
		float NormalizeChainDistance(float chainDist) const;
		void  CacheSegmentLengths();
		void  CacheLength();
		void  CacheBox();
	};

	Attractor GenerateBridgeAttractor(const ItemInfo& bridgeItem);
}
