#pragma once

namespace TEN::Math
{
	class BezierCurve2D
	{
	private:
		// Constants

		static constexpr auto CONTROL_POINT_COUNT = 4;

		// Members

		std::array<Vector2, CONTROL_POINT_COUNT> _controlPoints = {};

	public:
		// Presets

		static const BezierCurve2D Linear;
		static const BezierCurve2D EaseIn;
		static const BezierCurve2D EaseOut;
		static const BezierCurve2D EaseInOut;

		// Constructors

		BezierCurve2D() = default;
		BezierCurve2D(const Vector2& start, const Vector2& end, const Vector2& startHandle, const Vector2& endHandle);

		// Getters

		Vector2& GetStart();
		Vector2& GetEnd();
		Vector2& GetStartHandle();
		Vector2& GetEndHandle();

		Vector2 GetPoint(float alpha) const;
		float	GetY(float x) const;
	};
}
