#pragma once

namespace TEN::Hud
{
	class SpeedometerController
	{
	private:
		// Constants

		static constexpr auto LIFE_MAX = 0.75f;

		// Members

		bool _hasValueUpdated = false;

		float _value		= 0.0f;
		short _pointerAngle = 0;
		float _opacity		= 0.0f;
		float _life			= 0.0f;

	public:
		// Utilities

		void UpdateValue(float value);

		void Update();
		void Draw() const;
		void Clear();

		void DrawDebug() const;
	};
}
