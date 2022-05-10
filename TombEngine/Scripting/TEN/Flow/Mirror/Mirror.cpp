#include "framework.h"
#include "Mirror.h"

/***
A mirror effect.
As seen in TR4's Coastal Ruins and Sacred Lake levels.

__Not currently implemented.__ 

@tenclass Flow.Mirror
@pragma nostrip
*/

void Mirror::Register(sol::table& parent)
{
	parent.new_usertype<Mirror>("Mirror",
		sol::constructors<Mirror(short, int, int, int, int)>(),
		"room", &Mirror::Room,
		"startX", &Mirror::StartX,
		"endX", &Mirror::EndX,
		"startZ", &Mirror::StartZ,
		"endZ", &Mirror::EndZ
		);
}

Mirror::Mirror(short room, int startX, int endX, int startZ, int endZ)
{
	Room = room;
	StartX = startX;
	EndX = endX;
	StartZ = startZ;
	EndZ = endZ;
}
