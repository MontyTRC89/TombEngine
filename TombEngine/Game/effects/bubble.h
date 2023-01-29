#pragma once

namespace TEN::Effects::Bubble
{
	constexpr auto BUBBLE_COUNT_MAX = 1024;

	enum BubbleFlags
	{
		Large		  = (1 << 0),
		Clump		  = (1 << 1),
		HighAmplitude = (1 << 2)
	};

	struct Bubble
	{
		unsigned int SpriteIndex = 0;

		Vector3 Position	  = Vector3::Zero;
		Vector3 PositionBase  = Vector3::Zero;
		int		RoomNumber	  = 0;
		short	Orientation2D = 0;

		Vector4 Color		 = Vector4::Zero;
		Vector4 ColorStart	 = Vector4::Zero;
		Vector4 ColorEnd	 = Vector4::Zero;

		Vector3 Inertia		 = Vector3::Zero;
		Vector3 Amplitude	 = Vector3::Zero;
		Vector3 WavePeriod	 = Vector3::Zero;
		Vector3 WaveVelocity = Vector3::Zero;

		Vector2 Scale	   = Vector2::Zero;
		Vector2 ScaleMax   = Vector2::Zero;
		Vector2 ScaleMin   = Vector2::Zero;

		float Life				  = 0.0f;
		float Velocity			  = 0.0f;
		float OscillationPeriod	  = 0.0f;
		float OscillationVelocity = 0.0f;
		short Rotation			  = 0;
	};

	extern std::deque<Bubble> Bubbles;

	void SpawnBubble(const Vector3& pos, int roomNumber, const Vector3& inertia, int flags);
	void SpawnBubble(const Vector3& pos, int roomNumber, int flags);

	void UpdateBubbles();
	void ClearBubbles();
}
