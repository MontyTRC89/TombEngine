#include "objlight.h"
#include "control.h"
#include "effect2.h"

void TriggerAlertLight(int x, int y, int z, int r, int g, int b, int angle, short room, int falloff)
{
	GAME_VECTOR source, target;

	source.x = x;
	source.y = y;
	source.z = z;
	GetFloor(x, y, z, &room);
	source.roomNumber = room;
	target.x = x + phd_sin(16 * angle);
	target.y = y;
	target.z = z + phd_cos(16 * angle);
	if (!LOS(&source, &target))
		TriggerDynamicLight(target.x, target.y, target.z, falloff, r, g, b);
}
