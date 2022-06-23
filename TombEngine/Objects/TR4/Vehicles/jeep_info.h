#pragma once

namespace TEN::Entities::Vehicles
{
	struct JeepInfo
	{
		short jeepTurn;
		short rot1;
		short rot2;
		short rot3;
		short rot4;
		short momentumAngle;
		short extraRotation;

		int velocity;
		int fallSpeed;

		int revs;
		short engineRevs;
		int pitch;
		short trackMesh;

		short flags;

		short unknown0;
		short unknown1;
		short unknown2;
	};
}
