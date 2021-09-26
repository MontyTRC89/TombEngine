#include "framework.h"
#include "Renderer11.h"
#include "animation.h"
#include "lara.h"
#include "effects\effects.h"
#include "camera.h"
#include "level.h"
#include "items.h"
#include "setup.h"

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using TEN::Memory::LinearArrayBuffer;
	using std::vector;
	void Renderer11::collectRooms(RenderView &renderView)
	{
		short baseRoomIndex = renderView.camera.RoomNumber;

		for (int i = 0; i < g_Level.Rooms.size(); i++) {
			m_rooms[i].Visited = false;
		}

		getVisibleObjects(-1, baseRoomIndex, renderView);
	}

	void Renderer11::collectItems(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber) {
			return;
		}
		RendererRoom &const room = m_rooms[roomNumber];

		ROOM_INFO *r = room.Room;

		short itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = g_Level.Items[itemNum].nextItem)
		{
			//printf("ItemNum: %d, NextItem: %d\n", itemNum, g_Level.Items[itemNum].nextItem);

			ITEM_INFO *item = &g_Level.Items[itemNum];

			if (item->objectNumber == ID_LARA && itemNum == g_Level.Items[itemNum].nextItem)
				break;

			if (item->objectNumber == ID_LARA)
				continue;

			if (item->status == ITEM_INVISIBLE)
				continue;

			if (!m_moveableObjects[item->objectNumber].has_value())
				continue;
			RendererItem *newItem = &m_items[itemNum];
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);
			Vector3 min = (Vector3(bounds->X1, bounds->Y1, bounds->Z1)) + Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos);
			Vector3 max = (Vector3(bounds->X2, bounds->Y2, bounds->Z2)) + Vector3(item->pos.xPos, item->pos.yPos, item->pos.zPos);
			if (!renderView.camera.frustum.AABBInFrustum(min, max))
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
			collectLightsForItem(item->roomNumber, newItem, renderView);
			renderView.itemsToDraw.push_back(newItem);
		}
	}

	void Renderer11::collectStatics(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber) {
			return;
		}
		RendererRoom &const room = m_rooms[roomNumber];
		ROOM_INFO *r = room.Room;
		if (r->mesh.size() <= 0)
			return;
		int numStatics = r->mesh.size();
		for (int i = 0; i < numStatics; i++)
		{
			MESH_INFO *mesh = &r->mesh[i];
			if (mesh->flags & StaticMeshFlags::SM_VISIBLE)
			{
				RendererStatic* newStatic = &room.Statics[i];
				STATIC_INFO* sinfo = &StaticObjects[mesh->staticNumber];
				Vector3 min = Vector3(sinfo->visibilityBox.X1, sinfo->visibilityBox.Y1, sinfo->visibilityBox.Z1);
				Vector3 max = Vector3(sinfo->visibilityBox.X2, sinfo->visibilityBox.Y2, sinfo->visibilityBox.Z2);
				min += Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
				max += Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
				if (!renderView.camera.frustum.AABBInFrustum(min, max))
					continue;
				Matrix rotation = Matrix::CreateRotationY(TO_RAD(mesh->pos.yRot));
				Vector3 translation = Vector3(mesh->pos.xPos, mesh->pos.yPos, mesh->pos.zPos);
				newStatic->Mesh = mesh;
				newStatic->RoomIndex = roomNumber;
				newStatic->World = rotation * Matrix::CreateTranslation(translation);
				renderView.staticsToDraw.push_back(newStatic);
			}
		}
	}

	void Renderer11::collectLightsForEffect(short roomNumber, RendererEffect *effect, RenderView &renderView)
	{
		/*
		effect->Lights.clear();
		if (m_rooms.size() < roomNumber) {
			return;
		}
		RendererRoom &const room = m_rooms[roomNumber];

		ROOM_INFO *r = room.Room;

		if (r->lights.size() <= 0)
			return;
		LinearArrayBuffer<RendererLight*, 8> tempLights;

		Vector3 itemPosition = Vector3(effect->Effect->pos.xPos, effect->Effect->pos.yPos, effect->Effect->pos.zPos);

		// Dynamic lights have the priority
		for (int i = 0; i < m_dynamicLights.size(); i++)
		{
			RendererLight *light = m_dynamicLights[i];

			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();
			if (distance > light->Out)
				continue;

			tempLights.push_back(light);
		}

		int numLights = room.Lights.size();

		m_shadowLight = NULL;
		RendererLight *brightestLight = NULL;
		float brightest = 0.0f;

		for (int j = 0; j < numLights; j++)
		{
			RendererLight *light = &room.Lights[j];

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
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

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

			tempLights.push_back(light);
		}

		for (int i = 0; i < std::min(static_cast<size_t>(MAX_LIGHTS_PER_ITEM), tempLights.size()); i++)
		{
			renderView.lightsToDraw.push_back(tempLights[i]);
		}
		*/
	}

	void Renderer11::collectLightsForItem(short roomNumber, RendererItem *item, RenderView &renderView)
	{
		item->Lights.clear();
		if (m_rooms.size() < roomNumber){
			return;
		}
		RendererRoom &const room = m_rooms[roomNumber];

		ROOM_INFO *r = room.Room;
		LinearArrayBuffer<RendererLight*, 8> tempLights;

		Vector3 itemPosition = Vector3(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos);

		// Dynamic lights have the priority
		for (int i = 0; i < m_dynamicLights.size(); i++)
		{
			RendererLight *light = m_dynamicLights[i];

			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();
			if (distance > light->Out)
				continue;

			tempLights.push_back(light);
		}

		int numLights = room.Lights.size();

		m_shadowLight = NULL;
		RendererLight *brightestLight = NULL;
		float brightest = 0.0f;

		for (int j = 0; j < numLights; j++)
		{
			RendererLight *light = &room.Lights[j];

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
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

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
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

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

			tempLights.push_back(light);
		}

		for (int i = 0; i < std::min(static_cast<size_t>(MAX_LIGHTS_PER_ITEM), tempLights.size()); i++)
		{
			item->Lights.push_back(tempLights[i]);
		}

		if (item->Item->objectNumber == ID_LARA)
		{
			m_shadowLight = brightestLight;
		}
	}

	void Renderer11::collectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber){
			return;
		}
		RendererRoom &const room = m_rooms[roomNumber];
		ROOM_INFO *r = &g_Level.Rooms[roomNumber];

		int numLights = room.Lights.size();

		// Collect dynamic lights for rooms
		for (int i = 0; i < m_dynamicLights.size(); i++)
		{
			RendererLight *light = m_dynamicLights[i];

			Vector3 boxMin = Vector3(r->x - WALL_SIZE, -r->minfloor, r->z - WALL_SIZE);
			Vector3 boxMax = Vector3(r->x + r->xSize * WALL_SIZE, -r->maxceiling, r->z + r->ySize * WALL_SIZE);
			Vector3 center = Vector3(light->Position.x, -light->Position.y, light->Position.z);

			if (sphereBoxIntersection(boxMin, boxMax, center, light->Out))
				renderView.lightsToDraw.push_back(light);
		}
	}

	void Renderer11::prepareLights(RenderView& view)
	{
		// Add dynamic lights
		for (int i = 0; i < m_dynamicLights.size(); i++)
			view.lightsToDraw.push_back(m_dynamicLights[i]);

		// Now I have a list full of draw. Let's sort them.
		//std::sort(m_lightsToDraw.begin(), m_lightsToDraw.end(), SortLightsFunction);
		//m_lightsToDraw.Sort(SortLightsFunction);

		// Let's draw first 32 lights
		//if (m_lightsToDraw.size() > 32)
		//	m_lightsToDraw.resize(32);

		// Now try to search for a shadow caster, using Lara as reference
		RendererRoom &const room = m_rooms[LaraItem->roomNumber];

		// Search for the brightest light. We do a simple version of the classic calculation done in pixel shader.
		RendererLight *brightestLight = NULL;
		float brightest = 0.0f;

		// Try room lights

		for (int j = 0; j < room.Lights.size(); j++)
		{
			RendererLight *light = &room.Lights[j];

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
				intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;

			case LIGHT_TYPES::LIGHT_TYPE_SPOT:
				if (distance > light->Range)
					continue;

				attenuation = 1.0f - distance / light->Range;
				intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;
			}
		}

		// If the brightest light is found, then fill the data structure. We ignore for now dynamic lights for shadows.
		m_shadowLight = brightestLight;
	}

	void Renderer11::collectEffects(short roomNumber, RenderView &renderView)
	{
		/*
		if (m_rooms.size() < roomNumber)
			return;
		RendererRoom &const room = m_rooms[roomNumber];

		ROOM_INFO *r = room.Room;

		short fxNum = NO_ITEM;
		for (fxNum = r->fxNumber; fxNum != NO_ITEM; fxNum = EffectList[fxNum].nextFx)
		{
			FX_INFO *fx = &EffectList[fxNum];

			if (fx->objectNumber < 0)
				continue;

			OBJECT_INFO *obj = &Objects[fx->objectNumber];

			RendererEffect *newEffect = &m_effects[fxNum];

			newEffect->Effect = fx;
			newEffect->Id = fxNum;
			newEffect->World = Matrix::CreateFromYawPitchRoll(fx->pos.yRot, fx->pos.xPos, fx->pos.zPos) * Matrix::CreateTranslation(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
			newEffect->Mesh = getMesh(obj->nmeshes ? obj->meshIndex : fx->frameNumber);

			collectLightsForEffect(fx->roomNumber, newEffect, renderView);

			renderView.effectsToDraw.push_back(newEffect);
		}
		*/
	}

	void Renderer11::resetAnimations()
	{
		for (int i = 0; i < NUM_ITEMS; i++)
		{
			m_items[i].DoneAnimations = false;
		}
	}
} // namespace TEN::Renderer
