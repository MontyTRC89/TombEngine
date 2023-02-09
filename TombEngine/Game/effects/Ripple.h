#pragma once

namespace TEN::Effects::Ripple
{
	enum class RippleFlags
	{
		SlowFade		  = (1 << 0),
		LowOpacity		  = (1 << 1),
		OnGround		  = (1 << 2)
	};

	struct Ripple
	{
		unsigned int SpriteIndex = 0;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;
		Vector3 Normal	   = Vector3::Zero;
		Vector4 Color	   = Vector4::Zero;

		float Life			  = 0.0f;
		float LifeMax		  = 0.0f;
		float LifeFullOpacity = 0.0f;
		float LifeStartFading = 0.0f;
		float Scale			  = 0.0f;
		int	  Flags			  = 0;
	};

	extern std::deque<Ripple> Ripples;

	void SpawnRipple(const Vector3& pos, int roomNumber, float scale, int flags = 0, const Vector3& normal = Vector3::Down);

	void UpdateRipples();
	void ClearRipples();
}