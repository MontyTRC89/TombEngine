#include "framework.h"
#include "Renderer11.h"
#include "draw.h"
#include "lara.h"
#include "effect.h"
#include "camera.h"
#include "level.h"
#include "setup.h"

void Renderer11::collectRooms()
{
	short baseRoomIndex = Camera.pos.roomNumber;

	for (int i = 0; i < NumberRooms; i++)
	{
		m_rooms[i].Visited = false;
		m_rooms[i].LightsToDraw.clear();
	}

	Vector4 vp = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);

	getVisibleRooms(-1, baseRoomIndex, &vp, false, 0);
}

void Renderer11::collectItems(short roomNumber)
{
	if (m_rooms.size() <= roomNumber) {
		return;
	}
	RendererRoom& const room = m_rooms[roomNumber];

	ROOM_INFO* r = room.Room;

	short itemNum = NO_ITEM;
	for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = Items[itemNum].nextItem)
	{
		//printf("ItemNum: %d, NextItem: %d\n", itemNum, Items[itemNum].nextItem);

		ITEM_INFO* item = &Items[itemNum];

		if (item->objectNumber == ID_LARA && itemNum == Items[itemNum].nextItem)
			break;

		if (item->objectNumber == ID_LARA)
			continue;

		if (item->status == ITEM_INVISIBLE)
			continue;

		if (m_moveableObjects[item->objectNumber] == NULL)
			continue;
		RendererItem * newItem = &m_items[itemNum];
		short* bounds = GetBoundsAccurate(item);
		Vector3 min = (Vector3(bounds[0], bounds[2], bounds[4])) + Vector3(item->pos.xPos,item->pos.yPos,item->pos.zPos);
		Vector3 max = (Vector3(bounds[1], bounds[3], bounds[5])) + Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if(!frustum.AABBInFrustum(min,max))
			continue;
		newItem->Item = item;
		newItem->Id = itemNum;
		newItem->NumMeshes = Objects[item->objectNumber].nmeshes;
		newItem->Translation = Matrix::CreateTranslation(item->pos.xPos, item->pos.yPos, item->pos.zPos);
		newItem->Rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(item->pos.yRot),
			TO_RAD(item->pos.xRot),
			TO_RAD(item->pos.zRot));
		newItem->Scale = Matrix::CreateScale(1.0f);
		newItem->World = newItem->Rotation * newItem->Translation;
		collectLightsForItem(item->roomNumber, newItem);
		m_itemsToDraw.push_back(newItem);
	}
}

void Renderer11::collectStatics(short roomNumber)
{
	if (m_rooms.size() <= roomNumber) {
		return;
	}
	RendererRoom& const room = m_rooms[roomNumber];
	ROOM_INFO* r = room.Room;
	if (r->numMeshes <= 0)
		return;
	int numStatics = r->numMeshes;
	for (int i = 0; i < numStatics; i++)
	{
		MESH_INFO* mesh = &r->mesh[i];
		RendererStatic* newStatic = &room.Statics[i];
		StaticInfo* staticInfo = &StaticObjects[mesh->staticNumber];
		Vector3 min = Vector3(staticInfo->xMinc, staticInfo->yMinc, staticInfo->zMinc);
		Vector3 max = Vector3(staticInfo->xMaxc, staticInfo->yMaxc, staticInfo->zMaxc);
		min += Vector3(mesh->x, mesh->y, mesh->z);
		max += Vector3(mesh->x, mesh->y, mesh->z);
		if (!frustum.AABBInFrustum(min, max))
			continue;
		Matrix rotation = Matrix::CreateRotationY(TO_RAD(mesh->yRot));
		Vector3 translation = Vector3(mesh->x, mesh->y, mesh->z);
		newStatic->Mesh = mesh;
		newStatic->RoomIndex = roomNumber;
		newStatic->World = rotation * Matrix::CreateTranslation(translation);
		m_staticsToDraw.push_back(newStatic);
	}
}

