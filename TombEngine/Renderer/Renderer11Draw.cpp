#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/configuration.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/hair.h"
#include "Game/effects/weather.h"
#include "Game/savegame.h"
#include "Game/health.h"
#include "Game/camera.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Game/gui.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/winmain.h"
#include "Objects/Effects/tr4_locusts.h"
#include "Objects/Generic/Object/rope.h"
#include "Objects/TR4/Entity/tr4_beetle_swarm.h"
#include "Objects/TR5/Emitter/tr5_rats_emitter.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include "RenderView/RenderView.h"
#include <chrono>
#include <algorithm>
#include <execution>
#include <filesystem>

using namespace TEN::Entities::Generic;

extern TEN::Renderer::RendererHUDBar* g_HealthBar;
extern TEN::Renderer::RendererHUDBar* g_AirBar;
extern TEN::Renderer::RendererHUDBar* g_LoadingBar;

extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];

namespace TEN::Renderer
{
	using namespace TEN::Renderer;
	using namespace std::chrono;

	void Renderer11::RenderBlobShadows(RenderView& renderView)
	{
		std::vector<Sphere> nearestSpheres;
		nearestSpheres.reserve(g_Configuration.ShadowMaxBlobs);

		// Collect Lara spheres

		static const std::array<LARA_MESHES, 4> sphereMeshes = { LM_HIPS, LM_TORSO, LM_LFOOT, LM_RFOOT };
		static const std::array<float, 4> sphereScaleFactors = { 6.0f, 3.2f, 2.8f, 2.8f };

		for (auto& r : renderView.roomsToDraw) 
		{
			for (auto& i : r->ItemsToDraw) 
			{
				auto& nativeItem = g_Level.Items[i->ItemNumber];

				//Skip everything thats not "alive" or is not a vehicle

				if (Objects[nativeItem.ObjectNumber].shadowType == ShadowMode::None)
					continue;

				if (i->ObjectNumber == ID_LARA)
				{
					for (auto i = 0; i < sphereMeshes.size(); i++)
					{
						if (!nativeItem.TestBits(JointBitType::Mesh, sphereMeshes[i]))
							continue;

						MESH& m = g_Level.Meshes[Lara.MeshPtrs[sphereMeshes[i]]];
						Vector3Int pos = { (int)m.sphere.Center.x, (int)m.sphere.Center.y, (int)m.sphere.Center.z };

						// Push feet spheres a little bit down
						if (sphereMeshes[i] == LM_LFOOT || sphereMeshes[i] == LM_RFOOT)
							pos.y += 8;
						GetLaraJointPosition(&pos, sphereMeshes[i]);

						auto& newSphere = nearestSpheres.emplace_back();
						newSphere.position = Vector3(pos.x, pos.y, pos.z);
						newSphere.radius = m.sphere.Radius * sphereScaleFactors[i];
					}
				}
				else
				{
					auto bb = GetBoundsAccurate(&nativeItem);
					Vector3 center = ((Vector3(bb->X1, bb->Y1, bb->Z1) + Vector3(bb->X2, bb->Y2, bb->Z2)) / 2) +
						Vector3(nativeItem.Pose.Position.x, nativeItem.Pose.Position.y, nativeItem.Pose.Position.z);
					center.y = nativeItem.Pose.Position.y;
					float maxExtent = std::max(bb->X2 - bb->X1, bb->Z2 - bb->Z1);

					auto& newSphere = nearestSpheres.emplace_back();
					newSphere.position = center;
					newSphere.radius = maxExtent;
				}
			}
		}

		if (nearestSpheres.size() > g_Configuration.ShadowMaxBlobs) 
		{
			std::sort(nearestSpheres.begin(), nearestSpheres.end(), [](const Sphere& a, const Sphere& b) 
			{
				auto& laraPos = LaraItem->Pose.Position;
				auto laraPosition = Vector3(laraPos.x, laraPos.y, laraPos.z);
				return Vector3::Distance(laraPosition, a.position) < Vector3::Distance(laraPosition, b.position);
			});

			std::copy(nearestSpheres.begin(), nearestSpheres.begin() + g_Configuration.ShadowMaxBlobs, m_stShadowMap.Spheres);
			m_stShadowMap.NumSpheres = g_Configuration.ShadowMaxBlobs;
		}
		else 
		{
			std::copy(nearestSpheres.begin(), nearestSpheres.end(), m_stShadowMap.Spheres);
			m_stShadowMap.NumSpheres = nearestSpheres.size();
		}
	}

	void Renderer11::ClearShadowMap()
	{
		for (int step = 0; step < m_shadowMap.RenderTargetView.size(); step++)
		{
			m_context->ClearRenderTargetView(m_shadowMap.RenderTargetView[step].Get(), Colors::White);
			m_context->ClearDepthStencilView(m_shadowMap.DepthStencilView[step].Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0);
		}
	}

	void Renderer11::RenderShadowMap(RendererItem* item, RenderView& renderView)
	{
		// Doesn't cast shadow
		if (m_moveableObjects[item->ObjectNumber].value().ShadowType == ShadowMode::None)
			return;

		// Only render for Lara if such setting is active
		if (g_Configuration.ShadowType == ShadowMode::Lara && m_moveableObjects[item->ObjectNumber].value().ShadowType != ShadowMode::Lara)
			return;
		
		// No shadow light found
		if (shadowLight == nullptr)
			return;

		// Shadow light found but type is incorrect
		if (shadowLight->Type != LIGHT_TYPE_POINT && shadowLight->Type != LIGHT_TYPE_SPOT)
			return; 

		// Reset GPU state
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);

