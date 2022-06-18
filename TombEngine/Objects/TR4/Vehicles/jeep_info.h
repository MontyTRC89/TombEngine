#pragma once

namespace TEN::Entities::Vehicles
{
	struct JeepInfo
	{
		short jeepTurn = 0;
		short rot1 = 0;
		short rot2 = 0;
		short rot3 = 0;
		short rot4 = 0;
		short momentumAngle = 0;
		short extraRotation = 0;

		int velocity = 0;
		int fallSpeed = 0;

		int revs = 0;
		short engineRevs = 0;
		int pitch = 0;
		short trackMesh = 0;

		short flags = NULL;

		short unknown0 = 0;
		short unknown1 = 0;
		short unknown2 = 0;
	};
}