void Renderer11::collectLightsForEffect(short roomNumber, RendererEffect * effect)
{
	effect->Lights.clear();
	if (m_rooms.size() <= roomNumber) {
		return;
	}
	RendererRoom& const room = m_rooms[roomNumber];

	ROOM_INFO* r = room.Room;

	if (r->numLights <= 0)
		return;

	m_tempItemLights.clear();

	Vector3 itemPosition = Vector3(effect->Effect->pos.xPos, effect->Effect->pos.yPos, effect->Effect->pos.zPos);

	// Dynamic lights have the priority
	for (int i = 0; i < m_dynamicLights.size(); i++)
	{
		RendererLight* light = m_dynamicLights[i];

		Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

		float distance = (itemPosition - lightPosition).Length();
		if (distance > light->Out)
			continue;

		m_tempItemLights.push_back(light);
	}

	int numLights = room.Lights.size();

	m_shadowLight = NULL;
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;

	for (int j = 0; j < numLights; j++)
	{
		RendererLight* light = &room.Lights[j];

		// Check only lights different from sun
		if (light->Type == LIGHT_TYPE_SUN)
		{
			// Sun is added without checks
		}
		else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the out radius
			if (distance > light->Out)
				continue;

			// If Lara, try to collect shadow casting light
			if (effect->Effect->objectNumber == ID_LARA)
			{
				float attenuation = 1.0f - distance / light->Out;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
		}
		else if (light->Type == LIGHT_TYPE_SPOT)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the range
			if (distance > light->Range)
				continue;
		}
		else
		{
			// Invalid light type
			continue;
		}

		m_tempItemLights.push_back(light);
	}

	for (int i = 0; i < min(MAX_LIGHTS_PER_ITEM, m_tempItemLights.size()); i++)
	{
		effect->Lights.push_back(m_tempItemLights[i]);
	}
}

void Renderer11::collectLightsForItem(short roomNumber, RendererItem * item)
{
	item->Lights.clear();
	if (m_rooms.size() <= roomNumber) {
		return;
	}
	RendererRoom& const room = m_rooms[roomNumber];

	ROOM_INFO* r = room.Room;

	if (r->numLights <= 0)
		return;

	m_tempItemLights.clear();

	Vector3 itemPosition = Vector3(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos);

	// Dynamic lights have the priority
	for (int i = 0; i < m_dynamicLights.size(); i++)
	{
		RendererLight* light = m_dynamicLights[i];

		Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

		float distance = (itemPosition - lightPosition).Length();
		if (distance > light->Out)
			continue;

		m_tempItemLights.push_back(light);
	}

	int numLights = room.Lights.size();

	m_shadowLight = NULL;
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;

	for (int j = 0; j < numLights; j++)
	{
		RendererLight* light = &room.Lights[j];

		// Check only lights different from sun
		if (light->Type == LIGHT_TYPE_SUN)
		{
			// Sun is added without checks
		}
		else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the out radius
			if (distance > light->Out)
				continue;

			// If Lara, try to collect shadow casting light
			if (item->Item->objectNumber == ID_LARA)
			{
				float attenuation = 1.0f - distance / light->Out;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
		}
		else if (light->Type == LIGHT_TYPE_SPOT)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the range
			if (distance > light->Range)
				continue;

			// If Lara, try to collect shadow casting light
			if (item->Item->objectNumber == ID_LARA)
			{
				float attenuation = 1.0f - distance / light->Range;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
		}
		else
		{
			// Invalid light type
			continue;
		}

		m_tempItemLights.push_back(light);
	}

	for (int i = 0; i < min(MAX_LIGHTS_PER_ITEM, m_tempItemLights.size()); i++)
	{
		item->Lights.push_back(m_tempItemLights[i]);
	}

	if (item->Item->objectNumber == ID_LARA)
	{
		m_shadowLight = brightestLight;
	}
}

void Renderer11::collectLightsForRoom(short roomNumber)
{
	if (m_rooms.size() <= roomNumber) {
		return;
	}
	RendererRoom& const room = m_rooms[roomNumber];
	ROOM_INFO* r = &Rooms[roomNumber];

	int numLights = room.Lights.size();

	// Collect dynamic lights for rooms
	for (int i = 0; i < m_dynamicLights.size(); i++)
	{
		RendererLight* light = m_dynamicLights[i];

		Vector3 boxMin = Vector3(r->x - WALL_SIZE, -r->minfloor, r->z - WALL_SIZE);
		Vector3 boxMax = Vector3(r->x + r->xSize * WALL_SIZE, -r->maxceiling, r->z + r->ySize * WALL_SIZE);
		Vector3 center = Vector3(light->Position.x, -light->Position.y, light->Position.z);

		if (sphereBoxIntersection(boxMin, boxMax, center, light->Out))
			room.LightsToDraw.push_back(light);
	}
}

