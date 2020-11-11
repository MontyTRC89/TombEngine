#include "framework.h"
#include "generic_bridge.h"
#include "floordata.h"
#include "level.h"

using namespace T5M::Floordata;

int GetOffset(short angle, int x, int z)
{
	const auto point = GetSectorPoint(x, z);
	auto vector = Vector2(point.x, point.y);
	const auto matrix = Matrix::CreateRotationZ(TO_RAD(angle));
	Vector2::Transform(vector, matrix, vector);
	return -vector.x;
}
