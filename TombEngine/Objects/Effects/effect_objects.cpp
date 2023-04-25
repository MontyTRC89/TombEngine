#include "framework.h"
#include "Objects/Effects/effect_objects.h"

#include "Objects/Effects/flame_emitters.h"
#include "Objects/Effects/enemy_missile.h"
#include "Specific/setup.h"

using namespace TEN::Entities::Effects;

void InitializeEffectsObjects()
{
	ObjectInfo* obj;

	// Flame is always loaded
	obj = &Objects[ID_FLAME];
	{
		obj->control = nullptr;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER];
	if (obj->loaded)
	{
		obj->Initialize = InitializeFlameEmitter;
		obj->collision = FlameEmitterCollision;
		obj->control = FlameEmitterControl;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER2];
	if (obj->loaded)
	{
		obj->Initialize = InitializeFlameEmitter2;
		obj->collision = FlameEmitterCollision;
		obj->control = FlameEmitter2Control;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_FLAME_EMITTER3];
	if (obj->loaded)
	{
		obj->Initialize = InitializeFlameEmitter3;
		obj->control = FlameEmitter3Control;
		obj->drawRoutine = nullptr;
		obj->usingDrawAnimatingItem = false;
	}

	obj = &Objects[ID_ENERGY_BUBBLES];
	{
		obj->Initialize = nullptr;
		obj->collision = nullptr;
		obj->control = ControlEnemyMissile;
	}
}