void Renderer11::prepareLights()
{
	// Add dynamic lights
	for (int i = 0; i < m_dynamicLights.size(); i++)
		m_lightsToDraw.push_back(m_dynamicLights[i]);

	// Now I have a list full of draw. Let's sort them.
	//std::sort(m_lightsToDraw.begin(), m_lightsToDraw.end(), SortLightsFunction);
	//m_lightsToDraw.Sort(SortLightsFunction);

	// Let's draw first 32 lights
	//if (m_lightsToDraw.size() > 32)
	//	m_lightsToDraw.resize(32);

	// Now try to search for a shadow caster, using Lara as reference
	RendererRoom & const room = m_rooms[LaraItem->roomNumber];

	// Search for the brightest light. We do a simple version of the classic calculation done in pixel shader.
	RendererLight * brightestLight = NULL;
	float brightest = 0.0f;

	// Try room lights
	if (room.Lights.size() != 0)
	{
		for (int j = 0; j < room.Lights.size(); j++)
		{
			RendererLight* light = &room.Lights[j];

			Vector4 itemPos = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
			Vector4 lightVector = itemPos - light->Position;

			float distance = lightVector.Length();
			lightVector.Normalize();

			float intensity;
			float attenuation;
			float angle;
			float d;
			float attenuationRange;
			float attenuationAngle;

			switch ((int)light->Type)
			{
			case LIGHT_TYPES::LIGHT_TYPE_POINT:
				if (distance > light->Out || light->Out < 2048.0f)
					continue;

				attenuation = 1.0f - distance / light->Out;
				intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;

			case  LIGHT_TYPES::LIGHT_TYPE_SPOT:
				if (distance > light->Range)
					continue;

				attenuation = 1.0f - distance / light->Range;
				intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;
			}
		}
	}

	// If the brightest light is found, then fill the data structure. We ignore for now dynamic lights for shadows.
	m_shadowLight = brightestLight;
}

void Renderer11::collectEffects(short roomNumber)
{
	if (m_rooms.size() <= roomNumber) return;
	RendererRoom & const room = m_rooms[roomNumber];

	ROOM_INFO * r = room.Room;

	short fxNum = NO_ITEM;
	for (fxNum = r->fxNumber; fxNum != NO_ITEM; fxNum = Effects[fxNum].nextFx)
	{
		FX_INFO* fx = &Effects[fxNum];
		
		if (fx->objectNumber < 0)
			continue;

		ObjectInfo* obj = &Objects[fx->objectNumber];

		RendererEffect* newEffect = &m_effects[fxNum];

		newEffect->Effect = fx;
		newEffect->Id = fxNum;
		newEffect->World = Matrix::CreateFromYawPitchRoll(fx->pos.yRot, fx->pos.xPos, fx->pos.zPos) * Matrix::CreateTranslation(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
		newEffect->Mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(Meshes[(obj->nmeshes ? obj->meshIndex : fx->frameNumber)])];

		collectLightsForEffect(fx->roomNumber, newEffect);

		m_effectsToDraw.push_back(newEffect);

		short* mp = Meshes[(obj->nmeshes ? obj->meshIndex : fx->frameNumber)];
		short hhh = 0;
	}
}

void Renderer11::prepareCameraForFrame()
{
	// Set camera matrices
	m_stCameraMatrices.ViewProjection = ViewProjection;
	m_stCameraMatrices.Frame = GnFrameCounter;
	m_stCameraMatrices.CameraUnderwater = (Rooms[Camera.pos.roomNumber].flags & ENV_FLAG_WATER) != 0 ? 1: 0;
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);
	frustum.Update(View,Projection);
}

void Renderer11::ResetAnimations()
{
	for (int i = 0; i < NUM_ITEMS; i++)
	{
		m_items[i].DoneAnimations = false;
	}
}