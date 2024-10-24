#include "Renderer/Renderer.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/Sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Game/spotcam.h"
#include "Renderer/RenderView.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/level.h"

using namespace TEN::Collision::Sphere;

namespace TEN::Renderer
{
	using TEN::Memory::LinearArrayBuffer;

	void Renderer::CollectRooms(RenderView& renderView, bool onlyRooms)
	{
		constexpr auto VIEW_PORT = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);

		_visitedRoomsStack.clear();

		for (int i = 0; i < g_Level.Rooms.size(); i++)
		{ 
			auto& room = _rooms[i];
			                         
			room.ItemsToDraw.clear();        
			room.EffectsToDraw.clear();
			room.StaticsToDraw.clear();
			room.LightsToDraw.clear();
			room.Visited = false;
			room.ViewPort = VIEW_PORT;

			for (auto& door : room.Doors)
			{
				door.Visited = false;
				door.InvisibleFromCamera = false;
				door.DotProduct = FLT_MAX;
			}
		}

		GetVisibleRooms(NO_VALUE, renderView.Camera.RoomNumber, VIEW_PORT, false, 0, onlyRooms, renderView);

		_invalidateCache = false; 

		// Prepare real DX scissor test rectangle.
		for (auto* roomPtr : renderView.RoomsToDraw)
		{
			roomPtr->ClipBounds.Left = (roomPtr->ViewPort.x + 1.0f) * _screenWidth * 0.5f;
			roomPtr->ClipBounds.Bottom = (1.0f - roomPtr->ViewPort.y) * _screenHeight * 0.5f;
			roomPtr->ClipBounds.Right = (roomPtr->ViewPort.z + 1.0f) * _screenWidth * 0.5f;
			roomPtr->ClipBounds.Top = (1.0f - roomPtr->ViewPort.w) * _screenHeight * 0.5f;
		} 

		// Collect fog bulbs.
		std::vector<RendererFogBulb> tempFogBulbs;
		tempFogBulbs.reserve(MAX_FOG_BULBS_DRAW);

		for (auto& room : _rooms)     
		{
			if (!g_Level.Rooms[room.RoomNumber].Active())
				continue;
			      
			for (const auto& light : room.Lights)
			{
				if (light.Type != LightType::FogBulb)
					continue;

				// Test bigger radius to avoid bad clipping.
				if (renderView.Camera.Frustum.SphereInFrustum(light.Position, light.Out * 1.2f))
				{
					RendererFogBulb bulb;
					
					bulb.Position = light.Position;
					bulb.Density = light.Intensity;
					bulb.Color = light.Color;
					bulb.Radius = light.Out;
					bulb.FogBulbToCameraVector = bulb.Position - renderView.Camera.WorldPosition;
					bulb.Distance = bulb.FogBulbToCameraVector.Length();

					tempFogBulbs.push_back(bulb);
				}
			}
		}
		
		// Sort fog bulbs.
		std::sort(
			tempFogBulbs.begin(),
			tempFogBulbs.end(),
			[](const RendererFogBulb& bulb0, const RendererFogBulb& bulb1)
			{
				return bulb0.Distance < bulb1.Distance;
			});

