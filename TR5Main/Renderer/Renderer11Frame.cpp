#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Game/camera.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "RenderView\RenderView.h"
#include "Game/items.h"

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using TEN::Memory::LinearArrayBuffer;
	using std::vector;

	void Renderer11::CollectRooms(RenderView &renderView, bool onlyRooms)
	{
		short baseRoomIndex = renderView.camera.RoomNumber;

		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{
			m_rooms[i].ItemsToDraw.clear();
			m_rooms[i].EffectsToDraw.clear();
			m_rooms[i].TransparentFacesToDraw.clear();
			m_rooms[i].StaticsToDraw.clear();
			m_rooms[i].Visited = false;
		}

		GetVisibleObjects(-1, baseRoomIndex, renderView, onlyRooms);
	}

	void Renderer11::CollectItems(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = g_Level.Items[itemNum].NextItem)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];

			if (item->ObjectNumber == ID_LARA && itemNum == g_Level.Items[itemNum].NextItem)
				break;

			if (item->ObjectNumber == ID_LARA)
				continue;

			if (item->Status == ITEM_INVISIBLE)
				continue;

			if (!m_moveableObjects[item->ObjectNumber].has_value())
				continue;

			auto bounds = TO_DX_BBOX(item->Position, GetBoundsAccurate(item));
			Vector3 min = bounds.Center - bounds.Extents;
			Vector3 max = bounds.Center + bounds.Extents;

			if (!renderView.camera.frustum.AABBInFrustum(min, max))
				continue;

			auto newItem = &m_items[itemNum];

			newItem->ItemNumber = itemNum;
			newItem->Translation = Matrix::CreateTranslation(item->Position.xPos, item->Position.yPos, item->Position.zPos);
			newItem->Rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(item->Position.yRot),
															   TO_RAD(item->Position.xRot),
															   TO_RAD(item->Position.zRot));
			newItem->Scale = Matrix::CreateScale(1.0f);
			newItem->World = newItem->Rotation * newItem->Translation;

			CollectLightsForItem(item->RoomNumber, newItem, renderView);

			room.ItemsToDraw.push_back(newItem);
		}
	}

	void Renderer11::CollectStatics(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;

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

	void Renderer11::CollectLightsForEffect(short roomNumber, RendererEffect *effect, RenderView &renderView)
	{
		effect->Lights.clear();
		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		if (r->lights.size() <= 0)
			return;
		LinearArrayBuffer<RendererLight*, 8> tempLights;

		Vector3 itemPosition = Vector3(effect->Effect->pos.xPos, effect->Effect->pos.yPos, effect->Effect->pos.zPos);

		// Dynamic lights have the priority
		for (int i = 0; i < dynamicLights.size(); i++)
		{
			RendererLight* light = &dynamicLights[i];

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

	void Renderer11::CollectLightsForItem(short roomNumber, RendererItem *item, RenderView &renderView)
	{
		item->LightsToDraw.clear();

		if (m_rooms.size() < roomNumber)
			return;

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[room.RoomNumber];
		ITEM_INFO* nativeItem = &g_Level.Items[item->ItemNumber];

		std::vector<int> roomsToCheck;
		roomsToCheck.push_back(roomNumber);
		for (auto& door : nativeRoom->doors)
		{
			roomsToCheck.push_back(door.room);
		};

		// Interpolate ambient light between rooms
		if (item->PreviousRoomNumber == NO_ITEM)
		{
			item->PreviousRoomNumber = nativeItem->RoomNumber;
			item->CurrentRoomNumber = nativeItem->RoomNumber;
			item->AmbientLightSteps = AMBIENT_LIGHT_INTERPOLATION_STEPS;
		}
		else if (nativeItem->RoomNumber != item->CurrentRoomNumber)
		{
			item->PreviousRoomNumber = item->CurrentRoomNumber;
			item->CurrentRoomNumber = nativeItem->RoomNumber;
			item->AmbientLightSteps = 0;
		}
		else if (item->AmbientLightSteps < AMBIENT_LIGHT_INTERPOLATION_STEPS)
			item->AmbientLightSteps++;

		if (item->PreviousRoomNumber == NO_ITEM)
			item->AmbientLight = m_rooms[nativeItem->RoomNumber].AmbientLight;
		else
		{
			item->AmbientLight = (((AMBIENT_LIGHT_INTERPOLATION_STEPS - item->AmbientLightSteps) / (float)AMBIENT_LIGHT_INTERPOLATION_STEPS) * m_rooms[item->PreviousRoomNumber].AmbientLight +
				(item->AmbientLightSteps / (float)AMBIENT_LIGHT_INTERPOLATION_STEPS) * m_rooms[item->CurrentRoomNumber].AmbientLight);
			item->AmbientLight.w = 1.0f;
		}

		// Now collect lights from dynamic list and from rooms
		std::vector<RendererLight*> tempLights;
		tempLights.reserve(MAX_LIGHTS_DRAW);

		Vector3 itemPosition = Vector3(nativeItem->Position.xPos, nativeItem->Position.yPos, nativeItem->Position.zPos);

		if (nativeItem->ObjectNumber == ID_LARA)
			shadowLight = nullptr;

		RendererLight* brightestLight = NULL;
		float brightest = 0.0f;

		// Dynamic lights have the priority
		for (int i = 0; i < dynamicLights.size(); i++)
		{
			RendererLight* light = &dynamicLights[i];

			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();
			if (distance > light->Out)
				continue;

			float attenuation = 1.0f - distance / light->Out;
			float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

			light->LocalIntensity = intensity;
			light->Distance = distance;

			tempLights.push_back(light);
		}

		// Check current room and also neighbour rooms
		for (int roomToCheck : roomsToCheck)
		{
			RendererRoom& currentRoom = m_rooms[roomToCheck];
			int numLights = currentRoom.Lights.size();

			for (int j = 0; j < numLights; j++)
			{
				RendererLight* light = &currentRoom.Lights[j];

				light->AffectNeighbourRooms = light->Type != LIGHT_TYPES::LIGHT_TYPE_SUN;

				if (!light->AffectNeighbourRooms && roomToCheck != roomNumber)
					continue;

				// Check only lights different from sun
				if (light->Type == LIGHT_TYPE_SUN)
				{
					// Sun is added without checks
					light->Distance = D3D11_FLOAT32_MAX;
					light->LocalIntensity = 0;
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

					float attenuation = 1.0f - distance / light->Out;
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					light->LocalIntensity = intensity;

					// If Lara, try to collect shadow casting light
					if (nativeItem->ObjectNumber == ID_LARA && light->Type == LIGHT_TYPE_POINT)
					{
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

					float attenuation = 1.0f - distance / light->Range;
					float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					light->LocalIntensity = intensity;

					// If Lara, try to collect shadow casting light
					if (nativeItem->ObjectNumber == ID_LARA)
					{
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
		}

		if (nativeItem->ObjectNumber == ID_LARA)
			shadowLight = brightestLight;

		// Sort lights by distance
		std::sort(
			tempLights.begin(),
			tempLights.end(),
			[](RendererLight* a, RendererLight* b)
			{
				return a->LocalIntensity > b->LocalIntensity;
			}
		);

		// Add max 8 lights per item, including the shadow light for Lara eventually
		if (shadowLight != nullptr)
			item->LightsToDraw.push_back(shadowLight);
		for (int i = 0; i < tempLights.size(); i++)
		{
			if (shadowLight != nullptr && shadowLight == tempLights[i])
				continue;

			item->LightsToDraw.push_back(tempLights[i]);

			if (item->LightsToDraw.size() == MAX_LIGHTS_PER_ITEM)
				break;
		}
	}

	void Renderer11::CollectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
			return;
		
		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		int numLights = room.Lights.size();

		// Collect dynamic lights for rooms
		for (int i = 0; i < dynamicLights.size(); i++)
		{
			RendererLight* light = &dynamicLights[i];

			Vector3 boxMin = Vector3(r->x - 2 * WALL_SIZE, -(r->minfloor + STEP_SIZE), r->z - 2 * WALL_SIZE);
			Vector3 boxMax = Vector3(r->x + (r->xSize + 1) * WALL_SIZE, -(r->maxceiling - STEP_SIZE), r->z + (r->zSize + 1) * WALL_SIZE);
			Vector3 center = Vector3(light->Position.x, -light->Position.y, light->Position.z);

			if (renderView.lightsToDraw.size() < NUM_LIGHTS_PER_BUFFER - 1 &&
				SphereBoxIntersection(boxMin, boxMax, center, light->Out))
				renderView.lightsToDraw.push_back(light);
		}
	}

	void Renderer11::CollectEffects(short roomNumber, RenderView &renderView)
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
			newEffect->Mesh = GetMesh(obj->nmeshes ? obj->meshIndex : fx->frameNumber);

			CollectLightsForEffect(fx->roomNumber, newEffect, renderView);

			room.EffectsToDraw.push_back(newEffect);
		}
	}

	void Renderer11::resetAnimations()
	{
		for (int i = 0; i < NUM_ITEMS; i++)
			m_items[i].DoneAnimations = false;
	}
} // namespace TEN::Renderer
