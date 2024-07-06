#include "framework.h"
#include "Math/Objects/BezierCurve2D.h"

namespace TEN::Math
{
	const BezierCurve2D BezierCurve2D::Linear	 = BezierCurve2D(Vector2::Zero, Vector2::One, Vector2::Zero, Vector2::One);
	const BezierCurve2D BezierCurve2D::EaseIn	 = BezierCurve2D(Vector2::Zero, Vector2::One, Vector2(0.25f, 0.0f), Vector2::One);
	const BezierCurve2D BezierCurve2D::EaseOut	 = BezierCurve2D(Vector2::Zero, Vector2::One, Vector2::Zero, Vector2(0.75f, 1.0f));
	const BezierCurve2D BezierCurve2D::EaseInOut = BezierCurve2D(Vector2::Zero, Vector2::One, Vector2(0.25f, 0.0f), Vector2(0.75f, 1.0f));

	BezierCurve2D::BezierCurve2D(const Vector2& start, const Vector2& end, const Vector2& startHandle, const Vector2& endHandle)
	{
		_controlPoints[0] = start;
		_controlPoints[1] = startHandle;
		_controlPoints[2] = endHandle;
		_controlPoints[3] = end;
	}

	Vector2 BezierCurve2D::GetPoint(float alpha) const
	{
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		// De Casteljau interpolation.
		auto points = _controlPoints;
		for (int i = 1; i < _controlPoints.size(); i++)
		{
			for (int j = 0; j < (_controlPoints.size() - i); j++)
				points[j] = Vector2::Lerp(points[j], points[j + 1], alpha);
		}

		return points.front();
	}

	float BezierCurve2D::GetY(float x) const
	{
		constexpr auto TOLERANCE = EPSILON * 200;

		x = std::clamp(x, 0.0f, _controlPoints.back().x);

		// Directly return Y for exact endpoint.
		if (x <= TOLERANCE)
		{
			return _controlPoints.front().y;
		}
		else if (x >= (_controlPoints.back().x - TOLERANCE))
		{
			return _controlPoints.back().y;
		}

		float low = 0.0f;
		float high = 1.0f;

		// Binary search for approximate Y.
		float alpha = 0.5f;
		while ((high - low) > TOLERANCE)
		{
			alpha = (low + high) / 2;
			if (GetPoint(alpha).x < x)
			{
				low = alpha;
			}
			else
			{
				high = alpha;
			}
		}

		return GetPoint(alpha).y;
	}
}
