#include "framework.h"
#include "Math/Objects/Pose.h"

#include "Math/Geometry.h"
#include "Math/Objects/EulerAngles.h"
#include "Math/Objects/Vector3i.h"

using namespace TEN::Math;

//namespace TEN::Math
//{
	const Pose Pose::Zero = Pose(Vector3i::Zero, EulerAngles::Zero);

	Pose::Pose()
	{
	}

	Pose::Pose(const Vector3i& pos)
	{
		Position = pos;
	}

	Pose::Pose(int xPos, int yPos, int zPos)
	{
		Position = Vector3i(xPos, yPos, zPos);
	}

	Pose::Pose(const EulerAngles& orient)
	{
		Orientation = orient;
	}

	Pose::Pose(short xOrient, short yOrient, short zOrient)
	{
		Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	Pose::Pose(const Vector3i& pos, const EulerAngles& orient)
	{
		Position = pos;
		Orientation = orient;
	}

	Pose::Pose(const Vector3i& pos, short xOrient, short yOrient, short zOrient)
	{
		Position = pos;
		Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	Pose::Pose(int xPos, int yPos, int zPos, const EulerAngles& orient)
	{
		Position = Vector3i(xPos, yPos, zPos);
		Orientation = orient;
	}

	Pose::Pose(int xPos, int yPos, int zPos, short xOrient, short yOrient, short zOrient)
	{
		Position = Vector3i(xPos, yPos, zPos);
		Orientation = EulerAngles(xOrient, yOrient, zOrient);
	}

	void Pose::Translate(short headingAngle, float forward, float down, float right)
	{
		Position = Geometry::TranslatePoint(Position, headingAngle, forward, down, right);
	}

	void Pose::Translate(const EulerAngles& orient, const Vector3i& offset)
	{
		this->Position = Geometry::TranslatePoint(this->Position, orient, offset);
	}

	void Pose::Translate(const EulerAngles& orient, float dist)
	{
		Position = Geometry::TranslatePoint(Position, orient, dist);
	}

	void Pose::Translate(const Vector3& dir, float dist)
	{
		Position = Geometry::TranslatePoint(Position, dir, dist);
	}

	void Pose::InterpolateConstant(const Pose& toPose, float velocity, short turnRate)
	{
		// Translate position.
		float distance = Vector3i::Distance(Position, toPose.Position);
		if (distance <= velocity)
			this->Position = toPose.Position;
		else
		{
			auto direction = toPose.Position.ToVector3() - Position.ToVector3();
			this->Translate(direction, velocity);
		}

		// Interpolate orientation.
		this->Orientation.InterpolateConstant(toPose.Orientation, turnRate);
	}

	bool Pose::operator ==(const Pose& pose) const
	{
		return ((Position == pose.Position) && (Orientation == pose.Orientation));
	}

	bool Pose::operator !=(const Pose& pose) const
	{
		return !(*this == pose);
	}
//}
