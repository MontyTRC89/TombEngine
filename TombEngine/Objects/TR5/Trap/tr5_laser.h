#pragma once

constexpr auto LASER_BEAM_COUNT = 3;

struct LaserStructInfo
{
	std::array<Vector3, LASER_BEAM_COUNT> vert1 = {};
	std::array<Vector3, LASER_BEAM_COUNT> vert2 = {};
	std::array<Vector3, LASER_BEAM_COUNT> vert3 = {};
	std::array<Vector3, LASER_BEAM_COUNT> vert4 = {};
	Vector4 Color = Vector4::Zero;
	short Rand[18];
	int life = INFINITE;
};

extern std::vector<LaserStructInfo> Lasers;

void ControlLasers(short itemNumber);
void InitializeLasers(short itemNumber);
void UpdateLasers();
void RemoveLasers();
