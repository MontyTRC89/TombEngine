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
		SetStart(start);
		SetEnd(end);
		SetStartHandle(startHandle);
		SetEndHandle(endHandle);
	}

	const Vector2& BezierCurve2D::GetStart() const
	{
		return _controlPoints[0];
	}

	const Vector2& BezierCurve2D::GetEnd() const
	{
		return _controlPoints[3];
	}

	const Vector2& BezierCurve2D::GetStartHandle() const
	{
		return _controlPoints[1];
	}

	const Vector2& BezierCurve2D::GetEndHandle() const
	{
		return _controlPoints[2];
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

		// Directly return Y for exact endpoint.
		if (x <= (GetStart().x + TOLERANCE))
		{
			return GetStart().y;
		}
		else if (x >= (GetEnd().x - TOLERANCE))
		{
			return GetEnd().y;
		}

		float low = 0.0f;
		float high = 1.0f;
		auto point = Vector2::Zero;

		// Binary search for approximate Y.
		float alpha = 0.5f;
		while ((high - low) > TOLERANCE)
		{
			alpha = (low + high) / 2;
			point = GetPoint(alpha);

			if (point.x < x)
			{
				low = alpha;
			}
			else
			{
				high = alpha;
			}
		}

		return point.y;
	}

	void BezierCurve2D::SetStart(const Vector2& point)
	{
		_controlPoints[0] = point;
	}

	void BezierCurve2D::SetEnd(const Vector2& point)
	{
		_controlPoints[3] = point;
	}

	void BezierCurve2D::SetStartHandle(const Vector2& point)
	{
		_controlPoints[1] = point;
	}

	void BezierCurve2D::SetEndHandle(const Vector2& point)
	{
		_controlPoints[2] = point;
	}
}
