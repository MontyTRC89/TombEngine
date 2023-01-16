#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/spotcam.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "RenderView/RenderView.h"

using namespace TEN::Math;

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using TEN::Memory::LinearArrayBuffer;
	using std::vector;

	void Renderer11::CollectRooms(RenderView &renderView, bool onlyRooms)
	{
		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{
			RendererRoom* room = &m_rooms[i];

			room->ItemsToDraw.clear();
			room->EffectsToDraw.clear();
			room->TransparentFacesToDraw.clear();
			room->StaticsToDraw.clear();
			room->LightsToDraw.clear();
			room->Visited = false;
			room->ViewPort = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);

			for (int j = 0; j < room->Doors.size(); j++)
			{
				room->Doors[j].Visited = false;
			}
		}

		GetVisibleRooms(NO_ROOM, renderView.camera.RoomNumber, Vector4(-1.0f, -1.0f, 1.0f, 1.0f), false, 0, onlyRooms, renderView);

		m_invalidateCache = false;

		for (auto room : renderView.roomsToDraw)
		{
			room->ClipBounds.left = (room->ViewPort.x + 1.0f) * m_screenWidth * 0.5f;
			room->ClipBounds.bottom = (1.0f - room->ViewPort.y) * m_screenHeight * 0.5f;
			room->ClipBounds.right = (room->ViewPort.z + 1.0f) * m_screenWidth * 0.5f;
			room->ClipBounds.top = (1.0f - room->ViewPort.w) * m_screenHeight * 0.5f;
		}
	}

	bool Renderer11::CheckPortal(short parentRoomNumber, RendererDoor* door, Vector4 viewPort, Vector4* clipPort, RenderView& renderView)
	{
		m_numCheckPortalCalls++;

		RendererRoom* room = &m_rooms[parentRoomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[parentRoomNumber];

		int  zClip = 0;
		Vector4 p[4];

		*clipPort = Vector4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 4; i++)
		{
			if (!door->Visited)
			{
				p[i] = Vector4::Transform(door->AbsoluteVertices[i], renderView.camera.ViewProjection);
				if (p[i].w > 0.0f)
				{
					p[i].x *= (1.0f / p[i].w);
					p[i].y *= (1.0f / p[i].w);

				}
				door->TransformedVertices[i] = p[i];
			}
			else
			{
				p[i] = door->TransformedVertices[i];
			}

			if (p[i].w > 0.0f)
			{
				clipPort->x = std::min(clipPort->x, p[i].x);
				clipPort->y = std::min(clipPort->y, p[i].y);
				clipPort->z = std::max(clipPort->z, p[i].x);
				clipPort->w = std::max(clipPort->w, p[i].y);
			}
			else
			{
				zClip++;
			}
		}

		door->Visited = true;

		if (zClip == 4)
		{
			return false;
		}

		if (zClip > 0)
		{
			for (int i = 0; i < 4; i++) 
			{
				Vector4 a = p[i];
				Vector4 b = p[(i + 1) % 4];

				if ((a.w > 0.0f) ^ (b.w > 0.0f))
				{

					if (a.x < 0.0f && b.x < 0.0f)
					{
						clipPort->x = -1.0f;
					}
					else
					{
						if (a.x > 0.0f && b.x > 0.0f)
						{
							clipPort->z = 1.0f;
						}
						else 
						{
							clipPort->x = -1.0f;
							clipPort->z = 1.0f;
						}
					}

					if (a.y < 0.0f && b.y < 0.0f)
					{
						clipPort->y = -1.0f;
					}
					else
					{
						if (a.y > 0.0f && b.y > 0.0f)
						{
							clipPort->w = 1.0f;
						}
						else 
						{
							clipPort->y = -1.0f;
							clipPort->w = 1.0f;
						}
					}
				}
			}
		}

		if (clipPort->x > viewPort.z || clipPort->y > viewPort.w || clipPort->z < viewPort.x || clipPort->w < viewPort.y)
		{
			return false;
		}

		clipPort->x = std::max(clipPort->x, viewPort.x);
		clipPort->y = std::max(clipPort->y, viewPort.y);
		clipPort->z = std::min(clipPort->z, viewPort.z);
		clipPort->w = std::min(clipPort->w, viewPort.w);

		return true;
	}

	void Renderer11::GetVisibleRooms(short from, short to, Vector4 viewPort, bool water, int count, bool onlyRooms, RenderView& renderView)
	{
		// FIXME: This is an urgent hack to fix stack overflow crashes.
		// See https://github.com/MontyTRC89/TombEngine/issues/947 for details.
		// NOTE by MontyTRC: I'd keep this as a failsafe solution for 0.00000001% of cases we could have problems

		static constexpr int MAX_SEARCH_DEPTH = 128;
		if (m_rooms[to].Visited && count > MAX_SEARCH_DEPTH)
		{
			TENLog("Maximum room collection depth of " + std::to_string(MAX_SEARCH_DEPTH) + 
				   " was reached with room " + std::to_string(to), LogLevel::Warning);
			return;
		}

		m_numGetVisibleRoomsCalls++;

		RendererRoom* room = &m_rooms[to];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[to];

		auto cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		float xRad = nativeRoom->xSize * SECTOR(1) / 2.0f;
		float yRad = (nativeRoom->minfloor - nativeRoom->maxceiling) / 2.0f;
		float zRad = nativeRoom->zSize * SECTOR(1) / 2.0f;

		auto roomCentre = Vector3(nativeRoom->x + xRad, nativeRoom->minfloor - yRad, nativeRoom->z + zRad);

		float roomRad = std::max(std::max(xRad, yRad), zRad);
		float distance = std::max((roomCentre - cameraPosition).Length() - (roomRad * 1.5f), 0.0f);

		if (!m_rooms[to].Visited)
		{
			renderView.roomsToDraw.push_back(room);

			CollectLightsForRoom(to, renderView);

			if (!onlyRooms)
			{
				CollectItems(to, renderView);
				CollectStatics(to, renderView);
				CollectEffects(to);
			}
		}

		room->Distance = distance;
		room->Visited = true;
		room->ViewPort.x = std::min(room->ViewPort.x, viewPort.x);
		room->ViewPort.y = std::min(room->ViewPort.y, viewPort.y);
		room->ViewPort.z = std::max(room->ViewPort.z, viewPort.z);
		room->ViewPort.w = std::max(room->ViewPort.w, viewPort.w);

		Vector4 clipPort;
		for (int i = 0; i < room->Doors.size(); i++)
		{
			RendererDoor* door = &room->Doors[i];

			if (!door->Visited)
			{
				door->CameraToDoor = Vector3(
					Camera.pos.x - (door->AbsoluteVertices[0].x),
					Camera.pos.y - (door->AbsoluteVertices[0].y),
					Camera.pos.z - (door->AbsoluteVertices[0].z));
				door->CameraToDoor.Normalize();
			}

			// IMPORTANT: dot = 0 would generate ambiguity becase door could be traversed in both directions, potentially 
			// generating endless loops. We need to exclude this.

			if (door->Normal.x * door->CameraToDoor.x +
				door->Normal.y * door->CameraToDoor.y +
				door->Normal.z * door->CameraToDoor.z <= 0)
			{
				continue;
			}

			if (from != door->RoomNumber && CheckPortal(to, door, viewPort, &clipPort, renderView))
			{
				GetVisibleRooms(to, door->RoomNumber, clipPort, water, count + 1, onlyRooms, renderView);
			}
		}
	}

	void Renderer11::CollectItems(short roomNumber, RenderView& renderView)
	{
		if (m_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = g_Level.Items[itemNum].NextItem)
		{
			ItemInfo* item = &g_Level.Items[itemNum];

			if (item->ObjectNumber == ID_LARA && itemNum == g_Level.Items[itemNum].NextItem)
			{
				break;
			}

			if (item->Status == ITEM_INVISIBLE)
			{
				continue;
			}

			if (item->ObjectNumber == ID_LARA && (BinocularRange || SpotcamOverlay || SpotcamDontDrawLara))
			{
				continue;
			}

			if (item->ObjectNumber == ID_LARA && CurrentLevel == 0 && !g_GameFlow->IsLaraInTitleEnabled())
			{
				continue;
			}

			if (!m_moveableObjects[item->ObjectNumber].has_value())
			{
				continue;
			}

			auto& obj = m_moveableObjects[item->ObjectNumber].value();

			if (obj.DoNotDraw)
			{
				continue;
			}

			// Clip object by frustum only if it doesn't cast shadows. Otherwise we may see
			// disappearing shadows if object gets out of frustum.

			if (obj.ShadowType == ShadowMode::None)
			{
				// Get all spheres and check if frustum intersects any of them.
				static BoundingSphere spheres[MAX_BONES];
				int cnt = GetSpheres(itemNum, spheres, SPHERES_SPACE_WORLD, Matrix::Identity);

				bool inFrustum = false;
				for (int i = 0; !inFrustum, i < cnt; i++)
					// Blow up sphere radius by half for cases of too small calculated spheres.
					if (renderView.camera.Frustum.SphereInFrustum(spheres[i].Center, spheres[i].Radius * 1.5f))
						inFrustum = true;
				
				if (!inFrustum)
					continue;
			}

			auto newItem = &m_items[itemNum];

			newItem->ItemNumber = itemNum;
			newItem->ObjectNumber = item->ObjectNumber;
			newItem->Color = item->Model.Color;
			newItem->Position = item->Pose.Position.ToVector3();
			newItem->Translation = Matrix::CreateTranslation(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
			newItem->Rotation = item->Pose.Orientation.ToRotationMatrix();
			newItem->Scale = Matrix::CreateScale(1.0f);
			newItem->World = newItem->Rotation * newItem->Translation;

			CalculateLightFades(newItem);
			CollectLightsForItem(newItem);

			room.ItemsToDraw.push_back(newItem);
		}
	}

	void Renderer11::CollectStatics(short roomNumber, RenderView& renderView)
	{
		if (m_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		if (r->mesh.size() == 0)
		{
			return;
		}

		for (int i = 0; i < room.Statics.size(); i++)
		{
			auto* mesh = &room.Statics[i];
			MESH_INFO* nativeMesh = &r->mesh[i];

			if (nativeMesh->Dirty || m_invalidateCache)
			{
				mesh->ObjectNumber = nativeMesh->staticNumber;
				mesh->Color = nativeMesh->color;
				mesh->OriginalVisibilityBox = StaticObjects[mesh->ObjectNumber].visibilityBox;
				mesh->Pose = nativeMesh->pos;
				mesh->Scale = nativeMesh->scale;
				mesh->Update();

				nativeMesh->Dirty = false;
			}

			if (!(nativeMesh->flags & StaticMeshFlags::SM_VISIBLE))
			{
				continue;
			}

			if (!m_staticObjects[mesh->ObjectNumber].has_value())
			{
				continue;
			}

			auto& obj = *m_staticObjects[mesh->ObjectNumber];

			if (obj.ObjectMeshes.size() == 0)
			{
				continue;
			}

			auto length = Vector3(mesh->VisibilityBox.Extents).Length();
			if (!renderView.camera.Frustum.SphereInFrustum(mesh->VisibilityBox.Center, length))
			{
				continue;
			}

			// Collect the lights
			std::vector<RendererLight*> lights;
			std::vector<RendererLight*> cachedRoomLights;
			if (obj.ObjectMeshes.front()->LightMode != LIGHT_MODES::LIGHT_MODE_STATIC)
			{
				if (mesh->CacheLights || m_invalidateCache)
				{
					// Collect all lights and return also cached light for the next frames
					CollectLights(mesh->Pose.Position.ToVector3(), ITEM_LIGHT_COLLECTION_RADIUS, room.RoomNumber, NO_ROOM, false, false, &cachedRoomLights, &lights);
					mesh->CacheLights = false;
					mesh->CachedRoomLights = cachedRoomLights;
				}
				else
				{
					// Collecy only dynamic lights and use cached lights from rooms
					CollectLights(mesh->Pose.Position.ToVector3(), ITEM_LIGHT_COLLECTION_RADIUS, room.RoomNumber, NO_ROOM, false, true, &mesh->CachedRoomLights, &lights);
				}
			}
			mesh->LightsToDraw = lights;

			// At this point, we are sure that we must draw the static mesh
			room.StaticsToDraw.push_back(mesh);
		}
	}

	void Renderer11::CollectLights(Vector3 position, float radius, int roomNumber, int prevRoomNumber, bool prioritizeShadowLight, bool useCachedRoomLights, std::vector<RendererLight*>* roomsLights, std::vector<RendererLight*>* outputLights)
	{
		if (m_rooms.size() < roomNumber)
		{
			return;
		}

		// Now collect lights from dynamic list and from rooms
		std::vector<RendererLight*> tempLights;
		tempLights.reserve(MAX_LIGHTS_DRAW);
		
		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* nativeRoom = &g_Level.Rooms[room.RoomNumber];

		RendererLight* brightestLight = nullptr;
		float brightest = 0.0f;

		// Dynamic lights have the priority
		for (auto& light : dynamicLights)
		{
			float distanceSquared =
				SQUARE(position.x - light.Position.x) +
				SQUARE(position.y - light.Position.y) +
				SQUARE(position.z - light.Position.z);

			// Collect only lights nearer than 20 sectors
			if (distanceSquared >= SQUARE(SECTOR(20)))
			{
				continue;
			}

			// Check the out radius
			if (distanceSquared > SQUARE(light.Out + radius))
			{
				continue;
			}

			float distance = sqrt(distanceSquared);
			float attenuation = 1.0f - distance / light.Out;
			float intensity = std::max(0.0f, attenuation * light.Intensity * light.Luma);

			light.LocalIntensity = intensity;
			light.Distance = distance;

			tempLights.push_back(&light);
		}
	
		if (!useCachedRoomLights)
		{
			// Check current room and also neighbour rooms
			for (int roomToCheck : room.Neighbors)
			{
				RendererRoom& currentRoom = m_rooms[roomToCheck];
				int numLights = currentRoom.Lights.size();

				for (int j = 0; j < numLights; j++)
				{
					RendererLight* light = &currentRoom.Lights[j];

					// Check only lights different from sun
					if (light->Type == LIGHT_TYPE_SUN)
					{
						// Suns from non-adjacent rooms are not added!
						if (roomToCheck != roomNumber && (prevRoomNumber != roomToCheck || prevRoomNumber == NO_ROOM))
						{
							continue;
						}

						// Sun is added without distance checks
						light->Distance = 0;
						light->LocalIntensity = 0;
					}
					else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
					{
						float distanceSquared =
							SQUARE(position.x - light->Position.x) +
							SQUARE(position.y - light->Position.y) +
							SQUARE(position.z - light->Position.z);

						// Collect only lights nearer than 20 sectors
						if (distanceSquared >= SQUARE(SECTOR(20)))
						{
							continue;
						}

						// Check the out radius
						if (distanceSquared > SQUARE(light->Out + radius))
						{
							continue;
						}

						float distance = sqrt(distanceSquared);
						float attenuation = 1.0f - distance / light->Out;
						float intensity = std::max(0.0f, attenuation * light->Intensity * Luma(light->Color));

						light->LocalIntensity = intensity;
						light->Distance = distance;

						// If collecting shadows, try to collect shadow casting light
						if (light->CastShadows && prioritizeShadowLight && light->Type == LIGHT_TYPE_POINT)
						{
							if (intensity >= brightest)
							{
								brightest = intensity;
								brightestLight = light;
							}
						}
					}
					else if (light->Type == LIGHT_TYPE_SPOT)
					{
						float distanceSquared =
							SQUARE(position.x - light->Position.x) +
							SQUARE(position.y - light->Position.y) +
							SQUARE(position.z - light->Position.z);

						// Collect only lights nearer than 20 sectors
						if (distanceSquared >= SQUARE(SECTOR(20)))
						{
							continue;
						}

						// Check the range
						if (distanceSquared > SQUARE(light->Out + radius))
						{
							continue;
						}

						float distance = sqrt(distanceSquared);
						float attenuation = 1.0f - distance / light->Out;
						float intensity = std::max(0.0f, attenuation * light->Intensity * light->Luma);

						light->LocalIntensity = intensity;

						// If shadow pointer provided, try to collect shadow casting light
						if (light->CastShadows && prioritizeShadowLight)
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

					if (roomsLights != nullptr)
					{
						roomsLights->push_back(light);
					}
					tempLights.push_back(light);
				}
			}
		}
		else
		{
			for (int i = 0; i < roomsLights->size(); i++)
			{
				tempLights.push_back(roomsLights->at(i));
			}
		}

		// Sort lights by distance, if needed

		if (tempLights.size() > MAX_LIGHTS_PER_ITEM)
		{
			std::sort(
				tempLights.begin(),
				tempLights.end(),
				[](RendererLight* a, RendererLight* b)
				{
					return a->Distance < b->Distance;
				}
			);
		}

		// Now put actual lights to provided vector
		outputLights->clear();

		// Always add brightest light, if collecting shadow light is specified, even if it's far in range
		if (prioritizeShadowLight && brightestLight)
		{
			outputLights->push_back(brightestLight);
		}

		// Add max 8 lights per item, including the shadow light for Lara eventually
		for (auto l : tempLights)
		{
			if (prioritizeShadowLight && brightestLight == l)
			{
				continue;
			}

			outputLights->push_back(l);

			if (outputLights->size() == MAX_LIGHTS_PER_ITEM)
			{
				break;
			}
		}
	}

	void Renderer11::CollectLightsForCamera()
	{
		std::vector<RendererLight*> lightsToDraw;
		CollectLights(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), CAMERA_LIGHT_COLLECTION_RADIUS, Camera.pos.RoomNumber, NO_ROOM, true, false, nullptr, &lightsToDraw);

		if (lightsToDraw.size() > 0 && lightsToDraw.front()->CastShadows)
		{
			shadowLight = lightsToDraw.front();
		}
		else
		{
			shadowLight = nullptr;
		}
	}	
	
	void Renderer11::CollectLightsForEffect(short roomNumber, RendererEffect* effect)
	{
		CollectLights(effect->Position, ITEM_LIGHT_COLLECTION_RADIUS, roomNumber, NO_ROOM, false, false, nullptr, &effect->LightsToDraw);
	}

	void Renderer11::CollectLightsForItem(RendererItem* item)
	{
		CollectLights(item->Position, ITEM_LIGHT_COLLECTION_RADIUS, item->RoomNumber, item->PrevRoomNumber, false, false, nullptr, &item->LightsToDraw);
	}

	void Renderer11::CalculateLightFades(RendererItem *item)
	{
		ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];

		// Interpolate ambient light between rooms
		if (item->PrevRoomNumber == NO_ROOM)
		{
			item->PrevRoomNumber = nativeItem->RoomNumber;
			item->RoomNumber = nativeItem->RoomNumber;
			item->LightFade = 1.0f;
		}
		else if (nativeItem->RoomNumber != item->RoomNumber)
		{
			item->PrevRoomNumber = item->RoomNumber;
			item->RoomNumber = nativeItem->RoomNumber;
			item->LightFade = 0.0f;
		}
		else if (item->LightFade < 1.0f)
		{
			item->LightFade += AMBIENT_LIGHT_INTERPOLATION_STEP;
			item->LightFade = std::clamp(item->LightFade, 0.0f, 1.0f);
		}

		if (item->PrevRoomNumber == NO_ROOM || item->LightFade == 1.0f)
			item->AmbientLight = m_rooms[nativeItem->RoomNumber].AmbientLight;
		else
		{
			auto prev = m_rooms[item->PrevRoomNumber].AmbientLight;
			auto next = m_rooms[item->RoomNumber].AmbientLight;

			item->AmbientLight.x = Lerp(prev.x, next.x, item->LightFade);
			item->AmbientLight.y = Lerp(prev.y, next.y, item->LightFade);
			item->AmbientLight.z = Lerp(prev.z, next.z, item->LightFade);
		}

		// Multiply calculated ambient light by object tint
		item->AmbientLight *= nativeItem->Model.Color;
	}

	void Renderer11::CollectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (m_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = m_rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];
		
		// Collect dynamic lights for rooms
		for (int i = 0; i < dynamicLights.size(); i++)
		{
			RendererLight* light = &dynamicLights[i];

			// If no radius, ignore
			if (light->Out == 0.0f)
			{
				continue;
			}

			// Light buffer is full
			if (renderView.lightsToDraw.size() >= NUM_LIGHTS_PER_BUFFER)
			{
				break;
			}

			// Light already on a list
			if (std::find(renderView.lightsToDraw.begin(), renderView.lightsToDraw.end(), light) != renderView.lightsToDraw.end())
			{
				continue;
			}

			// Light is not within room bounds
			if (!room.BoundingBox.Intersects(light->BoundingSphere))
			{
				continue;
			}

			renderView.lightsToDraw.push_back(light);
			room.LightsToDraw.push_back(light);
		}
	}

	void Renderer11::CollectEffects(short roomNumber)
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

			ObjectInfo *obj = &Objects[fx->objectNumber];

			RendererEffect *newEffect = &m_effects[fxNum];

			Matrix translation = Matrix::CreateTranslation(fx->pos.Position.x, fx->pos.Position.y, fx->pos.Position.z);
			Matrix rotation = fx->pos.Orientation.ToRotationMatrix();

			newEffect->ObjectNumber = fx->objectNumber;
			newEffect->RoomNumber = fx->roomNumber;
			newEffect->Position = fx->pos.Position.ToVector3();
			newEffect->AmbientLight = room.AmbientLight;
			newEffect->Color = fx->color;
			newEffect->World = rotation * translation;
			newEffect->Mesh = GetMesh(obj->nmeshes ? obj->meshIndex : fx->frameNumber);

			CollectLightsForEffect(fx->roomNumber, newEffect);

			room.EffectsToDraw.push_back(newEffect);
		}
	}

	void Renderer11::ResetAnimations()
	{
		for (int i = 0; i < NUM_ITEMS; i++)
			m_items[i].DoneAnimations = false;
	}

} // namespace TEN::Renderer

