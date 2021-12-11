#include "framework.h"
#include "Renderer11.h"
#include "animation.h"
#include "lara.h"
#include "effects\effects.h"
#include "camera.h"
#include "level.h"
#include "setup.h"
#include "RenderView\RenderView.h"
#include "items.h"

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using TEN::Memory::LinearArrayBuffer;
	using std::vector;

	void Renderer11::collectRooms(RenderView &renderView, bool onlyRooms)
	{
		short baseRoomIndex = renderView.camera.RoomNumber;

		for (int i = 0; i < g_Level.Rooms.size(); i++) {
			m_rooms[i].ItemsToDraw.clear();
			m_rooms[i].EffectsToDraw.clear();
			m_rooms[i].TransparentFacesToDraw.clear();
			m_rooms[i].StaticsToDraw.clear();
			m_rooms[i].Visited = false;
		}

		getVisibleObjects(-1, baseRoomIndex, renderView, onlyRooms);
	}

	void Renderer11::collectItems(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber) {
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = g_Level.Items[itemNum].nextItem)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];

			if (item->objectNumber == ID_LARA && itemNum == g_Level.Items[itemNum].nextItem)
				break;

			if (item->objectNumber == ID_LARA)
				continue;

			if (item->status == ITEM_INVISIBLE)
				continue;

			if (!m_moveableObjects[item->objectNumber].has_value())
				continue;

			auto bounds = TO_DX_BBOX(item->pos, GetBoundsAccurate(item));
			Vector3 min = bounds.Center - bounds.Extents;
			Vector3 max = bounds.Center + bounds.Extents;

			if (!renderView.camera.frustum.AABBInFrustum(min, max))
				continue;

			auto newItem = &m_items[itemNum];

			newItem->ItemNumber = itemNum;
			newItem->Translation = Matrix::CreateTranslation(item->pos.xPos, item->pos.yPos, item->pos.zPos);
			newItem->Rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(item->pos.yRot),
															   TO_RAD(item->pos.xRot),
															   TO_RAD(item->pos.zRot));
			newItem->Scale = Matrix::CreateScale(1.0f);
			newItem->World = newItem->Rotation * newItem->Translation;

			collectLightsForItem(item->roomNumber, newItem, renderView);

			room.ItemsToDraw.push_back(newItem);
		}
	}

	void Renderer11::collectStatics(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber) {
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		if (r->mesh.size() <= 0)
			return;

		int numStatics = r->mesh.size();
		for (int i = 0; i < numStatics; i++)
		{
			auto mesh = &r->mesh[i];
			if (mesh->flags & StaticMeshFlags::SM_VISIBLE)
			{
				auto sinfo = &StaticObjects[mesh->staticNumber];

				auto bounds = TO_DX_BBOX(mesh->pos, &sinfo->visibilityBox);
				Vector3 min = bounds.Center - bounds.Extents;
				Vector3 max = bounds.Center + bounds.Extents;

				if (!renderView.camera.frustum.AABBInFrustum(min, max))
					continue;

				room.StaticsToDraw.push_back(mesh);
			}
		}
	}

	void Renderer11::collectLightsForEffect(short roomNumber, RendererEffect *effect, RenderView &renderView)
	{
		effect->Lights.clear();
		if (m_rooms.size() < roomNumber) {
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

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

		shadowLight = NULL;
		RendererLight* brightestLight = NULL;
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
			if (renderView.lightsToDraw.size() < NUM_LIGHTS_PER_BUFFER - 1)
				renderView.lightsToDraw.push_back(tempLights[i]);
		}
	}

	void Renderer11::collectLightsForItem(short roomNumber, RendererItem *item, RenderView &renderView)
	{
		item->LightsToDraw.clear();

		if (m_rooms.size() < roomNumber){
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[room.RoomNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];

		std::vector<RendererLight*> tempLights;
		tempLights.reserve(MAX_LIGHTS_DRAW);

		Vector3 itemPosition = Vector3(nativeItem->pos.xPos, nativeItem->pos.yPos, nativeItem->pos.zPos);

		// Dynamic lights have the priority
		for (int i = 0; i < m_dynamicLights.size(); i++)
		{
			RendererLight* light = m_dynamicLights[i];

			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();
			if (distance > light->Out)
				continue;

			light->Distance = distance;

			tempLights.push_back(light);
		}

		if (nativeItem->objectNumber == ID_LARA)
		{
			shadowLight = nullptr;
		}

		RendererLight* brightestLight = NULL;
		float brightest = 0.0f;

		int numLights = room.Lights.size();

		for (int j = 0; j < numLights; j++)
		{
			RendererLight* light = &room.Lights[j];

			// Check only lights different from sun
			if (light->Type == LIGHT_TYPE_SUN)
			{
				// Sun is added without checks
				light->Distance = D3D11_FLOAT32_MAX;
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
				if (nativeItem->objectNumber == ID_LARA && light->Type == LIGHT_TYPE_POINT)
				{
					float attenuation = 1.0f - distance / light->Out;
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = light;
					}
				}

				light->Distance = distance;
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
				if (nativeItem->objectNumber == ID_LARA)
				{
					float attenuation = 1.0f - distance / light->Range;
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = light;
					}
				}

				light->Distance = distance;
			}
			else
			{
				// Invalid light type
				continue;
			}

			tempLights.push_back(light);
		} 

		if (nativeItem->objectNumber == ID_LARA)
		{
			shadowLight = brightestLight;
		}

		// Sort lights by distance
		std::sort(
			tempLights.begin(),
			tempLights.end(),
			[](RendererLight* a, RendererLight* b) {
				return a->Distance > b->Distance;
			}
		);

		// Add max 8 lights per item, including the shadow light for Lara eventually
		numLights = std::min((size_t)MAX_LIGHTS_PER_ITEM, tempLights.size());
		if (shadowLight != nullptr)
		{
			item->LightsToDraw.push_back(shadowLight);
			numLights--;
		}
		for (int i = 0; i < numLights; i++)
		{
			if (shadowLight != nullptr && shadowLight == tempLights[i])
				continue;
			item->LightsToDraw.push_back(tempLights[i]);
		}
	}

	void Renderer11::collectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber){
			return;
		}
		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		int numLights = room.Lights.size();

		// Collect dynamic lights for rooms
		for (int i = 0; i < m_dynamicLights.size(); i++)
		{
			RendererLight *light = m_dynamicLights[i];

			Vector3 boxMin = Vector3(r->x - 2 * WALL_SIZE, -(r->minfloor + STEP_SIZE), r->z - 2 * WALL_SIZE);
			Vector3 boxMax = Vector3(r->x + (r->xSize + 1) * WALL_SIZE, -(r->maxceiling - STEP_SIZE), r->z + (r->zSize + 1) * WALL_SIZE);
			Vector3 center = Vector3(light->Position.x, -light->Position.y, light->Position.z);

			if (renderView.lightsToDraw.size() < NUM_LIGHTS_PER_BUFFER - 1
				&& sphereBoxIntersection(boxMin, boxMax, center, light->Out))
				renderView.lightsToDraw.push_back(light);
		}
	}

	void Renderer11::prepareLights(RenderView& view)
	{
		// Add dynamic lights
		for (int i = 0; i < m_dynamicLights.size(); i++)
			if (view.lightsToDraw.size() < NUM_LIGHTS_PER_BUFFER - 1)
				view.lightsToDraw.push_back(m_dynamicLights[i]);

		// Now I have a list full of draw. Let's sort them.
		//std::sort(m_lightsToDraw.begin(), m_lightsToDraw.end(), SortLightsFunction);
		//m_lightsToDraw.Sort(SortLightsFunction);

		// Let's draw first 32 lights
		//if (m_lightsToDraw.size() > 32)
		//	m_lightsToDraw.resize(32);

		// Now try to search for a shadow caster, using Lara as reference
		RendererRoom& room = m_rooms[LaraItem->roomNumber];

		// Search for the brightest light. We do a simple version of the classic calculation done in pixel shader.
		RendererLight* brightestLight = NULL;
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
		shadowLight = brightestLight;
	}

	void Renderer11::collectEffects(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

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

			room.EffectsToDraw.push_back(newEffect);
		}
	}

	void Renderer11::resetAnimations()
	{
		for (int i = 0; i < NUM_ITEMS; i++)
		{
			m_items[i].DoneAnimations = false;
		}
	}
} // namespace TEN::Renderer