#include "framework.h"
#include "Math/Containers/BoundingBox.h"

#include "Game/animation.h"
#include "Math/Containers/EulerAngles.h"
#include "Math/Containers/PoseData.h"

//namespace TEN::Math
//{
	const BOUNDING_BOX BOUNDING_BOX::Zero = BOUNDING_BOX(0, 0, 0, 0, 0, 0);

	BOUNDING_BOX::BOUNDING_BOX()
	{
	}

	BOUNDING_BOX::BOUNDING_BOX(int x1, int x2, int y1, int y2, int z1, int z2)
	{
		this->X1 = x1;
		this->X2 = x2;
		this->Y1 = y1;
		this->Y2 = y2;
		this->Z1 = z1;
		this->Z2 = z2;
	}
	
	BOUNDING_BOX::BOUNDING_BOX(ItemInfo* item, bool accurate)
	{
		int rate = 0;
		AnimFrame* framePtr[2];

		int frac = GetFrame(item, framePtr, &rate);
		if (frac == 0 || !accurate)
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

		this->X1 = bMin.x;
		this->X2 = bMax.x;
		this->Y1 = bMin.y;
		this->Y2 = bMax.y;
		this->Z1 = bMin.z;
		this->Z2 = bMax.z;
	}

	BoundingOrientedBox BOUNDING_BOX::ToBoundingOrientedBox(const PoseData& pose) const
	{
		return this->ToBoundingOrientedBox(pose.Position.ToVector3(), pose.Orientation.ToQuaternion());
	}

	BoundingOrientedBox BOUNDING_BOX::ToBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const
	{
		auto boxCenter = Vector3((X2 + X1) / 2.0f, (Y2 + Y1) / 2.0f, (Z2 + Z1) / 2.0f);
		auto boxExtent = Vector3((X2 - X1) / 2.0f, (Y2 - Y1) / 2.0f, (Z2 - Z1) / 2.0f);

		BoundingOrientedBox result;
		BoundingOrientedBox(boxCenter, boxExtent, Vector4::UnitY).Transform(result, 1.0f, orient, pos);
		return result;
	}

	BOUNDING_BOX BOUNDING_BOX::operator +(const BOUNDING_BOX& bounds) const
	{
		auto newBox = *this;
		newBox.X1 += bounds.X1;
		newBox.X2 += bounds.X2;
		newBox.Y1 += bounds.Y1;
		newBox.Y2 += bounds.Y2;
		newBox.Z1 += bounds.Z1;
		newBox.Z2 += bounds.Z2;
		return newBox;
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
	
	BOUNDING_BOX BOUNDING_BOX::operator -(const BOUNDING_BOX& bounds) const
	{
		auto newBox = *this;
		newBox.X1 -= bounds.X1;
		newBox.X2 -= bounds.X2;
		newBox.Y1 -= bounds.Y1;
		newBox.Y2 -= bounds.Y2;
		newBox.Z1 -= bounds.Z1;
		newBox.Z2 -= bounds.Z2;
		return newBox;
	}

	BOUNDING_BOX BOUNDING_BOX::operator -(const PoseData& pose) const
	{
		auto newBox = *this;
		newBox.X1 -= pose.Position.x;
		newBox.X2 -= pose.Position.x;
		newBox.Y1 -= pose.Position.y;
		newBox.Y2 -= pose.Position.y;
		newBox.Z1 -= pose.Position.z;
		newBox.Z2 -= pose.Position.z;
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