		for (int i = 0; i < std::min(MAX_FOG_BULBS_DRAW, (int)tempFogBulbs.size()); i++)
			renderView.FogBulbsToDraw.push_back(tempFogBulbs[i]);
	}

	bool Renderer::CheckPortal(short parentRoomNumber, RendererDoor* door, Vector4 viewPort, Vector4* clipPort, RenderView& renderView)
	{
		_numCheckPortalCalls++;

		RendererRoom* room = &_rooms[parentRoomNumber];

		int  zClip = 0;
		Vector4 p[4];

		*clipPort = Vector4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 4; i++)
		{
			if (!door->Visited)
			{
				p[i] = Vector4::Transform(door->AbsoluteVertices[i], renderView.Camera.ViewProjection);
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

	void Renderer::GetVisibleRooms(short from, short to, Vector4 viewPort, bool water, int count, bool onlyRooms, RenderView& renderView)
	{
		// FIXME: This is an urgent hack to fix stack overflow crashes.
		// See https://github.com/MontyTRC89/TombEngine/issues/947 for details.
		// NOTE by MontyTRC: I'd keep this as a failsafe solution for 0.00000001% of cases we could have problems

		int stackSize = (int)_visitedRoomsStack.size();
		int stackMinIndex = std::max(0, int(stackSize - 5));

		for (int i = stackSize - 1; i >= stackMinIndex; i--)
		{
			if (_visitedRoomsStack[i] == to)
			{
				TENLog("Circle detected! Room " + std::to_string(to), LogLevel::Warning, LogConfig::Debug);
				return;
			}
		}
		
		static constexpr int MAX_SEARCH_DEPTH = 64;
		if (_rooms[to].Visited && count > MAX_SEARCH_DEPTH)
		{
			TENLog("Maximum room collection depth of " + std::to_string(MAX_SEARCH_DEPTH) + 
				   " was reached with room " + std::to_string(to), LogLevel::Warning, LogConfig::Debug);
			return;
		}

		_visitedRoomsStack.push_back(to);

		_numGetVisibleRoomsCalls++;

		RendererRoom* room = &_rooms[to];

		if (!room->Visited)
		{
			room->Visited = true;

			renderView.RoomsToDraw.push_back(room);

			CollectLightsForRoom(to, renderView);

			if (!onlyRooms)
			{
				CollectItems(to, renderView);
				CollectStatics(to, renderView);
				CollectEffects(to);
			}
		}

		room->ViewPort.x = std::min(room->ViewPort.x, viewPort.x);
		room->ViewPort.y = std::min(room->ViewPort.y, viewPort.y);
		room->ViewPort.z = std::max(room->ViewPort.z, viewPort.z);
		room->ViewPort.w = std::max(room->ViewPort.w, viewPort.w);

		Vector4 clipPort;
		for (int i = 0; i < room->Doors.size(); i++)
		{
			RendererDoor* door = &room->Doors[i];

			if (door->InvisibleFromCamera)
			{
				continue;
			}

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

			if (door->DotProduct == FLT_MAX)
			{
				door->DotProduct = 
					door->Normal.x * door->CameraToDoor.x +
					door->Normal.y * door->CameraToDoor.y +
					door->Normal.z * door->CameraToDoor.z;
				_numDotProducts++;
			}

			if (door->DotProduct < 0)
			{
				door->InvisibleFromCamera = true;
				continue;
			}

			if (from != door->RoomNumber && CheckPortal(to, door, viewPort, &clipPort, renderView))
				GetVisibleRooms(to, door->RoomNumber, clipPort, water, count + 1, onlyRooms, renderView);
		}

		_visitedRoomsStack.pop_back();
	}

	void Renderer::CollectItems(short roomNumber, RenderView& renderView)
	{
		if (_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = _rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short itemNum = NO_VALUE;
		for (itemNum = r->itemNumber; itemNum != NO_VALUE; itemNum = g_Level.Items[itemNum].NextItem)
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

			if (item->ObjectNumber == ID_LARA && (Lara.Control.Look.OpticRange || SpotcamOverlay || SpotcamDontDrawLara))
			{
				continue;
			}

			if (item->ObjectNumber == ID_LARA && CurrentLevel == 0 && !g_GameFlow->IsLaraInTitleEnabled())
			{
				continue;
			}

			if (!_moveableObjects[item->ObjectNumber].has_value())
			{
				continue;
			}

			auto& obj = _moveableObjects[item->ObjectNumber].value();

			if (obj.DoNotDraw)
			{
				continue;
			}

			// Clip object by frustum only if it doesn't cast shadows. Otherwise we may see
			// disappearing shadows if object gets out of frustum.

			if (obj.ShadowType == ShadowMode::None)
			{
				// Get all spheres and check if frustum intersects any of them.
				auto spheres = GetSpheres(itemNum);

				bool inFrustum = false;
				for (int i = 0; !inFrustum, i < spheres.size(); i++)
					// Blow up sphere radius by half for cases of too small calculated spheres.
					if (renderView.Camera.Frustum.SphereInFrustum(spheres[i].Center, spheres[i].Radius * 1.5f))
						inFrustum = true;
				
				if (!inFrustum)
					continue;
			}

			auto newItem = &_items[itemNum];

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

	void Renderer::CollectStatics(short roomNumber, RenderView& renderView)
	{
		if (_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = _rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		if (r->mesh.empty())
		{
			return;
		}

		for (int i = 0; i < room.Statics.size(); i++)
		{
			auto* mesh = &room.Statics[i];
			MESH_INFO* nativeMesh = &r->mesh[i];

			if (nativeMesh->Dirty || _invalidateCache)
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

			if (!_staticObjects[mesh->ObjectNumber].has_value())
			{
				continue;
			}

			auto& obj = *_staticObjects[mesh->ObjectNumber];

			if (obj.ObjectMeshes.empty())
			{
				continue;
			}

			if (!renderView.Camera.Frustum.SphereInFrustum(mesh->VisibilityBox.Center, mesh->VisibilitySphereRadius))
			{
				continue;
			}
			 
			// Collect the lights
			std::vector<RendererLight*> lights;
			std::vector<RendererLightNode> cachedRoomLights;
			if (obj.ObjectMeshes.front()->LightMode != LightMode::Static)
			{
				if (mesh->CacheLights || _invalidateCache)
				{
					// Collect all lights and return also cached light for the next frames
					CollectLights(mesh->Pose.Position.ToVector3(),1024, room.RoomNumber, NO_VALUE, false, false, &cachedRoomLights, &lights);
					mesh->CacheLights = false;
					mesh->CachedRoomLights = cachedRoomLights;
				}
				else
				{
					// Collecy only dynamic lights and use cached lights from rooms
					CollectLights(mesh->Pose.Position.ToVector3(), 1024, room.RoomNumber, NO_VALUE, false, true, &mesh->CachedRoomLights, &lights);
				}
			}
			mesh->LightsToDraw = lights;

			// At this point, we are sure that we must draw the static mesh
			room.StaticsToDraw.push_back(mesh);

			if (renderView.SortedStaticsToDraw.find(mesh->ObjectNumber) == renderView.SortedStaticsToDraw.end())
			{
				std::vector<RendererStatic*> vec;
				renderView.SortedStaticsToDraw.insert(std::pair<int, std::vector<RendererStatic*>>(mesh->ObjectNumber, std::vector<RendererStatic*>()));
			}
			renderView.SortedStaticsToDraw[mesh->ObjectNumber].push_back(mesh);
		}
	}

	void Renderer::CollectLights(Vector3 position, float radius, int roomNumber, int prevRoomNumber, bool prioritizeShadowLight, bool useCachedRoomLights, std::vector<RendererLightNode>* roomsLights, std::vector<RendererLight*>* outputLights)
	{
		if (_rooms.size() < roomNumber)
		{
			return;
		}

		// Now collect lights from dynamic list and from rooms
		std::vector<RendererLightNode> tempLights;
		tempLights.reserve(MAX_LIGHTS_DRAW);
		
		RendererRoom& room = _rooms[roomNumber];

		RendererLight* brightestLight = nullptr;
		float brightest = 0.0f;

		// Dynamic lights have the priority
		for (auto& light : _dynamicLights)
		{
			float distanceSquared =
				SQUARE(position.x - light.Position.x) +
				SQUARE(position.y - light.Position.y) +
				SQUARE(position.z - light.Position.z);

			// Collect only lights nearer than 20 sectors
			if (distanceSquared >= SQUARE(BLOCK(20)))
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
			float intensity = attenuation * light.Intensity * light.Luma;

			RendererLightNode node = { &light, intensity, distance, 1 };
			tempLights.push_back(node);
		}
	
		if (!useCachedRoomLights)
		{
			// Check current room and also neighbour rooms
			for (int roomToCheck : room.Neighbors)
			{
				RendererRoom& currentRoom = _rooms[roomToCheck];
				int numLights = (int)currentRoom.Lights.size();

				for (int j = 0; j < numLights; j++)
				{
					RendererLight* light = &currentRoom.Lights[j];

					float intensity = 0;
					float distance = 0;

					// Check only lights different from sun
					if (light->Type == LightType::Sun)
					{
						// Suns from non-adjacent rooms are not added!
						if (roomToCheck != roomNumber && (prevRoomNumber != roomToCheck || prevRoomNumber == NO_VALUE))
						{
							continue;
						}

						// Sun is added without distance checks
						intensity = light->Intensity * Luma(light->Color);						
					}
					else if (light->Type == LightType::Point || light->Type == LightType::Shadow)
					{
						float distanceSquared =
							SQUARE(position.x - light->Position.x) +
							SQUARE(position.y - light->Position.y) +
							SQUARE(position.z - light->Position.z);

						// Collect only lights nearer than 20 sectors
						if (distanceSquared >= SQUARE(BLOCK(20)))
						{
							continue;
						}

						// Check the out radius
						if (distanceSquared > SQUARE(light->Out + radius))
						{
							continue;
						}

						distance = sqrt(distanceSquared);
						float attenuation = 1.0f - distance / light->Out;
						intensity = attenuation * light->Intensity * Luma(light->Color);

						// If collecting shadows, try to collect shadow casting light
						if (light->CastShadows && prioritizeShadowLight && light->Type == LightType::Point)
						{
							if (intensity >= brightest)
							{
								brightest = intensity;
								brightestLight = light;
							}
						}
					}
					else if (light->Type == LightType::Spot)
					{
						float distanceSquared =
							SQUARE(position.x - light->Position.x) +
							SQUARE(position.y - light->Position.y) +
							SQUARE(position.z - light->Position.z);

						// Collect only lights nearer than 20 sectors
						if (distanceSquared >= SQUARE(BLOCK(20)))
						{
							continue;
						}

						// Check the range
						if (distanceSquared > SQUARE(light->Out + radius))
						{
							continue;
						}

						distance = sqrt(distanceSquared);
						float attenuation = 1.0f - distance / light->Out;
						intensity = attenuation * light->Intensity * light->Luma;

						// If shadow pointer provided, try to collect shadow casting light
						if (light->CastShadows && prioritizeShadowLight)
						{
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

					RendererLightNode node = { light, intensity, distance, 0 };

					if (roomsLights != nullptr)
					{
						roomsLights->push_back(node);
					}

					tempLights.push_back(node);
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

		// Sort lights
		if (tempLights.size() > MAX_LIGHTS_PER_ITEM)
		{
			std::sort(
				tempLights.begin(),
				tempLights.end(),
				[](RendererLightNode a, RendererLightNode b)
				{
					if (a.Dynamic == b.Dynamic)
						return a.LocalIntensity > b.LocalIntensity;
					else
						return a.Dynamic > b.Dynamic;
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
			if (prioritizeShadowLight && brightestLight == l.Light)
			{
				continue;
			}

			outputLights->push_back(l.Light);

			if (outputLights->size() == MAX_LIGHTS_PER_ITEM)
			{
				break;
			}
		}
	}

	void Renderer::CollectLightsForCamera()
	{
		std::vector<RendererLight*> lightsToDraw;
		CollectLights(Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), CAMERA_LIGHT_COLLECTION_RADIUS, Camera.pos.RoomNumber, NO_VALUE, true, false, nullptr, &lightsToDraw);

		if (lightsToDraw.size() > 0 && lightsToDraw.front()->CastShadows)
		{
			_shadowLight = lightsToDraw.front();
		}
		else
		{
			_shadowLight = nullptr;
		}
	}	
	
	void Renderer::CollectLightsForEffect(short roomNumber, RendererEffect* effect)
	{
		CollectLights(effect->Position, ITEM_LIGHT_COLLECTION_RADIUS, roomNumber, NO_VALUE, false, false, nullptr, &effect->LightsToDraw);
	}

	void Renderer::CollectLightsForItem(RendererItem* item)
	{
		CollectLights(item->Position, ITEM_LIGHT_COLLECTION_RADIUS, item->RoomNumber, item->PrevRoomNumber, false, false, nullptr, &item->LightsToDraw);
	}

	void Renderer::CalculateLightFades(RendererItem *item)
	{
		ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];

		// Interpolate ambient light between rooms
		if (item->PrevRoomNumber == NO_VALUE)
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

		if (item->PrevRoomNumber == NO_VALUE || item->LightFade == 1.0f)
			item->AmbientLight = _rooms[nativeItem->RoomNumber].AmbientLight;
		else
		{
			auto prev = _rooms[item->PrevRoomNumber].AmbientLight;
			auto next = _rooms[item->RoomNumber].AmbientLight;

			item->AmbientLight.x = Lerp(prev.x, next.x, item->LightFade);
			item->AmbientLight.y = Lerp(prev.y, next.y, item->LightFade);
			item->AmbientLight.z = Lerp(prev.z, next.z, item->LightFade);
		}

		// Multiply calculated ambient light by object tint
		item->AmbientLight *= nativeItem->Model.Color;
	}

	void Renderer::CollectLightsForRoom(short roomNumber, RenderView &renderView)
	{
		if (_rooms.size() < roomNumber)
		{
			return;
		}

		RendererRoom& room = _rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];
		
		// Collect dynamic lights for rooms
		for (int i = 0; i < _dynamicLights.size(); i++)
		{
			RendererLight* light = &_dynamicLights[i];

			// If no radius, ignore
			if (light->Out == 0.0f)
			{
				continue;
			}

			// Light buffer is full
			if (renderView.LightsToDraw.size() >= NUM_LIGHTS_PER_BUFFER)
			{
				break;
			}

			// Light already on a list
			if (std::find(renderView.LightsToDraw.begin(), renderView.LightsToDraw.end(), light) != renderView.LightsToDraw.end())
			{
				continue;
			}

			// Light is not within room bounds
			if (!room.BoundingBox.Intersects(light->BoundingSphere))
			{
				continue;
			}

			renderView.LightsToDraw.push_back(light);
			room.LightsToDraw.push_back(light);
		}
	}

	void Renderer::CollectEffects(short roomNumber)
	{
		if (_rooms.size() < roomNumber)
			return;

		RendererRoom& room = _rooms[roomNumber];
		ROOM_INFO* r = &g_Level.Rooms[room.RoomNumber];

		short fxNum = NO_VALUE;
		for (fxNum = r->fxNumber; fxNum != NO_VALUE; fxNum = EffectList[fxNum].nextFx)
		{
			FX_INFO *fx = &EffectList[fxNum];
			if (fx->objectNumber < 0 || fx->color.w <= 0)
				continue;

			ObjectInfo *obj = &Objects[fx->objectNumber];

			RendererEffect *newEffect = &_effects[fxNum];

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

	void Renderer::ResetAnimations()
	{
		for (int i = 0; i < ITEM_COUNT_MAX; i++)
			_items[i].DoneAnimations = false;
	}

} // namespace TEN::Renderer
