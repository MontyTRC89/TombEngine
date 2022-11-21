#pragma once
#include "Game/effects/effects.h"
#include "Game/room.h"

namespace TEN::Effects::Bubble
{
	constexpr auto BUBBLE_NUM_MAX = 256;
	
	enum BubbleFlags
	{
		BigSize		  = (1 << 0),
		Clump		  = (1 << 1),
		HighAmplitude = (1 << 2)
	};

	struct Bubble
	{
		bool		 IsActive	 = false;
		unsigned int SpriteIndex = 0;

		Vector3 Position	 = Vector3::Zero;
		Vector3 PositionHome = Vector3::Zero;
		int		RoomNumber	 = NO_ROOM;
		Vector4 Color		 = Vector4::Zero;
		Vector4 ColorStart	 = Vector4::Zero;
		Vector4 ColorEnd	 = Vector4::Zero;

		Vector3 Amplitude	 = Vector3::Zero;
		Vector3 WavePeriod	 = Vector3::Zero;
		Vector3 WaveVelocity = Vector3::Zero;

		float Life	   = 0.0f;
		float Scale	   = 0.0f;
		float ScaleMax = 0.0f;
		float Velocity = 0.0f;
		float Rotation = 0.0f;
	};

	extern std::array<Bubble, BUBBLE_NUM_MAX> Bubbles;

	Bubble& GetFreeBubble();

	void SpawnBubble(const Vector3& pos, int roomNumber, int unk1, int unk2, int flags, int xv, int yv, int zv);

	void UpdateBubbles();
	void ClearBubbles();
}
