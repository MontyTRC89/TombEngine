#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Effects.h"

#include "Game/effects/effects.h"
#include "Game/effects/Ripple.h"
#include "Game/Setup.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"

using namespace TEN::Effects::Ripple;

namespace TEN::Entities::Generic
{
	constexpr auto FRAMES_BETWEEN_RIPPLES = 8.0f;

	void DoPushableRipples(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//TODO: Enhace the effect to make the ripples increase their size through the time.
		if (pushable.StartPos.y != NO_HEIGHT && std::fmod(GameTimer, FRAMES_BETWEEN_RIPPLES) <= 0.0f)
		{
			SpawnRipple(Vector3(pushableItem.Pose.Position.x, pushable.WaterSurfaceHeight, pushableItem.Pose.Position.z), pushableItem.RoomNumber, GameBoundingBox(&pushableItem).GetWidth() + (GetRandomControl() & 15), (int)RippleFlags::SlowFade | (int)RippleFlags::LowOpacity);
		}
	}

	void DoPushableSplash(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		SplashSetup.y = pushable.WaterSurfaceHeight - 1;
		SplashSetup.x = pushableItem.Pose.Position.x;
		SplashSetup.z = pushableItem.Pose.Position.z;
		SplashSetup.splashPower = pushableItem.Animation.Velocity.y * 2;
		SplashSetup.innerRadius = 250;
		SetupSplash(&SplashSetup, pushableItem.RoomNumber);
	}

	void DoPushableBubbles(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//TODO
	}

	void FloatingItem(ItemInfo& item, float floatForce)
	{
		constexpr auto BOX_VOLUME_MIN = 512.0f;

		auto time = GameTimer + item.Animation.Velocity.y;

		// Calculate bounding box volume scaling factor.
		auto bounds = GameBoundingBox(&item);
		float boxVolume = bounds.GetWidth() * bounds.GetDepth() * bounds.GetHeight();
		float boxScale = std::sqrt(std::min(BOX_VOLUME_MIN, boxVolume)) / 32.0f;
		boxScale *= floatForce;

		float xOscillation = (std::sin(time * 0.05f) * 0.5f) * boxScale;
		float zOscillation = (std::sin(time * 0.1f) * 0.75f) * boxScale;

		short xAngle = ANGLE(xOscillation * 20.0f);
		short zAngle = ANGLE(zOscillation * 20.0f);
		item.Pose.Orientation = EulerAngles(xAngle, item.Pose.Orientation.y, zAngle);
	}

	void FloatingBridge(ItemInfo& item, float floatForce)
	{
		constexpr auto BOX_VOLUME_MIN = 512.0f;

		auto time = GameTimer + item.Animation.Velocity.y;

		// Calculate bounding box volume scaling factor.
		auto bounds = GameBoundingBox(&item);
		float boxVolume = bounds.GetWidth() * bounds.GetDepth() * bounds.GetHeight();
		float boxScale = std::sqrt(std::min(BOX_VOLUME_MIN, boxVolume)) / 32.0f;
		boxScale *= floatForce;

		// Vertical oscillation (up and down).
		float yOscillation = (std::sin(time * 0.2f) * 0.5f) * boxScale * 32;
		short yTranslation = static_cast<short>(yOscillation);

		item.Pose.Position.y += yTranslation;
	}
}