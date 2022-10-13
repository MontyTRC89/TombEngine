#include "framework.h"
#include "Math/Containers/BoundingBox.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/Pose.h"

//namespace TEN::Math
//{
	const BOUNDING_BOX BOUNDING_BOX::Zero = BOUNDING_BOX(0, 0, 0, 0, 0, 0);

	BOUNDING_BOX::BOUNDING_BOX()
	{
	}

	BOUNDING_BOX::BOUNDING_BOX(float x1, float x2, float y1, float y2, float z1, float z2)
	{
		this->X1 = (int)round(x1);
		this->X2 = (int)round(x2);
		this->Y1 = (int)round(y1);
		this->Y2 = (int)round(y2);
		this->Z1 = (int)round(z1);
		this->Z2 = (int)round(z2);
	}

	BOUNDING_BOX::BOUNDING_BOX(ItemInfo* item, bool isAccurate)
	{
		int rate = 0;
		AnimFrame* framePtr[2];

		int frac = GetFrame(item, framePtr, rate);
		if (frac == 0 || !isAccurate)
			*this = framePtr[0]->boundingBox;
		else
			*this = framePtr[0]->boundingBox + ((framePtr[1]->boundingBox - framePtr[0]->boundingBox) * (frac / rate));
	}

	int BOUNDING_BOX::GetWidth() const
	{
		return abs(X2 - X1);
	}

	int BOUNDING_BOX::GetHeight() const
	{
		return abs(Y2 - Y1);
	}

	int BOUNDING_BOX::GetDepth() const
	{
		return abs(Z2 - Z1);
	}

	void BOUNDING_BOX::RotNoPersp(const EulerAngles& orient, const BOUNDING_BOX& bounds)
	{
		auto world = orient.ToRotationMatrix();
		auto bMin = Vector3(bounds.X1, bounds.Y1, bounds.Z1);
		auto bMax = Vector3(bounds.X2, bounds.Y2, bounds.Z2);

		bMin = Vector3::Transform(bMin, world);
		bMax = Vector3::Transform(bMax, world);

		this->X1 = (int)round(bMin.x);
		this->X2 = (int)round(bMax.x);
		this->Y1 = (int)round(bMin.y);
		this->Y2 = (int)round(bMax.y);
		this->Z1 = (int)round(bMin.z);
		this->Z2 = (int)round(bMax.z);
	}

	BoundingOrientedBox BOUNDING_BOX::ToBoundingOrientedBox(const Pose& pose) const
	{
		return this->ToBoundingOrientedBox(pose.Position.ToVector3(), pose.Orientation.ToQuaternion());
	}

	BoundingOrientedBox BOUNDING_BOX::ToBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const
	{
		auto boxCenter = Vector3(X2 + X1, Y2 + Y1, Z2 + Z1) / 2.0f;
		auto boxExtents = Vector3(X2 - X1, Y2 - Y1, Z2 - Z1) / 2.0f;

		BoundingOrientedBox box;
		BoundingOrientedBox(boxCenter, boxExtents, Vector4::UnitY).Transform(box, 1.0f, orient, pos);
		return box;
	}

	BOUNDING_BOX BOUNDING_BOX::operator +(const BOUNDING_BOX& bounds) const
	{
		return BOUNDING_BOX(
			X1 + bounds.X1, X2 + bounds.X2,
			Y1 + bounds.Y1, Y2 + bounds.Y2,
			Z1 + bounds.Z1, Z2 + bounds.Z2
		);
	}

	BOUNDING_BOX BOUNDING_BOX::operator +(const Pose& pose) const
	{
		return BOUNDING_BOX(
			X1 + pose.Position.x, X2 + pose.Position.x,
			Y1 + pose.Position.y, Y2 + pose.Position.y,
			Z1 + pose.Position.z, Z2 + pose.Position.z
		);
	}

	BOUNDING_BOX BOUNDING_BOX::operator -(const BOUNDING_BOX& bounds) const
	{
		return BOUNDING_BOX(
			X1 - bounds.X1, X2 - bounds.X2,
			Y1 - bounds.Y1, Y2 - bounds.Y2,
			Z1 - bounds.Z1, Z2 - bounds.Z2
		);
	}

	BOUNDING_BOX BOUNDING_BOX::operator -(const Pose& pose) const
	{
		return BOUNDING_BOX(
			X1 - pose.Position.x, X2 - pose.Position.x,
			Y1 - pose.Position.y, Y2 - pose.Position.y,
			Z1 - pose.Position.z, Z2 - pose.Position.z
		);
	}

	BOUNDING_BOX BOUNDING_BOX::operator *(float scale) const
	{
		return BOUNDING_BOX(
			X1 * scale, X2 * scale,
			Y1 * scale, Y2 * scale,
			Z1 * scale, Z2 * scale
		);
	}
//}
