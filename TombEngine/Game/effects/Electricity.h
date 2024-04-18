#pragma once

namespace TEN::Effects::Electricity
{
	constexpr auto ELECTRICITY_KNOTS_SIZE  = 6;
	constexpr auto ELECTRICITY_BUFFER_SIZE = 2048;

	enum class ElectricityFlags
	{
		Spline	 = (1 << 0),
		MoveEnd	 = (1 << 1),
		ThinOut	 = (1 << 2),
		ThinIn	 = (1 << 3),
		SparkEnd = (1 << 4)
	};

	// TODO: Make sense of this struct.
	struct Electricity
	{
		Vector3 pos1;
		Vector3 pos2;
		Vector3 pos3;
		Vector3 pos4;
		std::array<Vector3, 3> interpolation = {};

		byte r;
		byte g;
		byte b;

		float life;
		float sLife;
		float amplitude;
		float sAmplitude;
		float width;
		unsigned int segments;

		int segmentSize;
		int direction;
		int rotation;
		int type;
		int flags;

		Vector3 oldPos1;
		Vector3 oldPos2;
		Vector3 oldPos3;
		Vector3 oldPos4;
		byte oldR;
		byte oldG;
		byte oldB;
		float oldLife;

		void StoreInterpolationData()
		{
			oldPos1 = pos1;
			oldPos2 = pos2;
			oldPos3 = pos3;
			oldPos4 = pos4;
			oldR = r;
			oldG = g;
			oldB = b;
			oldLife = life;
		}
	};

	struct HelicalLaser
	{
		unsigned int NumSegments = 0;

		Vector3 Origin = Vector3::Zero;
		Vector3 Target = Vector3::Zero;
		short	Orientation2D = 0;
		Vector3 LightPosition = Vector3::Zero; // TODO: Use light cone instead?
		Vector4 Color = Vector4::Zero;

		float Life = 0.0f;
		float Radius = 0.0f;
		float Length = 0.0f;
		float LengthEnd = 0.0f;
		float Opacity = 0.0f;
		short Rotation = 0;

		Vector3 OldOrigin = Vector3::Zero;
		Vector3 OldTarget = Vector3::Zero;
		float OldLife = 0.0f;
		float OldRadius = 0.0f;
		float OldLength = 0.0f;
		float OldOpacity = 0.0f;
		Vector4 OldColor = Vector4::Zero;
		short OldOrientation2D = 0;

		void StoreInterpolationData()
		{
			OldOrigin = Origin;
			OldTarget = Target;
			OldLife = Life;
			OldRadius = Radius;
			OldLength = Length;
			OldOpacity = Opacity;
			OldColor = Color;
			OldOrientation2D = Orientation2D;
		}
	};

	extern std::vector<Electricity>	 ElectricityArcs;
	extern std::vector<HelicalLaser> HelicalLasers;

	extern std::array<Vector3, ELECTRICITY_KNOTS_SIZE>	ElectricityKnots;
	extern std::array<Vector3, ELECTRICITY_BUFFER_SIZE> ElectricityBuffer;

	void SpawnElectricity(const Vector3& origin, const Vector3& target, float amplitude, byte r, byte g, byte b, float life, int flags, float width, unsigned int numSegments);
	void SpawnElectricityGlow(const Vector3& pos, float scale, byte r, byte g, byte b);
	void SpawnHelicalLaser(const Vector3& origin, const Vector3& target);

	void UpdateElectricityArcs();
	void UpdateHelicalLasers();

	void CalculateElectricitySpline(const Electricity& arc, const std::array<Vector3, ELECTRICITY_KNOTS_SIZE>& knots, std::array<Vector3, ELECTRICITY_BUFFER_SIZE>& buffer);
	void CalculateHelixSpline(const HelicalLaser& laser, std::array<Vector3, ELECTRICITY_KNOTS_SIZE>& knots, std::array<Vector3, ELECTRICITY_BUFFER_SIZE>& buffer);
}
