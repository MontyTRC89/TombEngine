#include "framework.h"
#include "Math/Containers/BoundingBox.h"

#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/PoseData.h"

//namespace TEN::Math
//{
	int BOUNDING_BOX::Height() const
	{
		return abs(Y2 - Y1);
	}

	void BOUNDING_BOX::RotNoPersp(const EulerAngles& orient, const BOUNDING_BOX& bounds)
	{
		auto world = orient.ToRotationMatrix();
		auto bMin = Vector3(bounds.X1, bounds.Y1, bounds.Z1);
		auto bMax = Vector3(bounds.X2, bounds.Y2, bounds.Z2);

		bMin = Vector3::Transform(bMin, world);
		bMax = Vector3::Transform(bMax, world);

		this->X1 = bMin.x;
		this->X2 = bMax.x;
		this->Y1 = bMin.y;
		this->Y2 = bMax.y;
		this->Z1 = bMin.z;
		this->Z2 = bMax.z;
	}

	BoundingOrientedBox BOUNDING_BOX::ToDXBoundingOrientedBox(const PoseData& pose) const
	{
		return this->ToDXBoundingOrientedBox(pose.Position.ToVector3(), pose.Orientation.ToQuaternion());
	}

	BoundingOrientedBox BOUNDING_BOX::ToDXBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const
	{
		auto boxCenter = Vector3((X2 + X1) / 2.0f, (Y2 + Y1) / 2.0f, (Z2 + Z1) / 2.0f);
		auto boxExtent = Vector3((X2 - X1) / 2.0f, (Y2 - Y1) / 2.0f, (Z2 - Z1) / 2.0f);

		BoundingOrientedBox result;
		BoundingOrientedBox(boxCenter, boxExtent, Vector4::UnitY).Transform(result, 1.0f, orient, pos);
		return result;
	}

	BOUNDING_BOX BOUNDING_BOX::operator +(const PoseData& pose) const
	{
		auto newBox = *this;
		newBox.X1 += pose.Position.x;
		newBox.X2 += pose.Position.x;
		newBox.Y1 += pose.Position.y;
		newBox.Y2 += pose.Position.y;
		newBox.Z1 += pose.Position.z;
		newBox.Z2 += pose.Position.z;
		return newBox;
	}

	BOUNDING_BOX BOUNDING_BOX::operator *(float scale) const
	{
		auto newBox = *this;
		newBox.X1 *= scale;
		newBox.X2 *= scale;
		newBox.Y1 *= scale;
		newBox.Y2 *= scale;
		newBox.Z1 *= scale;
		newBox.Z2 *= scale;
		return newBox;
	}
//}
