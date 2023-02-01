#pragma once

namespace TEN::Effects::Bubble
{
	enum class BubbleFlags
	{
		Large		  = (1 << 0),
		HighAmplitude = (1 << 1)
	};

	struct Bubble
	{
		unsigned int SpriteIndex = 0;

		Vector3 Position	 = Vector3::Zero;
		Vector3 PositionBase = Vector3::Zero;
		int		RoomNumber	 = 0;

		Vector4 Color	   = Vector4::Zero;
		Vector4 ColorStart = Vector4::Zero;
		Vector4 ColorEnd   = Vector4::Zero;

		Vector3 Inertia		 = Vector3::Zero;
		Vector3 Amplitude	 = Vector3::Zero;
		Vector3 WavePeriod	 = Vector3::Zero;
		Vector3 WaveVelocity = Vector3::Zero;

		Vector2 Scale	 = Vector2::Zero;
		Vector2 ScaleMax = Vector2::Zero;
		Vector2 ScaleMin = Vector2::Zero;

		float Life				  = 0.0f;
		float Gravity			  = 0.0f;
		float OscillationPeriod	  = 0.0f;
		float OscillationVelocity = 0.0f;
	};

	extern std::deque<Bubble> Bubbles;

	void SpawnBubble(const Vector3& pos, int roomNumber, float scale, float amplitude, const Vector3& inertia = Vector3::Zero);
	void SpawnBubble(const Vector3& pos, int roomNumber, int flags = 0, const Vector3& inertia = Vector3::Zero);
	void SpawnChaffBubble(const Vector3& pos, int roomNumber);

	void UpdateBubbles();
	void ClearBubbles();
}
