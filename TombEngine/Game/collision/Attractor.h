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
		AttractorType		 _type		 = AttractorType::Edge;
		std::vector<Vector3> _points	 = {};
		int					 _roomNumber = 0;
		float				 _length	 = 0.0f;
		BoundingBox			 _box		 = BoundingBox(); 

		// TODO
		//std::unordered_map<int, ItemInfo*> _attachedPlayers = {}; // Key = item number

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
		float						GetLength() const;
		const BoundingBox&			GetBox() const;

		// Inquirers
		bool IsEdge() const;
		bool IsLooped() const;

		// Utilities
		Vector3		 GetPointAtChainDistance(float chainDist) const;
		unsigned int GetSegmentIDAtChainDistance(float chainDist) const;

		void Update(const std::vector<Vector3>& points, int roomNumber);
		void AttachPlayer(ItemInfo& itemNumber);
		void DetachPlayer(ItemInfo& itemNumber);

		void DrawDebug() const;

	private:
		// Helpers
		float NormalizeChainDistance(float chainDist) const;
		void  CacheLength();
		void  CacheBox();
	};

	// TEMP
	std::vector<Attractor> GenerateSectorAttractors(const CollisionResult& pointColl);
}
