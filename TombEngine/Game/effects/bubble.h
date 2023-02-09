#pragma once

namespace TEN::Effects::Bubble
{
	enum class BubbleFlags
	{
		LargeScale	  = (1 << 0),
		HighAmplitude = (1 << 1)
	};

	struct Bubble
	{
		unsigned int SpriteIndex = 0;

		Vector3 Position	 = Vector3::Zero;
		Vector3 PositionBase = Vector3::Zero;
		int		RoomNumber	 = 0;

		Vector2 Size	= Vector2::Zero;
		Vector2 SizeMax = Vector2::Zero;
		Vector2 SizeMin = Vector2::Zero;

		Vector4 Color	   = Vector4::Zero;
		Vector4 ColorStart = Vector4::Zero;
		Vector4 ColorEnd   = Vector4::Zero;

		Vector3 Amplitude	 = Vector3::Zero;
		Vector3 WavePeriod	 = Vector3::Zero;
		Vector3 WaveVelocity = Vector3::Zero;

		float Life				  = 0.0f;
		float Gravity			  = 0.0f;
		float OscillationPeriod	  = 0.0f;
		float OscillationVelocity = 0.0f;
	};

	extern std::deque<Bubble> Bubbles;

	void SpawnBubble(const Vector3& pos, int roomNumber, float size, float amplitude);
	void SpawnBubble(const Vector3& pos, int roomNumber, int flags = 0);
	void SpawnDiveBubbles(const Vector3& pos, int roomNumber, unsigned int count);
	void SpawnChaffBubble(const Vector3& pos, int roomNumber);

	void UpdateBubbles();
	void ClearBubbles();
}