		int steps = shadowLight->Type == LIGHT_TYPE_POINT ? 6 : 1;
		for (int step = 0; step < steps; step++) 
		{
			// Bind render target
			m_context->OMSetRenderTargets(1, m_shadowMap.RenderTargetView[step].GetAddressOf(),
				m_shadowMap.DepthStencilView[step].Get());

			m_context->RSSetViewports(1, &m_shadowMapViewport);
			ResetScissor();

			if (shadowLight->Position == item->Position)
				return;

			UINT stride = sizeof(RendererVertex);
			UINT offset = 0;

			// Set shaders
			m_context->VSSetShader(m_vsShadowMap.Get(), nullptr, 0);
			m_context->PSSetShader(m_psShadowMap.Get(), nullptr, 0);

			m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());
			m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			// Set texture
			BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[0]), SAMPLER_ANISOTROPIC_CLAMP);
			BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[0]), SAMPLER_NONE);

			// Set camera matrices
			Matrix view;
			Matrix projection;
			if (shadowLight->Type == LIGHT_TYPE_POINT) 
			{
				view = Matrix::CreateLookAt(shadowLight->Position, shadowLight->Position +
					RenderTargetCube::forwardVectors[step] * SECTOR(10),
					RenderTargetCube::upVectors[step]);

				projection = Matrix::CreatePerspectiveFieldOfView(90.0f * PI / 180.0f, 1.0f, 16.0f, shadowLight->Out);

			}
			else if (shadowLight->Type == LIGHT_TYPE_SPOT) 
			{
				view = Matrix::CreateLookAt(shadowLight->Position,
					shadowLight->Position - shadowLight->Direction * SECTOR(10),
					Vector3(0.0f, -1.0f, 0.0f));

				// Vertex lighting fades out in 1024-steps. increase angle artificially for a bigger blend radius.
				float projectionAngle = shadowLight->OutRange * 1.5f * (PI / 180.0f); 
				projection = Matrix::CreatePerspectiveFieldOfView(projectionAngle, 1.0f, 16.0f, shadowLight->Out);
			}

			CCameraMatrixBuffer shadowProjection;
			shadowProjection.ViewProjection = view * projection;
			m_cbCameraMatrices.updateData(shadowProjection, m_context.Get());
			BindConstantBufferVS(CB_CAMERA, m_cbCameraMatrices.get());

			m_stShadowMap.LightViewProjections[step] = (view * projection);

			SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

			RendererObject& obj = *m_moveableObjects[item->ObjectNumber];
			RendererObject& skin = item->ObjectNumber == ID_LARA ? *m_moveableObjects[ID_LARA_SKIN] : obj;
			RendererRoom& room = m_rooms[item->RoomNumber];

			m_stItem.World = item->World;
			m_stItem.Color = item->Color;
			m_stItem.AmbientLight = item->AmbientLight;
			memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * MAX_BONES);
			for (int k = 0; k < MAX_BONES; k++)
				m_stItem.BoneLightModes[k] = LIGHT_MODES::LIGHT_MODE_STATIC;

			m_cbItem.updateData(m_stItem, m_context.Get());
			BindConstantBufferVS(CB_ITEM, m_cbItem.get());
			BindConstantBufferPS(CB_ITEM, m_cbItem.get());

			for (int k = 0; k < skin.ObjectMeshes.size(); k++)
			{
				auto* mesh = item->ObjectNumber == ID_LARA ? GetMesh(Lara.MeshPtrs[k]) : skin.ObjectMeshes[k];

				for (auto& bucket : mesh->Buckets)
				{
					if (bucket.NumVertices == 0)
						continue;

					if (bucket.BlendMode != BLEND_MODES::BLENDMODE_OPAQUE && bucket.BlendMode != BLEND_MODES::BLENDMODE_ALPHATEST)
						continue;

					// Draw vertices
					DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

					m_numMoveablesDrawCalls++;
				}
			}

			if (item->ObjectNumber != ID_LARA)
				continue;

			if (m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
			{
				auto& laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];

				for (int k = 0; k < laraSkinJoints.ObjectMeshes.size(); k++)
				{
					auto* mesh = laraSkinJoints.ObjectMeshes[k];

					for (auto& bucket : mesh->Buckets)
					{
						if (bucket.NumVertices == 0 && bucket.BlendMode != BLEND_MODES::BLENDMODE_OPAQUE)
							continue;

						// Draw vertices
						DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

						m_numMoveablesDrawCalls++;
					}
				}
			}

			auto& hairsObj = *m_moveableObjects[ID_LARA_HAIR];

			// First matrix is Lara's head matrix, then all 6 hairs matrices. Bones are adjusted at load time for accounting this.
			m_stItem.World = Matrix::Identity;
			m_stItem.BonesMatrices[0] = obj.AnimationTransforms[LM_HEAD] * item->World;

			for (int i = 0; i < hairsObj.BindPoseTransforms.size(); i++)
			{
				auto* hairs = &Hairs[0][i];
				Matrix world = Matrix::CreateFromYawPitchRoll(TO_RAD(hairs->pos.Orientation.y), TO_RAD(hairs->pos.Orientation.x), 0) *
							   Matrix::CreateTranslation(hairs->pos.Position.x, hairs->pos.Position.y, hairs->pos.Position.z);
				m_stItem.BonesMatrices[i + 1] = world;
				m_stItem.BoneLightModes[i] = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
			}

			m_cbItem.updateData(m_stItem, m_context.Get());
			BindConstantBufferVS(CB_ITEM, m_cbItem.get());
			BindConstantBufferPS(CB_ITEM, m_cbItem.get());

			for (int k = 0; k < hairsObj.ObjectMeshes.size(); k++)
			{
				auto* mesh = hairsObj.ObjectMeshes[k];

				for (auto& bucket : mesh->Buckets)
				{
					if (bucket.NumVertices == 0 && bucket.BlendMode != BLEND_MODES::BLENDMODE_OPAQUE)
						continue;

					// Draw vertices
					DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

					m_numMoveablesDrawCalls++;
				}
			}
		}
	}

	void Renderer11::DrawGunShells(RenderView& view)
	{
		RendererRoom& room = m_rooms[LaraItem->RoomNumber];
		RendererItem* item = &m_items[Lara.ItemNumber];

		m_stStatic.Color = room.AmbientLight;
		m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_DYNAMIC;

		BindLights(item->LightsToDraw);

		SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

		for (int i = 0; i < MAX_GUNSHELL; i++)
		{
			GUNSHELL_STRUCT* gunshell = &Gunshells[i];

			if (gunshell->counter <= 0)
				continue;

			ObjectInfo* obj = &Objects[gunshell->objectNumber];
			RendererObject& moveableObj = *m_moveableObjects[gunshell->objectNumber];

			Matrix translation = Matrix::CreateTranslation(gunshell->pos.Position.x, gunshell->pos.Position.y,
															gunshell->pos.Position.z);
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(gunshell->pos.Orientation.y), TO_RAD(gunshell->pos.Orientation.x),
																TO_RAD(gunshell->pos.Orientation.z));
			Matrix world = rotation * translation;
			m_stStatic.World = world;

			m_cbStatic.updateData(m_stStatic, m_context.Get());
			BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
			BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

			RendererMesh* mesh = moveableObj.ObjectMeshes[0];

			for (auto& bucket : mesh->Buckets)
			{
				if (bucket.NumVertices == 0 && bucket.BlendMode == BLEND_MODES::BLENDMODE_OPAQUE)
					continue;

				BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[bucket.Texture]), SAMPLER_ANISOTROPIC_CLAMP);
				BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[bucket.Texture]), SAMPLER_NONE);

				// Draw vertices
				DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

				m_numMoveablesDrawCalls++;
			}
		}
	}

	void Renderer11::DrawRopes(RenderView& view)
	{
		for (auto& rope : Ropes)
		{
			if (!rope.active)
				continue;

			Matrix world = Matrix::CreateTranslation(
				rope.position.x,
				rope.position.y,
				rope.position.z
			);

			Vector3 absolute[24];

			for (int n = 0; n < ROPE_SEGMENTS; n++)
			{
				Vector3Int* s = &rope.meshSegment[n];
				Vector3 t;
				Vector3 output;

				t.x = s->x >> FP_SHIFT;
				t.y = s->y >> FP_SHIFT;
				t.z = s->z >> FP_SHIFT;

				output = Vector3::Transform(t, world);

				Vector3 absolutePosition = Vector3(output.x, output.y, output.z);
				absolute[n] = absolutePosition;
			}

			for (int n = 0; n < ROPE_SEGMENTS - 1; n++)
			{
				Vector3 pos1 = Vector3(absolute[n].x, absolute[n].y, absolute[n].z);
				Vector3 pos2 = Vector3(absolute[n + 1].x, absolute[n + 1].y, absolute[n + 1].z);

				Vector3 d = pos2 - pos1;
				d.Normalize();

				Vector3 c = (pos1 + pos2) / 2.0f;

				AddSpriteBillboardConstrained(&m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_EMPTY1],
					c,
					m_rooms[rope.room].AmbientLight,
					(PI / 2),
					1.0f,
					{ 32,
					Vector3::Distance(pos1, pos2) },
					BLENDMODE_ALPHATEST,
					d, view);
			}
		}
	}

	void Renderer11::DrawLines2D()
	{
		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsSolid.Get(), nullptr, 0);
		m_context->PSSetShader(m_psSolid.Get(), nullptr, 0);
		Matrix world = Matrix::CreateOrthographicOffCenter(0, m_screenWidth, m_screenHeight, 0, m_viewport.MinDepth,
														   m_viewport.MaxDepth);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();

		for (int i = 0; i < m_lines2DToDraw.size(); i++)
		{
			RendererLine2D* line = &m_lines2DToDraw[i];

			RendererVertex v1;
			v1.Position.x = line->Vertices[0].x;
			v1.Position.y = line->Vertices[0].y;
			v1.Position.z = 1.0f;
			v1.Color.x = line->Color.x / 255.0f;
			v1.Color.y = line->Color.y / 255.0f;
			v1.Color.z = line->Color.z / 255.0f;
			v1.Color.w = line->Color.w / 255.0f;

			RendererVertex v2;
			v2.Position.x = line->Vertices[1].x;
			v2.Position.y = line->Vertices[1].y;
			v2.Position.z = 1.0f;
			v2.Color.x = line->Color.x / 255.0f;
			v2.Color.y = line->Color.y / 255.0f;
			v2.Color.z = line->Color.z / 255.0f;
			v2.Color.w = line->Color.w / 255.0f;

			v1.Position = Vector3::Transform(v1.Position, world);
			v2.Position = Vector3::Transform(v2.Position, world);

			v1.Position.z = 0.5f;
			v2.Position.z = 0.5f;

			m_primitiveBatch->DrawLine(v1, v2);
		}

		m_primitiveBatch->End();

		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);
	}

	void Renderer11::DrawSpiders(RenderView& view)
	{
		/*XMMATRIX world;
		UINT cPasses = 1;

		if (Objects[ID_SPIDERS_EMITTER].loaded)
		{
		    ObjectInfo* obj = &Objects[ID_SPIDERS_EMITTER];
		    RendererObject* moveableObj = m_moveableObjects[ID_SPIDERS_EMITTER].get();
		    short* meshPtr = Meshes[Objects[ID_SPIDERS_EMITTER].meshIndex + ((Wibble >> 2) & 2)];
		    RendererMesh* mesh = m_meshPointersToMesh[meshPtr];
		    RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		    if (bucket->NumVertices == 0)
		        return true;

		    setGpuStateForBucket(bucketIndex);

		    m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		    m_device->SetIndices(bucket->GetIndexBuffer());

		    LPD3DXEFFECT effect;
		    if (pass == RENDERER_PASS_SHADOW_MAP)
		        effect = m_shaderDepth->GetEffect();
		    else if (pass == RENDERER_PASS_RECONSTRUCT_DEPTH)
		        effect = m_shaderReconstructZBuffer->GetEffect();
		    else if (pass == RENDERER_PASS_GBUFFER)
		        effect = m_shaderFillGBuffer->GetEffect();
		    else
		        effect = m_shaderTransparent->GetEffect();

		    effect->SetBool(effect->GetParameterByName(nullptr, "UseSkinning"), false);
		    effect->SetInt(effect->GetParameterByName(nullptr, "ModelType"), MODEL_TYPE_MOVEABLE);

		    if (bucketIndex == RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKET_SOLID_DS)
		        effect->SetInt(effect->GetParameterByName(nullptr, "BlendMode"), BLENDMODE_OPAQUE);
		    else
		        effect->SetInt(effect->GetParameterByName(nullptr, "BlendMode"), BLENDMODE_ALPHATEST);

		    for (int i = 0; i < NUM_SPIDERS; i++)
		    {
		        SPIDER_STRUCT* spider = &Spiders[i];

		        if (spider->on)
		        {
		            XMMATRIXTranslation(&m_tempTranslation, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);
		            XMMATRIXRotationYawPitchRoll(&m_tempRotation, spider->pos.yRot, spider->pos.xRot, spider->pos.zRot);
		            XMMATRIXMultiply(&m_tempWorld, &m_tempRotation, &m_tempTranslation);
		            effect->SetMatrix(effect->GetParameterByName(nullptr, "World"), &m_tempWorld);

		            effect->SetVector(effect->GetParameterByName(nullptr, "AmbientLight"), &m_rooms[spider->roomNumber]->AmbientLight);

		            for (int iPass = 0; iPass < cPasses; iPass++)
		            {
		                effect->BeginPass(iPass);
		                effect->CommitChanges();

		                drawPrimitives(D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->Indices.size() / 3);

		                effect->EndPass();
		            }
		        }
		    }
		}*/
	}

	void Renderer11::DrawRats(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_RATS_EMITTER].loaded)
		{
			ObjectInfo* obj = &Objects[ID_RATS_EMITTER];
			RendererObject& moveableObj = *m_moveableObjects[ID_RATS_EMITTER];

			m_stStatic.LightMode = moveableObj.ObjectMeshes[0]->LightMode;

			for (int i = 0; i < NUM_RATS; i++)
			{
				auto* rat = &Rats[i];

				if (rat->On)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_RATS_EMITTER].meshIndex + (rand() % 8));
					Matrix translation = Matrix::CreateTranslation(rat->Pose.Position.x, rat->Pose.Position.y, rat->Pose.Position.z);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(rat->Pose.Orientation.y), TO_RAD(rat->Pose.Orientation.x),
																	 TO_RAD(rat->Pose.Orientation.z));
					Matrix world = rotation * translation;

					m_stStatic.World = world;
					m_stStatic.Color = Vector4::One;
					m_stStatic.AmbientLight = m_rooms[rat->RoomNumber].AmbientLight;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

					BindLights(m_rooms[rat->RoomNumber].LightsToDraw);

					for (int b = 0; b < mesh->Buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->Buckets[b];

						if (bucket->Polygons.size() == 0)
							continue;

						DrawIndexedTriangles(bucket->NumIndices, bucket->StartIndex, 0);

						m_numMoveablesDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawBats(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_BATS_EMITTER].loaded)
		{
			ObjectInfo* obj = &Objects[ID_BATS_EMITTER];
			RendererObject& moveableObj = *m_moveableObjects[ID_BATS_EMITTER];
			RendererMesh* mesh = GetMesh(Objects[ID_BATS_EMITTER].meshIndex - (GlobalCounter & 3));

			m_stStatic.LightMode = moveableObj.ObjectMeshes[0]->LightMode;

			for (int b = 0; b < mesh->Buckets.size(); b++)
			{
				RendererBucket* bucket = &mesh->Buckets[b];

				if (bucket->Polygons.size() == 0)
					continue;

				for (int i = 0; i < NUM_BATS; i++)
				{
					auto* bat = &Bats[i];

					if (bat->On)
					{
						Matrix translation = Matrix::CreateTranslation(bat->Pose.Position.x, bat->Pose.Position.y, bat->Pose.Position.z);
						Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(bat->Pose.Orientation.y), TO_RAD(bat->Pose.Orientation.x), 
																		 TO_RAD(bat->Pose.Orientation.z));
						Matrix world = rotation * translation;

						m_stStatic.World = world;
						m_stStatic.Color = Vector4::One;
						m_stStatic.AmbientLight = m_rooms[bat->RoomNumber].AmbientLight;
						m_cbStatic.updateData(m_stStatic, m_context.Get());
						BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

						BindLights(m_rooms[bat->RoomNumber].LightsToDraw);

						DrawIndexedTriangles(bucket->NumIndices, bucket->StartIndex, 0);

						m_numMoveablesDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawScarabs(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_LITTLE_BEETLE].loaded)
		{
			ObjectInfo* obj = &Objects[ID_LITTLE_BEETLE];
			RendererObject& moveableObj = *m_moveableObjects[ID_LITTLE_BEETLE];

			m_stStatic.LightMode = moveableObj.ObjectMeshes[0]->LightMode;

			for (int i = 0; i < TEN::Entities::TR4::NUM_BEETLES; i++)
			{
				auto* beetle = &TEN::Entities::TR4::BeetleSwarm[i];

				if (beetle->On)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_LITTLE_BEETLE].meshIndex + ((Wibble >> 2) % 2));
					Matrix translation = Matrix::CreateTranslation(beetle->Pose.Position.x, beetle->Pose.Position.y, beetle->Pose.Position.z);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(beetle->Pose.Orientation.y), TO_RAD(beetle->Pose.Orientation.x),
																	 TO_RAD(beetle->Pose.Orientation.z));
					Matrix world = rotation * translation;

					m_stStatic.World = world;
					m_stStatic.Color = Vector4::One;
					m_stStatic.AmbientLight = m_rooms[beetle->RoomNumber].AmbientLight;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

					BindLights(m_rooms[beetle->RoomNumber].LightsToDraw);

					for (int b = 0; b < mesh->Buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->Buckets[b];

						if (bucket->Polygons.size() == 0)
							continue;

						DrawIndexedTriangles(bucket->NumIndices, bucket->StartIndex, 0);

						m_numMoveablesDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawLocusts(RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_LOCUSTS].loaded)
		{
			ObjectInfo* obj = &Objects[ID_LOCUSTS];
			RendererObject& moveableObj = *m_moveableObjects[ID_LOCUSTS];
			
			m_stStatic.LightMode = moveableObj.ObjectMeshes[0]->LightMode;

			for (int i = 0; i < TEN::Entities::TR4::MAX_LOCUSTS; i++)
			{
				LOCUST_INFO* locust = &TEN::Entities::TR4::Locusts[i];

				if (locust->on)
				{
					RendererMesh* mesh = GetMesh(Objects[ID_LOCUSTS].meshIndex + (-locust->counter & 3));
					Matrix translation = Matrix::CreateTranslation(locust->pos.Position.x, locust->pos.Position.y, locust->pos.Position.z);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(locust->pos.Orientation.y), TO_RAD(locust->pos.Orientation.x),
																	 TO_RAD(locust->pos.Orientation.z));
					Matrix world = rotation * translation;

					m_stStatic.World = world;
					m_stStatic.Color = Vector4::One;
					m_stStatic.AmbientLight = m_rooms[locust->roomNumber].AmbientLight;
					m_cbStatic.updateData(m_stStatic, m_context.Get());
					BindConstantBufferVS(CB_STATIC, m_cbStatic.get());

					for (int b = 0; b < mesh->Buckets.size(); b++)
					{
						RendererBucket* bucket = &mesh->Buckets[b];

						if (bucket->Polygons.size() == 0)
							continue;

						DrawIndexedTriangles(bucket->NumIndices, bucket->StartIndex, 0);

						m_numMoveablesDrawCalls++;
					}
				}
			}
		}
	}

	void Renderer11::DrawLines3D(RenderView& view)
	{
		SetBlendMode(BLENDMODE_ADDITIVE);
		SetCullMode(CULL_MODE_NONE);

		m_context->VSSetShader(m_vsSolid.Get(), nullptr, 0);
		m_context->PSSetShader(m_psSolid.Get(), nullptr, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		m_primitiveBatch->Begin();

		for (int i = 0; i < m_lines3DToDraw.size(); i++)
		{
			RendererLine3D* line = &m_lines3DToDraw[i];

			RendererVertex v1;
			v1.Position = line->Start;
			v1.Color = line->Color;

			RendererVertex v2;
			v2.Position = line->End;
			v2.Color = line->Color;
			m_primitiveBatch->DrawLine(v1, v2);
		}

		m_primitiveBatch->End();

		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);
	}

	void Renderer11::AddLine3D(Vector3 start, Vector3 end, Vector4 color)
	{
		if (m_Locked)
			return;

		RendererLine3D line;

		line.Start = start;
		line.End = end;
		line.Color = color;

		m_lines3DToDraw.push_back(line);
	}

	void Renderer11::AddSphere(Vector3 center, float radius, Vector4 color)
	{
		if (m_Locked)
			return;

		constexpr auto subdivisions = 10;
		constexpr auto steps = 6;
		constexpr auto step = PI / steps;

		std::array<Vector3, 3> prevPoint;

		for (int s = 0; s < steps; s++)
		{
			auto x = sin(step * (float)s) * radius;
			auto z = cos(step * (float)s) * radius;
			float currAngle = 0.0f;

			for (int i = 0; i < subdivisions; i++)
			{
				std::array<Vector3, 3> point =
				{
					center + Vector3(sin(currAngle) * abs(x), z, cos(currAngle) * abs(x)),
					center + Vector3(cos(currAngle) * abs(x), sin(currAngle) * abs(x), z),
					center + Vector3(z, sin(currAngle) * abs(x), cos(currAngle) * abs(x))
				};

				if (i > 0)
					for (int p = 0; p < 3; p++)
						AddLine3D(prevPoint[p], point[p], color);

				prevPoint = point;
				currAngle += ((PI * 2) / (subdivisions - 1));
			}
		}
	}

	void Renderer11::AddDebugSphere(Vector3 center, float radius, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;

		AddSphere(center, radius, color);
#endif _DEBUG
	}

	void Renderer11::AddBox(Vector3* corners, Vector4 color)
	{
		if (m_Locked)
			return;

		for (int i = 0; i < 12; i++)
		{
			RendererLine3D line;

			switch (i)
			{
			case 0: line.Start = corners[0];
				line.End = corners[1];
				break;
			case 1: line.Start = corners[1];
				line.End = corners[2];
				break;
			case 2: line.Start = corners[2];
				line.End = corners[3];
				break;
			case 3: line.Start = corners[3];
				line.End = corners[0];
				break;

			case 4: line.Start = corners[4];
				line.End = corners[5];
				break;
			case 5: line.Start = corners[5];
				line.End = corners[6];
				break;
			case 6: line.Start = corners[6];
				line.End = corners[7];
				break;
			case 7: line.Start = corners[7];
				line.End = corners[4];
				break;

			case 8: line.Start = corners[0];
				line.End = corners[4];
				break;
			case 9: line.Start = corners[1];
				line.End = corners[5];
				break;
			case 10: line.Start = corners[2];
				line.End = corners[6];
				break;
			case 11: line.Start = corners[3];
				line.End = corners[7];
				break;
			}

			line.Color = color;
			m_lines3DToDraw.push_back(line);
		}
	}

	void Renderer11::AddBox(Vector3 min, Vector3 max, Vector4 color)
	{
		if (m_Locked)
			return;

		for (int i = 0; i < 12; i++)
		{
			RendererLine3D line;

			switch (i)
			{
			case 0: line.Start = Vector3(min.x, min.y, min.z);
				line.End = Vector3(min.x, min.y, max.z);
				break;
			case 1: line.Start = Vector3(min.x, min.y, max.z);
				line.End = Vector3(max.x, min.y, max.z);
				break;
			case 2: line.Start = Vector3(max.x, min.y, max.z);
				line.End = Vector3(max.x, min.y, min.z);
				break;
			case 3: line.Start = Vector3(max.x, min.y, min.z);
				line.End = Vector3(min.x, min.y, min.z);
				break;

			case 4: line.Start = Vector3(min.x, max.y, min.z);
				line.End = Vector3(min.x, max.y, max.z);
				break;
			case 5: line.Start = Vector3(min.x, max.y, max.z);
				line.End = Vector3(max.x, max.y, max.z);
				break;
			case 6: line.Start = Vector3(max.x, max.y, max.z);
				line.End = Vector3(max.x, max.y, min.z);
				break;
			case 7: line.Start = Vector3(max.x, max.y, min.z);
				line.End = Vector3(min.x, max.y, min.z);
				break;

			case 8: line.Start = Vector3(min.x, min.y, min.z);
				line.End = Vector3(min.x, max.y, min.z);
				break;
			case 9: line.Start = Vector3(min.x, min.y, max.z);
				line.End = Vector3(min.x, max.y, max.z);
				break;
			case 10: line.Start = Vector3(max.x, min.y, max.z);
				line.End = Vector3(max.x, max.y, max.z);
				break;
			case 11: line.Start = Vector3(max.x, min.y, min.z);
				line.End = Vector3(max.x, max.y, min.z);
				break;
			}

			line.Color = color;
			m_lines3DToDraw.push_back(line);
		}
	}

	void Renderer11::AddDebugBox(BoundingOrientedBox box, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;

		Vector3 corners[8];
		box.GetCorners(corners);
		AddBox(corners, color);
#endif _DEBUG
	}

	void Renderer11::AddDebugBox(Vector3 min, Vector3 max, Vector4 color, RENDERER_DEBUG_PAGE page)
	{
#ifdef _DEBUG
		if (m_numDebugPage != page)
			return;
		AddBox(min, max, color);
#endif _DEBUG
	}

	void Renderer11::AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
	{
		if (m_Locked)
			return;

		RendererLight dynamicLight = {};

		if (falloff >= 8)
		{
			dynamicLight.Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f) * 2.0f;
		}
		else
		{
			r = (r * falloff) >> 3;
			g = (g * falloff) >> 3;
			b = (b * falloff) >> 3;

			dynamicLight.Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f) * 2.0f;
		}

		dynamicLight.RoomNumber = NO_ROOM;
		dynamicLight.Intensity = 1.0f;
		dynamicLight.Position = Vector3(float(x), float(y), float(z));
		dynamicLight.Out = falloff * 256.0f;
		dynamicLight.Type = LIGHT_TYPES::LIGHT_TYPE_POINT;

		dynamicLights.push_back(dynamicLight);
	}

	void Renderer11::ClearDynamicLights()
	{
		dynamicLights.clear();
	}

	void Renderer11::ClearScene()
	{
		ResetAnimations();

		ClearFires();
		ClearDynamicLights();
		ClearSceneItems();
		ClearShadowMap();

		m_transparentFaces.clear();

		m_currentCausticsFrame++;
		m_currentCausticsFrame %= 32;

		CalculateFrameRate();
	}

	void Renderer11::DrawTransparentFaces(RenderView& view)
	{
		std::for_each(std::execution::par_unseq,
					  view.roomsToDraw.begin(),
					  view.roomsToDraw.end(),
					  [](RendererRoom* room)
					  {
						  std::sort(
							  room->TransparentFacesToDraw.begin(),
							  room->TransparentFacesToDraw.end(),
							  [](RendererTransparentFace& a, RendererTransparentFace& b)
							  {
								  return (a.distance > b.distance);
							  }
						  );
					  }
		);

		for (int r = view.roomsToDraw.size() - 1; r >= 0; r--)
		{
			RendererRoom& room = *view.roomsToDraw[r];

			int size = room.TransparentFacesToDraw.size();
			int currentBlendMode = -1;
			int texture = -1;
			int lastType = 0;

			std::vector<RendererVertex> vertices;

			m_transparentFacesVertices.clear();
			m_transparentFacesIndices.clear();

			RendererTransparentFaceType oldType = RendererTransparentFaceType::TRANSPARENT_FACE_NONE;
			RendererTransparentFaceInfo* oldInfo = nullptr;
			bool outputPolygons = false;

			for (auto& face : room.TransparentFacesToDraw)
			{  
				if (oldInfo != nullptr)
				{
					// Check if it's time to output polygons
					if (oldType != face.type)
					{
						outputPolygons = true;
					}
					else
					{
						// If same type, check additional conditions
						if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM &&
							(oldInfo->room != face.info.room
								|| oldInfo->texture != face.info.texture
								|| oldInfo->animated != face.info.animated
								|| oldInfo->blendMode != face.info.blendMode
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->item->ItemNumber != face.info.item->ItemNumber
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->staticMesh != face.info.staticMesh
								|| m_transparentFacesIndices.size() + (face.info.polygon->shape ? 3 : 6) >
								MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
						else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE &&
							(oldInfo->blendMode != face.info.blendMode
								|| oldInfo->sprite->Type != face.info.sprite->Type
								|| oldInfo->sprite->color != face.info.sprite->color
								|| oldInfo->sprite->Sprite != face.info.sprite->Sprite
								|| m_transparentFacesIndices.size() + 6 > MAX_TRANSPARENT_VERTICES))
						{
							outputPolygons = true;
						}
					}
				}

				if (outputPolygons)
				{
					// Here we send polygons to the GPU for drawing and we clear the vertex buffer
					if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
					{
						DrawRoomsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
					{
						DrawItemsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
					{
						DrawStaticsTransparent(oldInfo, view);
					}
					else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
					{
						DrawSpritesTransparent(oldInfo, view);
					}

					outputPolygons = false;
					m_transparentFacesVertices.clear();
					m_transparentFacesIndices.clear();
				}

				oldInfo = &face.info;
				oldType = face.type;

				// Accumulate vertices in the buffer
				if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(roomsIndices.data(), face.info.polygon->baseIndex,
															 numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(moveablesIndices.data(), face.info.polygon->baseIndex,
															 numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
				{
					// For rooms, we already pass world coordinates, just copy vertices
					int numIndices = (face.info.polygon->shape == 0 ? 6 : 3);
					m_transparentFacesIndices.bulk_push_back(staticsIndices.data(), face.info.polygon->baseIndex,
															 numIndices);
				}
				else if (face.type == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
				{
					// For sprites, we need to compute the corners of the quad and multiply 
					// by the world matrix that can be an identity (for 3D sprites) or 
					// a billboard matrix. We transform vertices on the CPU because 
					// CPUs nowadays are fast enough and we save draw calls, in fact 
					// each sprite would require a different world matrix and then 
					// we would fall in the case 1 poly = 1 draw call (worst case scenario).
					// For the same reason, we store also color directly there and we simply 
					// pass a Vector4::One as color to the shader.

					RendererSpriteToDraw* spr = face.info.sprite;

					Vector3 p0t;
					Vector3 p1t;
					Vector3 p2t;
					Vector3 p3t;

					Vector2 uv0;
					Vector2 uv1;
					Vector2 uv2;
					Vector2 uv3;

					if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
					{
						p0t = spr->vtx1;
						p1t = spr->vtx2;
						p2t = spr->vtx3;
						p3t = spr->vtx4;

						uv0 = spr->Sprite->UV[0];
						uv1 = spr->Sprite->UV[1];
						uv2 = spr->Sprite->UV[2];
						uv3 = spr->Sprite->UV[3];
					}
					else
					{
						p0t = Vector3(-0.5, 0.5, 0);
						p1t = Vector3(0.5, 0.5, 0);
						p2t = Vector3(0.5, -0.5, 0);
						p3t = Vector3(-0.5, -0.5, 0);

						uv0 = Vector2(0, 0);
						uv1 = Vector2(1, 0);
						uv2 = Vector2(1, 1);
						uv3 = Vector2(0, 1);
					}

					RendererVertex v0;
					v0.Position = Vector3::Transform(p0t, face.info.world);
					v0.UV = uv0;
					v0.Color = spr->color;

					RendererVertex v1;
					v1.Position = Vector3::Transform(p1t, face.info.world);
					v1.UV = uv1;
					v1.Color = spr->color;

					RendererVertex v2;
					v2.Position = Vector3::Transform(p2t, face.info.world);
					v2.UV = uv2;
					v2.Color = spr->color;

					RendererVertex v3;
					v3.Position = Vector3::Transform(p3t, face.info.world);
					v3.UV = uv3;
					v3.Color = spr->color;

					m_transparentFacesVertices.push_back(v0);
					m_transparentFacesVertices.push_back(v1);
					m_transparentFacesVertices.push_back(v3);
					m_transparentFacesVertices.push_back(v2);
					m_transparentFacesVertices.push_back(v3);
					m_transparentFacesVertices.push_back(v1);
				}
			}

			// Here we send polygons to the GPU for drawing and we clear the vertex buffer
			if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_ROOM)
			{
				DrawRoomsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE)
			{
				DrawItemsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_STATIC)
			{
				DrawStaticsTransparent(oldInfo, view);
			}
			else if (oldType == RendererTransparentFaceType::TRANSPARENT_FACE_SPRITE)
			{
				DrawSpritesTransparent(oldInfo, view);
			}
		}

		SetCullMode(CULL_MODE_CCW);
	}

	void Renderer11::DrawRoomsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		ROOM_INFO* nativeRoom = &g_Level.Rooms[info->room->RoomNumber];

		// Set vertex buffer
		//m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());

		m_context->IASetVertexBuffers(0, 1, m_roomsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		// Set shaders
		if (!info->animated)
		{
			m_context->VSSetShader(m_vsRooms.Get(), nullptr, 0);
		}
		else
		{
			BindConstantBufferVS(CB_ANIMATED_TEXTURES, m_cbAnimated.get());
			m_context->VSSetShader(m_vsRooms_Anim.Get(), nullptr, 0);
		}

		m_context->PSSetShader(m_psRooms.Get(), nullptr, 0);

		// Set texture
		int nmeshes = -Objects[ID_CAUSTICS_TEXTURES].nmeshes;
		int meshIndex = Objects[ID_CAUSTICS_TEXTURES].meshIndex;
		int causticsFrame = nmeshes ? meshIndex + ((GlobalCounter) % nmeshes) : 0;
		BindTexture(TEXTURE_CAUSTICS, m_sprites[causticsFrame].Texture, SAMPLER_NONE);
		BindTexture(TEXTURE_SHADOW_MAP, &m_shadowMap, SAMPLER_SHADOW_MAP);

		// Set shadow map data
		if (shadowLight != nullptr)
		{
			memcpy(&m_stShadowMap.Light, shadowLight, sizeof(ShaderLight));
			m_stShadowMap.ShadowMapSize = g_Configuration.ShadowMapSize;
			m_stShadowMap.CastShadows = true;
			//m_stShadowMap.ViewProjectionInverse = ViewProjection.Invert();
		}
		else
		{
			m_stShadowMap.CastShadows = false;
		}
		m_cbShadowMap.updateData(m_stShadowMap, m_context.Get());
		BindConstantBufferVS(CB_SHADOW_LIGHT, m_cbShadowMap.get());
		BindConstantBufferPS(CB_SHADOW_LIGHT, m_cbShadowMap.get());

		BindLights(view.lightsToDraw);

		m_stMisc.Caustics = (nativeRoom->flags & ENV_FLAG_WATER);
		m_cbMisc.updateData(m_stMisc, m_context.Get());
		BindConstantBufferPS(CB_MISC, m_cbMisc.get());

		m_stRoom.AmbientColor = info->room->AmbientLight;
		m_stRoom.Water = (nativeRoom->flags & ENV_FLAG_WATER) != 0 ? 1 : 0;
		m_cbRoom.updateData(m_stRoom, m_context.Get());

		BindConstantBufferVS(CB_ROOM, m_cbRoom.get());
		BindConstantBufferPS(CB_ROOM, m_cbRoom.get());

		// Draw geometry
		if (info->animated)
		{
			BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_animatedTextures[info->texture]),
			            SAMPLER_ANISOTROPIC_CLAMP);
			BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_animatedTextures[info->texture]),
			            SAMPLER_NONE);

			RendererAnimatedTextureSet& set = m_animatedTextureSets[info->texture];
			m_stAnimated.NumFrames = set.NumTextures;
			m_stAnimated.Type = 0;
			m_stAnimated.Fps = set.Fps;

			for (unsigned char i = 0; i < set.NumTextures; i++)
			{
				auto& tex = set.Textures[i];
				m_stAnimated.Textures[i].topLeft = set.Textures[i].UV[0];
				m_stAnimated.Textures[i].topRight = set.Textures[i].UV[1];
				m_stAnimated.Textures[i].bottomRight = set.Textures[i].UV[2];
				m_stAnimated.Textures[i].bottomLeft = set.Textures[i].UV[3];
			}
			m_cbAnimated.updateData(m_stAnimated, m_context.Get());
		}
		else
		{
			BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_roomTextures[info->texture]),
			            SAMPLER_ANISOTROPIC_CLAMP);
			BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_roomTextures[info->texture]),
			            SAMPLER_NONE);
		}

		SetBlendMode(info->blendMode);
		SetAlphaTest(ALPHA_TEST_NONE, 1.0f);

		m_biggestRoomIndexBuffer = std::fmaxf(m_biggestRoomIndexBuffer, m_transparentFacesIndices.size());

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
							 ? size - drawnVertices
							 : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			DrawIndexedTriangles(count, 0, 0);

			m_numTransparentDrawCalls++;
			m_numRoomsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}

		SetCullMode(CULL_MODE_CCW);
	}

	void Renderer11::DrawStaticsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_staticsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		// Set shaders
		m_context->VSSetShader(m_vsStatics.Get(), nullptr, 0);
		m_context->PSSetShader(m_psStatics.Get(), nullptr, 0);

		// Set texture
		BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_staticsTextures[info->bucket->Texture]),
		            SAMPLER_ANISOTROPIC_CLAMP);
		BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_staticsTextures[info->bucket->Texture]),
		            SAMPLER_NONE);

		m_stStatic.World = info->world;
		m_stStatic.Color = info->color;
		m_stStatic.AmbientLight = info->room->AmbientLight;
		m_stStatic.LightMode = m_staticObjects[info->staticMesh->ObjectNumber]->ObjectMeshes[0]->LightMode;

		m_cbStatic.updateData(m_stStatic, m_context.Get());
		BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
		BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

		BindLights(info->staticMesh->LightsToDraw);

		SetBlendMode(info->blendMode);
		
		SetAlphaTest(ALPHA_TEST_NONE, 1.0f);

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
							 ? size - drawnVertices
							 : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			DrawIndexedTriangles(count, 0, 0);

			m_numTransparentDrawCalls++;
			m_numStaticsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}
	}

	void Renderer11::RenderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
	{
		ResetDebugVariables();
		m_Locked = false;

		using ns = std::chrono::nanoseconds;
		using get_time = std::chrono::steady_clock;

		ScriptInterfaceLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		// Prepare the scene to draw
		auto time1 = std::chrono::high_resolution_clock::now();
		CollectRooms(view, false);
		UpdateLaraAnimations(false);
		UpdateItemAnimations(view);

		m_stAlphaTest.AlphaTest = -1;
		m_stAlphaTest.AlphaThreshold = -1;

		CollectLightsForCamera();
		RenderItemShadows(view);

		auto time2 = std::chrono::high_resolution_clock::now();
		m_timeUpdate = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
		time1 = time2;

		// Reset GPU state
		SetBlendMode(BLENDMODE_OPAQUE, true);
		SetDepthState(DEPTH_STATE_WRITE_ZBUFFER, true);
		SetCullMode(CULL_MODE_CCW, true);

		// Bind and clear render target
		m_context->ClearRenderTargetView(m_renderTarget.RenderTargetView.Get(), Colors::Black);
		m_context->ClearDepthStencilView(m_renderTarget.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		                                 1.0f, 0);

		m_context->ClearRenderTargetView(m_depthMap.RenderTargetView.Get(), Colors::White);
		m_context->ClearDepthStencilView(m_depthMap.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f, 0);

		ID3D11RenderTargetView* m_pRenderViews[2]; 
		m_pRenderViews[0] = m_renderTarget.RenderTargetView.Get(); 
		m_pRenderViews[1] = m_depthMap.RenderTargetView.Get(); 
		m_context->OMSetRenderTargets(2, &m_pRenderViews[0],
		                              m_renderTarget.DepthStencilView.Get());

		m_context->RSSetViewports(1, &view.viewport);
		ResetScissor();

		// The camera constant buffer contains matrices, camera position, fog values and other 
		// things that are shared for all shaders
		CCameraMatrixBuffer cameraConstantBuffer;
		view.fillConstantBuffer(cameraConstantBuffer);
		cameraConstantBuffer.Frame = GlobalCounter;
		cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;

		if (level->GetFogEnabled())
		{
			auto fogCol = level->GetFogColor();
			cameraConstantBuffer.FogColor = Vector4(fogCol.GetR() / 255.0f, fogCol.GetG() / 255.0f, fogCol.GetB() / 255.0f, 1.0f);
			cameraConstantBuffer.FogMinDistance = level->GetFogMinDistance();
			cameraConstantBuffer.FogMaxDistance = level->GetFogMaxDistance();
		}
		else
		{
			cameraConstantBuffer.FogMaxDistance = 0;
		}

		m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
		BindConstantBufferVS(CB_CAMERA, m_cbCameraMatrices.get());
		BindConstantBufferPS(CB_CAMERA, m_cbCameraMatrices.get());

		// Draw the horizon and the sky
		DrawHorizonAndSky(view, m_renderTarget.DepthStencilView.Get());

		// Draw rooms and objects
		DrawRooms(view, false);
		DrawItems(view, false);
		DrawStatics(view, false);
		DrawEffects(view, false);
		DrawGunShells(view);
		DrawDebris(view, false);
		DrawBats(view);
		DrawRats(view);
		DrawSpiders(view);
		DrawScarabs(view);
		DrawLocusts(view);

		m_context->OMSetRenderTargets(1, m_renderTarget.RenderTargetView.GetAddressOf(),
			m_renderTarget.DepthStencilView.Get());

		// Do special effects and weather 
		// NOTE: functions here just fill the sprites to draw array
		DrawFires(view);
		DrawSmokes(view);
		DrawSmokeParticles(view);
		DrawSimpleParticles(view);
		DrawSparkParticles(view);
		DrawExplosionParticles(view);
		DrawFootprints(view);
		DrawDripParticles(view);
		DrawBlood(view);
		DrawWeatherParticles(view);
		DrawParticles(view);
		DrawBubbles(view);
		DrawDrips(view);
		DrawRipples(view);
		DrawSplashes(view);
		DrawShockwaves(view);
		DrawLightning(view);
		DrawRopes(view);

		// Here is where we actually output sprites
		DrawSprites(view);
		DrawLines3D(view);

		// Here we sort transparent faces and draw them with a simplified shaders for alpha blending
		DrawRooms(view, true);
		DrawItems(view, true);
		DrawStatics(view, true);
		DrawEffects(view, true);
		DrawDebris(view, true);
		DrawGunFlashes(view);
		DrawBaddyGunflashes(view);

		DrawTransparentFaces(view);

		DrawPostprocess(target, depthTarget, view);

		// Draw GUI stuff at the end
		DrawLines2D();

		// Bars
		DrawHUD(LaraItem);

		// Draw binoculars or lasersight
		DrawOverlays(view); 

		time2 = std::chrono::high_resolution_clock::now();
		m_timeFrame = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
		time1 = time2;

		DrawDebugInfo(view);
		DrawAllStrings();

		ClearScene();
	}

	void Renderer11::RenderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
	                                   RenderView& view)
	{
		ScriptInterfaceLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		CollectRooms(view, true);
		// Draw shadow map

		// Reset GPU state
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);

		// Bind and clear render target

		m_context->ClearRenderTargetView(target, Colors::Black);
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		m_context->OMSetRenderTargets(1, &target, depthTarget);

		m_context->RSSetViewports(1, &view.viewport);
		ResetScissor();

		// Opaque geometry
		SetBlendMode(BLENDMODE_OPAQUE);
		SetCullMode(CULL_MODE_CCW);

		CCameraMatrixBuffer cameraConstantBuffer;
		view.fillConstantBuffer(cameraConstantBuffer);
		cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;
		m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
		BindConstantBufferVS(CB_CAMERA, m_cbCameraMatrices.get());
		DrawHorizonAndSky(view, depthTarget);
		DrawRooms(view, false);
	}

	void Renderer11::DumpGameScene()
	{
		RenderScene(m_dumpScreenRenderTarget.RenderTargetView.Get(), m_dumpScreenRenderTarget.DepthStencilView.Get(),
		            gameCamera);
	}

	void Renderer11::DrawItems(RenderView& view, bool transparent)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set shaders
		m_context->VSSetShader(m_vsItems.Get(), nullptr, 0);
		m_context->PSSetShader(m_psItems.Get(), nullptr, 0);

		for (auto room : view.roomsToDraw)
		{
			for (auto itemToDraw : room->ItemsToDraw)
			{
				switch (itemToDraw->ObjectNumber)
				{
				case ID_LARA:
					DrawLara(view, transparent);
					break;

				case ID_DARTS:
					DrawDarts(itemToDraw, view);
					break;

				case ID_WATERFALL1:
				case ID_WATERFALL2:
				case ID_WATERFALL3:
				case ID_WATERFALL4:
				case ID_WATERFALL5:
				case ID_WATERFALL6:
				case ID_WATERFALLSS1:
				case ID_WATERFALLSS2:
					DrawWaterfalls(itemToDraw, view, 10, transparent);
					continue;

				default:
					DrawAnimatingItem(itemToDraw, view, transparent);
					break;
				}
			}
		}
	}

	void Renderer11::RenderItemShadows(RenderView& renderView)
	{
		RenderBlobShadows(renderView);

		if (g_Configuration.ShadowType != ShadowMode::None)
		{
			for (auto room : renderView.roomsToDraw)
				for (auto itemToDraw : room->ItemsToDraw)
					RenderShadowMap(itemToDraw, renderView);
		}
	}

	void Renderer11::DrawWaterfalls(RendererItem* item, RenderView& view, int fps, bool transparent)
	{
		// Extremely hacky function to get first rendered face of a waterfall object mesh, calculate
		// its texture height and scroll all the textures according to that height.

		RendererObject& moveableObj = *m_moveableObjects[item->ObjectNumber];

		// No mesh or bucket, abort
		if (!moveableObj.ObjectMeshes.size() || !moveableObj.ObjectMeshes[0]->Buckets.size())
			return;

		// Get first three vertices of a waterfall object, meaning the very first triangle
		const auto& v1 = moveablesVertices[moveableObj.ObjectMeshes[0]->Buckets[0].StartVertex + 0];
		const auto& v2 = moveablesVertices[moveableObj.ObjectMeshes[0]->Buckets[0].StartVertex + 1];
		const auto& v3 = moveablesVertices[moveableObj.ObjectMeshes[0]->Buckets[0].StartVertex + 2];

		// Calculate height of the texture by getting min/max UV.y coords of all three vertices
		auto minY = std::min(std::min(v1.UV.y, v2.UV.y), v3.UV.y);
		auto maxY = std::max(std::max(v1.UV.y, v2.UV.y), v3.UV.y);
		auto minX = std::min(std::min(v1.UV.x, v2.UV.x), v3.UV.x);
		auto maxX = std::max(std::max(v1.UV.x, v2.UV.x), v3.UV.x);

		// Setup animated buffer
		m_stAnimated.Fps = fps;
		m_stAnimated.NumFrames = 1;
		m_stAnimated.Type = 1; // UVRotate

		// We need only top/bottom Y coordinate for UVRotate, but we pass whole
		// rectangle anyway, in case later we may want to implement different UVRotate modes.
		m_stAnimated.Textures[0].topLeft     = Vector2(minX, minY);
		m_stAnimated.Textures[0].topRight    = Vector2(maxX, minY);
		m_stAnimated.Textures[0].bottomLeft  = Vector2(minX, maxY);
		m_stAnimated.Textures[0].bottomRight = Vector2(maxX, maxY);
		
		m_cbAnimated.updateData(m_stAnimated, m_context.Get());
		BindConstantBufferPS(CB_ANIMATED_TEXTURES, m_cbAnimated.get());

		DrawAnimatingItem(item, view, transparent);

		// Reset animated buffer after rendering just in case
		m_stAnimated.Fps = m_stAnimated.NumFrames = m_stAnimated.Type = 0;
		m_cbAnimated.updateData(m_stAnimated, m_context.Get());
	}

	void Renderer11::DrawAnimatingItem(RendererItem* item, RenderView& view, bool transparent)
	{
		ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];
		RendererRoom* room = &m_rooms[item->RoomNumber];
		RendererObject& moveableObj = *m_moveableObjects[item->ObjectNumber];
		ObjectInfo* obj = &Objects[item->ObjectNumber];

		// Bind item main properties
		m_stItem.World = item->World;
		m_stItem.Color = item->Color;
		m_stItem.AmbientLight = item->AmbientLight;
		memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * MAX_BONES);
		for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
			m_stItem.BoneLightModes[k] = moveableObj.ObjectMeshes[k]->LightMode;

		m_cbItem.updateData(m_stItem, m_context.Get());
		BindConstantBufferVS(CB_ITEM, m_cbItem.get());
		BindConstantBufferPS(CB_ITEM, m_cbItem.get());

		// Bind lights touching that item
		BindLights(item->LightsToDraw, item->RoomNumber, item->PrevRoomNumber, item->LightFade);

		for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
		{
			if (!(nativeItem->MeshBits & (1 << k)))
				continue;

			RendererMesh* mesh = moveableObj.ObjectMeshes[k];

			// Do the swapmesh
			if (obj->meshSwapSlot != -1 && m_moveableObjects[obj->meshSwapSlot].has_value() && ((nativeItem->MeshSwapBits >> k) & 1))
			{
				RendererObject& swapMeshObj = *m_moveableObjects[obj->meshSwapSlot];
				if (swapMeshObj.ObjectMeshes.size() > k)
					mesh = swapMeshObj.ObjectMeshes[k];
			}

			DrawMoveableMesh(item, mesh, room, k, transparent);
		}
	}

	void Renderer11::DrawItemsTransparent(RendererTransparentFaceInfo* info, RenderView& view)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Set vertex buffer
		//m_transparentFacesVertexBuffer.Update(m_context.Get(), m_transparentFacesVertices, 0, m_transparentFacesVertices.size());

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		ItemInfo* nativeItem = &g_Level.Items[info->item->ItemNumber];
		RendererRoom& room = m_rooms[nativeItem->RoomNumber];
		RendererObject& moveableObj = *m_moveableObjects[nativeItem->ObjectNumber];
		ObjectInfo* obj = &Objects[nativeItem->ObjectNumber];

		// Set shaders
		m_context->VSSetShader(m_vsItems.Get(), nullptr, 0);
		m_context->PSSetShader(m_psItems.Get(), nullptr, 0);

		m_stItem.World = info->item->World;
		m_stItem.Color = info->color;
		m_stItem.AmbientLight = info->item->AmbientLight;
		memcpy(m_stItem.BonesMatrices, info->item->AnimationTransforms, sizeof(Matrix) * MAX_BONES);
		for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
			m_stItem.BoneLightModes[k] = moveableObj.ObjectMeshes[k]->LightMode;

		m_cbItem.updateData(m_stItem, m_context.Get());
		BindConstantBufferVS(CB_ITEM, m_cbItem.get());
		BindConstantBufferPS(CB_ITEM, m_cbItem.get());

		BindLights(info->item->LightsToDraw, info->item->RoomNumber, info->item->PrevRoomNumber, info->item->LightFade);

		// Set texture
		BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[info->bucket->Texture]),
		            SAMPLER_ANISOTROPIC_CLAMP);
		BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[info->bucket->Texture]),
		            SAMPLER_NONE);

		SetBlendMode(info->blendMode);

		SetAlphaTest(ALPHA_TEST_NONE, 1.0f);

		int drawnVertices = 0;
		int size = m_transparentFacesIndices.size();

		while (drawnVertices < size)
		{
			int count = (drawnVertices + TRANSPARENT_BUCKET_SIZE > size
							 ? size - drawnVertices
							 : TRANSPARENT_BUCKET_SIZE);

			m_transparentFacesIndexBuffer.Update(m_context.Get(), m_transparentFacesIndices, drawnVertices, count);
			m_context->IASetIndexBuffer(m_transparentFacesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			DrawIndexedTriangles(count, 0, 0);

			m_numTransparentDrawCalls++;
			m_numStaticsTransparentDrawCalls++;

			drawnVertices += TRANSPARENT_BUCKET_SIZE;
		}
	}

	void Renderer11::DrawDarts(RendererItem* item, RenderView& view)
	{
		ItemInfo* nativeItem = &g_Level.Items[item->ItemNumber];

		Vector3 start = Vector3(
			nativeItem->Pose.Position.x,
			nativeItem->Pose.Position.y,
			nativeItem->Pose.Position.z);

		float speed = (-96 * phd_cos(TO_RAD(nativeItem->Pose.Orientation.x)));

		Vector3 end = Vector3(
			nativeItem->Pose.Position.x + speed * phd_sin(TO_RAD(nativeItem->Pose.Orientation.y)),
			nativeItem->Pose.Position.y + 96 * phd_sin(TO_RAD(nativeItem->Pose.Orientation.x)),
			nativeItem->Pose.Position.z + speed * phd_cos(TO_RAD(nativeItem->Pose.Orientation.y)));

		AddLine3D(start, end, Vector4(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 0.5f));
	}

	void Renderer11::DrawStatics(RenderView& view, bool transparent)
	{
		// Static mesh shader is used for all forthcoming renderer routines, so we
		// must assign it before any early exits.
		m_context->VSSetShader(m_vsStatics.Get(), nullptr, 0);
		m_context->PSSetShader(m_psStatics.Get(), nullptr, 0);

		// If no static textures are loaded, don't draw anything.
		if (m_staticsTextures.size() == 0)
			return;

		// Bind vertex and index buffer
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_staticsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_staticsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		for (auto room : view.roomsToDraw)
		{
			for (auto& msh : room->StaticsToDraw)
			{
				RendererObject& staticObj = *m_staticObjects[msh.ObjectNumber];

				if (staticObj.ObjectMeshes.size() > 0)
				{
					RendererMesh* mesh = staticObj.ObjectMeshes[0]; 

					for (auto& bucket : mesh->Buckets)
					{
						if (!((bucket.BlendMode == BLENDMODE_OPAQUE || bucket.BlendMode == BLENDMODE_ALPHATEST) ^ transparent))
						{
							continue;
						}

						if (bucket.NumVertices == 0)
						{
							continue;
						}

						if (DoesBlendModeRequireSorting(bucket.BlendMode))
						{
							// Collect transparent faces
							for (int j = 0; j < bucket.Polygons.size(); j++)
							{
								RendererPolygon* p = &bucket.Polygons[j];

								// As polygon distance, for moveables, we use the averaged distance
								Vector3 centre = Vector3::Transform(p->centre, msh.World);
								int distance = (centre - cameraPosition).Length();

								RendererTransparentFace face;
								face.type = RendererTransparentFaceType::TRANSPARENT_FACE_STATIC;
								face.info.polygon = p;
								face.distance = distance;
								face.info.animated = bucket.Animated;
								face.info.texture = bucket.Texture;
								face.info.room = room;
								face.info.staticMesh = &msh;
								face.info.world = m_stStatic.World;
								face.info.position = msh.Position;
								face.info.color = msh.Color;
								face.info.blendMode = bucket.BlendMode;
								face.info.bucket = &bucket;
								room->TransparentFacesToDraw.push_back(face);
							}
						}
						else
						{
							m_stStatic.World = msh.World;
							m_stStatic.Color = msh.Color;
							m_stStatic.AmbientLight = room->AmbientLight;
							m_stStatic.LightMode = mesh->LightMode;

							m_cbStatic.updateData(m_stStatic, m_context.Get());
							BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
							BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

							BindLights(msh.LightsToDraw);

							int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

							for (int pass = 0; pass < passes; pass++)
							{
								if (pass == 0)
								{
									SetBlendMode(bucket.BlendMode);
									SetAlphaTest(
										bucket.BlendMode == BLENDMODE_ALPHATEST ? ALPHA_TEST_GREATER_THAN : ALPHA_TEST_NONE,
										ALPHA_TEST_THRESHOLD
									);
								}
								else
								{
									SetBlendMode(BLENDMODE_ALPHABLEND);
									SetAlphaTest(ALPHA_TEST_LESS_THAN, FAST_ALPHA_BLEND_THRESHOLD);
								}

								BindTexture(TEXTURE_COLOR_MAP,
								            &std::get<0>(m_staticsTextures[bucket.Texture]),
								            SAMPLER_ANISOTROPIC_CLAMP);
								BindTexture(TEXTURE_NORMAL_MAP,
								            &std::get<1>(m_staticsTextures[bucket.Texture]), SAMPLER_NONE);

								DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

								m_numStaticsDrawCalls++;
							}
						}
					}
				}
			}
		}
	}

	void Renderer11::DrawRooms(RenderView& view, bool transparent)
	{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Bind vertex and index buffer
		m_context->IASetVertexBuffers(0, 1, m_roomsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_roomsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Bind pixel shaders
		m_context->PSSetShader(m_psRooms.Get(), nullptr, 0);

		// Bind caustics and shadow map textures
		int nmeshes = -Objects[ID_CAUSTICS_TEXTURES].nmeshes;
		int meshIndex = Objects[ID_CAUSTICS_TEXTURES].meshIndex;
		int causticsFrame = nmeshes ? meshIndex + ((GlobalCounter) % nmeshes) : 0;

		// BindTexture(TEXTURE_CAUSTICS, m_sprites[causticsFrame].Texture, SAMPLER_NONE);

		// Set shadow map data
		if (shadowLight != nullptr)
		{
			memcpy(&m_stShadowMap.Light, shadowLight, sizeof(ShaderLight));
			m_stShadowMap.ShadowMapSize = g_Configuration.ShadowMapSize;
			m_stShadowMap.CastShadows = true;

			BindTexture(TEXTURE_SHADOW_MAP, &m_shadowMap, SAMPLER_SHADOW_MAP);
		}
		else
		{
			m_stShadowMap.CastShadows = false;
		}

		m_numRoomsTransparentPolygons = 0;
		for (int i = view.roomsToDraw.size() - 1; i >= 0; i--)
		{
			int index = i;
			RendererRoom* room = view.roomsToDraw[index];
			m_cbShadowMap.updateData(m_stShadowMap, m_context.Get());

			BindConstantBufferPS(CB_SHADOW_LIGHT, m_cbShadowMap.get());
			BindConstantBufferVS(CB_SHADOW_LIGHT, m_cbShadowMap.get());

			ROOM_INFO* nativeRoom = &g_Level.Rooms[room->RoomNumber];

			Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);
			Vector3 roomPosition = Vector3(nativeRoom->x, nativeRoom->y, nativeRoom->z);

			BindLights(view.lightsToDraw);

			// TODO: make caustics optional in Tomb Editor
			m_stMisc.Caustics = false; // (nativeRoom->flags & ENV_FLAG_WATER);

			m_stRoom.AmbientColor = room->AmbientLight;
			m_stRoom.Water = (nativeRoom->flags & ENV_FLAG_WATER) != 0 ? 1 : 0;
			m_cbRoom.updateData(m_stRoom, m_context.Get());
			BindConstantBufferVS(CB_ROOM, m_cbRoom.get());
			BindConstantBufferPS(CB_ROOM, m_cbRoom.get());

			SetScissor(room->Clip);

			for (int animated = 0; animated < 2; animated++)
			{
				if (animated == 0)
				{
					m_context->VSSetShader(m_vsRooms.Get(), nullptr, 0);
				}
				else
				{
					m_context->VSSetShader(m_vsRooms_Anim.Get(), nullptr, 0);
					BindConstantBufferVS(CB_ANIMATED_TEXTURES, m_cbAnimated.get());
				}

				for (auto& bucket : room->Buckets)
				{
					if ((animated == 1) ^ bucket.Animated || bucket.NumVertices == 0)
					{
						continue;
					}

					if (!((bucket.BlendMode == BLENDMODE_OPAQUE || bucket.BlendMode == BLENDMODE_ALPHATEST) ^ transparent))
					{
						continue;
					}

					if (DoesBlendModeRequireSorting(bucket.BlendMode))
					{
						// Collect transparent faces
						for (int j = 0; j < bucket.Polygons.size(); j++)
						{
							RendererPolygon* p = &bucket.Polygons[j];

							m_numRoomsTransparentPolygons++;

							// As polygon distance, for rooms, we use the farthest vertex distance                            
							int d1 = (roomsVertices[roomsIndices[p->baseIndex + 0]].Position - cameraPosition).Length();
							int d2 = (roomsVertices[roomsIndices[p->baseIndex + 1]].Position - cameraPosition).Length();
							int d3 = (roomsVertices[roomsIndices[p->baseIndex + 2]].Position - cameraPosition).Length();
							int d4 = 0;
							if (p->shape == 0)
								d4 = (roomsVertices[roomsIndices[p->baseIndex + 3]].Position - cameraPosition).Length();

							int distance = std::max(std::max(std::max(d1, d2), d3), d4);

							RendererTransparentFace face;
							face.type = RendererTransparentFaceType::TRANSPARENT_FACE_ROOM;
							face.info.polygon = p;
							face.distance = distance;
							face.info.animated = bucket.Animated;
							face.info.texture = bucket.Texture;
							face.info.room = room;
							face.info.blendMode = bucket.BlendMode;
							face.info.bucket = &bucket;
							room->TransparentFacesToDraw.push_back(face);
						}
					}
					else
					{
						int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

						for (int pass = 0; pass < passes; pass++)
						{
							if (pass == 0)
							{
								SetBlendMode(bucket.BlendMode);
								SetAlphaTest(
									bucket.BlendMode == BLENDMODE_ALPHATEST ? ALPHA_TEST_GREATER_THAN : ALPHA_TEST_NONE,
									FAST_ALPHA_BLEND_THRESHOLD
								);
							}
							else
							{
								SetBlendMode(BLENDMODE_ALPHABLEND);
								SetAlphaTest(ALPHA_TEST_LESS_THAN, FAST_ALPHA_BLEND_THRESHOLD);
							}

							// Draw geometry
							if (animated)
							{
								BindTexture(TEXTURE_COLOR_MAP,
								            &std::get<0>(m_animatedTextures[bucket.Texture]),
								            SAMPLER_ANISOTROPIC_CLAMP);
								BindTexture(TEXTURE_NORMAL_MAP,
								            &std::get<1>(m_animatedTextures[bucket.Texture]), SAMPLER_NONE);

								RendererAnimatedTextureSet& set = m_animatedTextureSets[bucket.Texture];
								m_stAnimated.NumFrames = set.NumTextures;
								m_stAnimated.Type = 0;
								m_stAnimated.Fps = set.Fps;

								for (unsigned char j = 0; j < set.NumTextures; j++)
								{
									if (j >= m_stAnimated.Textures.size())
									{
										TENLog("Animated frame " + std::to_string(j) + " is out of bounds, too many frames in sequence.");
										break;
									}

									auto& tex = set.Textures[j];
									m_stAnimated.Textures[j].topLeft = set.Textures[j].UV[0];
									m_stAnimated.Textures[j].topRight = set.Textures[j].UV[1];
									m_stAnimated.Textures[j].bottomRight = set.Textures[j].UV[2];
									m_stAnimated.Textures[j].bottomLeft = set.Textures[j].UV[3];
								}
								m_cbAnimated.updateData(m_stAnimated, m_context.Get());
							}
							else
							{
								BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_roomTextures[bucket.Texture]),
								            SAMPLER_ANISOTROPIC_CLAMP);
								BindTexture(TEXTURE_NORMAL_MAP,
								            &std::get<1>(m_roomTextures[bucket.Texture]), SAMPLER_NONE);
							}

							DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

							m_numRoomsDrawCalls++;
						}
					}
				}
			}
		}

		ResetScissor();
	}
	
	void Renderer11::DrawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget)
	{
		ScriptInterfaceLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		bool anyOutsideRooms = false;
		for (int k = 0; k < renderView.roomsToDraw.size(); k++)
		{
			ROOM_INFO* nativeRoom = &g_Level.Rooms[renderView.roomsToDraw[k]->RoomNumber];
			if (nativeRoom->flags & ENV_FLAG_OUTSIDE)
			{
				anyOutsideRooms = true;
				break;
			}
		}

		if (!level->Horizon || !anyOutsideRooms)
			return;

		if (BinocularRange)
			AlterFOV(14560 - BinocularRange);

		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		// Draw the sky
		Matrix rotation = Matrix::CreateRotationX(PI);

		RendererVertex vertices[4];
		float size = 9728.0f;

		vertices[0].Position.x = -size / 2.0f;
		vertices[0].Position.y = 0.0f;
		vertices[0].Position.z = size / 2.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color.x = 1.0f;
		vertices[0].Color.y = 1.0f;
		vertices[0].Color.z = 1.0f;
		vertices[0].Color.w = 1.0f;

		vertices[1].Position.x = size / 2.0f;
		vertices[1].Position.y = 0.0f;
		vertices[1].Position.z = size / 2.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color.x = 1.0f;
		vertices[1].Color.y = 1.0f;
		vertices[1].Color.z = 1.0f;
		vertices[1].Color.w = 1.0f;

		vertices[2].Position.x = size / 2.0f;
		vertices[2].Position.y = 0.0f;
		vertices[2].Position.z = -size / 2.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color.x = 1.0f;
		vertices[2].Color.y = 1.0f;
		vertices[2].Color.z = 1.0f;
		vertices[2].Color.w = 1.0f;

		vertices[3].Position.x = -size / 2.0f;
		vertices[3].Position.y = 0.0f;
		vertices[3].Position.z = -size / 2.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color.x = 1.0f;
		vertices[3].Color.y = 1.0f;
		vertices[3].Color.z = 1.0f;
		vertices[3].Color.w = 1.0f;

		m_context->VSSetShader(m_vsSky.Get(), nullptr, 0);
		m_context->PSSetShader(m_psSky.Get(), nullptr, 0);

		BindTexture(TEXTURE_COLOR_MAP, &m_skyTexture, SAMPLER_ANISOTROPIC_CLAMP);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());

		SetBlendMode(BLENDMODE_ADDITIVE);

		for (int s = 0; s < 2; s++)
			for (int i = 0; i < 2; i++)
			{
				auto weather = TEN::Effects::Environment::Weather;

				Matrix translation = Matrix::CreateTranslation(Camera.pos.x + weather.SkyPosition(s) - i * 9728.0f,
															   Camera.pos.y - 1536.0f, Camera.pos.z);
				Matrix world = rotation * translation;

				m_stStatic.World = (rotation * translation);
				m_stStatic.Color = weather.SkyColor(s);
				m_stStatic.AmbientLight = Vector4::One;
				m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_STATIC;

				m_cbStatic.updateData(m_stStatic, m_context.Get());
				BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
				BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

				m_primitiveBatch->Begin();
				m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
				m_primitiveBatch->End();
			}
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

		// Draw horizon
		if (m_moveableObjects[ID_HORIZON].has_value())
		{
			m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());
			m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

			RendererObject& moveableObj = *m_moveableObjects[ID_HORIZON];

			m_stStatic.World = Matrix::CreateTranslation(Camera.pos.x, Camera.pos.y, Camera.pos.z);
			m_stStatic.Color = Vector4::One;
			m_stStatic.AmbientLight = Vector4::One;
			m_stStatic.LightMode = LIGHT_MODES::LIGHT_MODE_STATIC;

			m_cbStatic.updateData(m_stStatic, m_context.Get());
			BindConstantBufferVS(CB_STATIC, m_cbStatic.get());
			BindConstantBufferPS(CB_STATIC, m_cbStatic.get());

			for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
			{
				RendererMesh* mesh = moveableObj.ObjectMeshes[k];

				for (auto& bucket : mesh->Buckets)
				{
					if (bucket.NumVertices == 0)
						continue;

					BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[bucket.Texture]),
					            SAMPLER_ANISOTROPIC_CLAMP);
					BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[bucket.Texture]),
					            SAMPLER_NONE);

					// Always render horizon as alpha-blended surface
					SetBlendMode(bucket.BlendMode == BLEND_MODES::BLENDMODE_ALPHATEST ? BLEND_MODES::BLENDMODE_ALPHABLEND : bucket.BlendMode);
					SetAlphaTest(ALPHA_TEST_NONE, ALPHA_TEST_THRESHOLD);

					// Draw vertices
					DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

					m_numMoveablesDrawCalls++;
				}
			}
		}

		// Clear just the Z-buffer so we can start drawing on top of the horizon
		m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	void Renderer11::Draw()
	{
		//RenderToCubemap(m_reflectionCubemap, Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos - 1024, LaraItem->pos.zPos), LaraItem->roomNumber);
		RenderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
		m_context->ClearState();
		m_swapChain->Present(1, 0);
	}

	void Renderer11::DrawMoveableMesh(RendererItem* itemToDraw, RendererMesh* mesh, RendererRoom* room, int boneIndex, bool transparent)
	{
		Vector3 cameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		for (auto& bucket : mesh->Buckets)
		{
			if (!((bucket.BlendMode == BLENDMODE_OPAQUE || bucket.BlendMode == BLENDMODE_ALPHATEST) ^ transparent))
			{
				continue;
			}

			if (bucket.NumVertices == 0)
			{
				continue;
			}

			if (DoesBlendModeRequireSorting(bucket.BlendMode))
			{
				// Collect transparent faces
				for (int j = 0; j < bucket.Polygons.size(); j++)
				{
					RendererPolygon* p = &bucket.Polygons[j];

					// As polygon distance, for moveables, we use the averaged distance
					Vector3 centre = Vector3::Transform(
						p->centre, itemToDraw->AnimationTransforms[boneIndex] * itemToDraw->World);
					int distance = (centre - cameraPosition).Length();

					RendererTransparentFace face;
					face.type = RendererTransparentFaceType::TRANSPARENT_FACE_MOVEABLE;
					face.info.polygon = p;
					face.distance = distance;
					face.info.animated = bucket.Animated;
					face.info.texture = bucket.Texture;
					face.info.room = room;
					face.info.item = itemToDraw;
					face.info.color = itemToDraw->Color;
					face.info.blendMode = bucket.BlendMode;
					face.info.bucket = &bucket;
					room->TransparentFacesToDraw.push_back(face);
				}
			}
			else
			{
				int passes = bucket.BlendMode == BLENDMODE_ALPHATEST ? 2 : 1;

				BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[bucket.Texture]),
				            SAMPLER_ANISOTROPIC_CLAMP);
				BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[bucket.Texture]),
				            SAMPLER_NONE);

				for (int pass = 0; pass < passes; pass++)
				{
					if (pass == 0)
					{
						SetBlendMode(bucket.BlendMode);
						SetAlphaTest(
							bucket.BlendMode == BLENDMODE_ALPHATEST ? ALPHA_TEST_GREATER_THAN : ALPHA_TEST_NONE,
							ALPHA_TEST_THRESHOLD
						);
					}
					else
					{
						SetBlendMode(BLENDMODE_ALPHABLEND);
						SetAlphaTest(ALPHA_TEST_LESS_THAN, FAST_ALPHA_BLEND_THRESHOLD);
					}

					DrawIndexedTriangles(bucket.NumIndices, bucket.StartIndex, 0);

					m_numMoveablesDrawCalls++;
				}
			}
		}
	}
}
