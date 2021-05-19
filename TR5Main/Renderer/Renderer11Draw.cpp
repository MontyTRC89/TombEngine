#include "framework.h"
#include "Renderer11.h"
#include "configuration.h"
#include "savegame.h"
#include "health.h"
#include "camera.h"
#include "draw.h"
#include "inventory.h"
#include "newinv2.h"
#include "lara.h"
#include "gameflow.h"
#include "rope.h"
#include "tomb4fx.h"
#include "door.h"
#include "level.h"
#include "setup.h"
#include "control.h"
#include "sound.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include <Objects\TR4\Entity\tr4_wraith.h>
#include <Objects\TR4\Entity\tr4_littlebeetle.h>
#include "RenderView/RenderView.h"
#include "hair.h"
#include "winmain.h"
#include <chrono>
extern T5M::Renderer::RendererHUDBar *g_DashBar;
extern T5M::Renderer::RendererHUDBar *g_SFXVolumeBar;
extern T5M::Renderer::RendererHUDBar *g_MusicVolumeBar;
extern GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL];

namespace T5M::Renderer
{
    using namespace T5M::Renderer;
    using namespace std::chrono;

    void Renderer11::drawPickup(short objectNum)
    {
        drawObjectOn2DPosition(700 + PickupX, 450, objectNum, 0, m_pickupRotation, 0); // TODO: + PickupY
        m_pickupRotation += 45 * 360 / 30;
    }

    void Renderer11::drawObjectOn2DPosition(short x, short y, short objectNum, short rotX, short rotY, short rotZ)
    {
        Matrix translation;
        Matrix rotation;
        Matrix world;
        Matrix view;
        Matrix projection;
        Matrix scale;

        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        x *= (ScreenWidth / 800.0f);
        y *= (ScreenHeight / 600.0f);

        view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 2048.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
        projection = Matrix::CreateOrthographic(ScreenWidth, ScreenHeight, -1024.0f, 1024.0f);

        OBJECT_INFO *obj = &Objects[objectNum];
        RendererObject &moveableObj = *m_moveableObjects[objectNum];

        if (obj->animIndex != -1)
        {
            ANIM_FRAME *frame[] = {&g_Level.Frames[g_Level.Anims[obj->animIndex].framePtr]};
            updateAnimation(NULL, moveableObj, frame, 0, 0, 0xFFFFFFFF);
        }

        Vector3 pos = m_viewportToolkit.Unproject(Vector3(x, y, 1), projection, view, Matrix::Identity);

        // Clear just the Z-buffer so we can start drawing on top of the scene
        ID3D11DepthStencilView *dsv;
        m_context->OMGetRenderTargets(1, NULL, &dsv);
        m_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // Set vertex buffer
        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Set shaders
        m_context->VSSetShader(m_vsInventory.Get(), NULL, 0);
        m_context->PSSetShader(m_psInventory.Get(), NULL, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());

        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        // Set matrices
        CCameraMatrixBuffer HudCamera;
        HudCamera.ViewProjection = view * projection;
        m_cbCameraMatrices.updateData(HudCamera, m_context.Get());
        m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

        for (int n = 0; n < moveableObj.ObjectMeshes.size(); n++)
        {
            RendererMesh *mesh = moveableObj.ObjectMeshes[n];

            // Finish the world matrix
            translation = Matrix::CreateTranslation(pos.x, pos.y, pos.z + 1024.0f);
            rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(rotY), TO_RAD(rotX), TO_RAD(rotZ));
            scale = Matrix::CreateScale(0.5f);

            world = scale * rotation;
            world = world * translation;

            if (obj->animIndex != -1)
                m_stItem.World = (moveableObj.AnimationTransforms[n] * world);
            else
                m_stItem.World = (moveableObj.BindPoseTransforms[n] * world);
            m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
            m_cbItem.updateData(m_stItem, m_context.Get());
            m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
            m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

            for (int m = 0; m < NUM_BUCKETS; m++)
            {
                RendererBucket *bucket = &mesh->Buckets[m];
                if (bucket->Vertices.size() == 0)
                    continue;

                if (m < 2)
                    m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
                else
                    m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

                m_stMisc.AlphaTest = (m < 2);
                m_cbMisc.updateData(m_stMisc, m_context.Get());
                m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

                m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
            }
        }

    }

    void Renderer11::renderShadowMap(RenderView& renderView)
    {
        m_shadowLight = NULL;
        RendererLight *brightestLight = NULL;
        float brightest = 0.0f;
        Vector3 itemPosition = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);

        for (int k = 0; k < renderView.roomsToDraw.size(); k++)
        {
            RendererRoom *room = renderView.roomsToDraw[k];
            int numLights = room->Lights.size();

            for (int j = 0; j < numLights; j++)
            {
                RendererLight *light = &room->Lights[j];

                // Check only lights different from sun
                if (light->Type == LIGHT_TYPE_SUN)
                {
                    // Sun is added without checks
                }
                else if (light->Type == LIGHT_TYPE_POINT)
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

                    if (intensity >= brightest)
                    {
                        brightest = intensity;
                        brightestLight = light;
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
                    float attenuation = 1.0f - distance / light->Range;
                    float intensity = std::max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

                    if (intensity >= brightest)
                    {
                        brightest = intensity;
                        brightestLight = light;
                    }
                }
                else
                {
                    // Invalid light type
                    continue;
                }
            }
        }

        m_shadowLight = brightestLight;

        if (m_shadowLight == NULL)
            return;

        // Reset GPU state
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

        // Bind and clear render target
        m_context->ClearRenderTargetView(m_shadowMap.RenderTargetView.Get(), Colors::White);
        m_context->ClearDepthStencilView(m_shadowMap.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_context->OMSetRenderTargets(1, m_shadowMap.RenderTargetView.GetAddressOf(), m_shadowMap.DepthStencilView.Get());

        m_context->RSSetViewports(1, &m_shadowMapViewport);

        //drawLara(false, true);

        Vector3 lightPos = Vector3(m_shadowLight->Position.x, m_shadowLight->Position.y, m_shadowLight->Position.z);
        Vector3 itemPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
        if (lightPos == itemPos)
            return;

        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        // Set shaders
        m_context->VSSetShader(m_vsShadowMap.Get(), NULL, 0);
        m_context->PSSetShader(m_psShadowMap.Get(), NULL, 0);

        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());

        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        // Set camera matrices
        Matrix view = Matrix::CreateLookAt(lightPos,
                                           itemPos,
                                           Vector3(0.0f, -1.0f, 0.0f));
        Matrix projection = Matrix::CreatePerspectiveFieldOfView(90.0f * RADIAN, 1.0f, 64.0f,
                                                                 (m_shadowLight->Type == LIGHT_TYPE_POINT ? m_shadowLight->Out : m_shadowLight->Range) * 1.2f);
        CCameraMatrixBuffer shadowProjection;
        shadowProjection.ViewProjection = view * projection;
        m_cbCameraMatrices.updateData(shadowProjection, m_context.Get());
        m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

        m_stShadowMap.LightViewProjection = (view * projection);

        m_stMisc.AlphaTest = true;
        m_cbMisc.updateData(m_stMisc, m_context.Get());
        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

        RendererObject &laraObj = *m_moveableObjects[ID_LARA];
        RendererObject &laraSkin = *m_moveableObjects[ID_LARA_SKIN];
        RendererRoom &const room = m_rooms[LaraItem->roomNumber];

        m_stItem.World = m_LaraWorldMatrix;
        m_stItem.Position = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
        m_stItem.AmbientLight = room.AmbientLight;
        memcpy(m_stItem.BonesMatrices, laraObj.AnimationTransforms.data(), sizeof(Matrix) * 32);
        m_cbItem.updateData(m_stItem, m_context.Get());
        m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
        m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

        for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
        {
            RendererMesh *mesh = getMesh(Lara.meshPtrs[k]);

            for (int j = 0; j < 2; j++)
            {
                RendererBucket *bucket = &mesh->Buckets[j];

                if (bucket->Vertices.size() == 0)
                    continue;

                // Draw vertices
                m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                m_numDrawCalls++;
            }
        }

        if (m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
        {
            RendererObject &laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];

            for (int k = 0; k < laraSkinJoints.ObjectMeshes.size(); k++)
            {
                RendererMesh *mesh = laraSkinJoints.ObjectMeshes[k];

                for (int j = 0; j < 2; j++)
                {
                    RendererBucket *bucket = &mesh->Buckets[j];

                    if (bucket->Vertices.size() == 0)
                        continue;

                    // Draw vertices
                    m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    m_numDrawCalls++;
                }
            }
        }

        for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
        {
            RendererMesh *mesh = laraSkin.ObjectMeshes[k];

            for (int j = 0; j < NUM_BUCKETS; j++)
            {
                RendererBucket *bucket = &mesh->Buckets[j];

                if (bucket->Vertices.size() == 0)
                    continue;

                // Draw vertices
                m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                m_numDrawCalls++;
            }
        }

        // Draw items

        RendererObject &hairsObj = *m_moveableObjects[ID_LARA_HAIR];

        // First matrix is Lara's head matrix, then all 6 hairs matrices. Bones are adjusted at load time for accounting this.
        m_stItem.World = Matrix::Identity;
        Matrix matrices[7];
        matrices[0] = laraObj.AnimationTransforms[LM_HEAD] * m_LaraWorldMatrix;
        for (int i = 0; i < hairsObj.BindPoseTransforms.size(); i++)
        {
            HAIR_STRUCT *hairs = &Hairs[0][i];
            Matrix world = Matrix::CreateFromYawPitchRoll(TO_RAD(hairs->pos.yRot), TO_RAD(hairs->pos.xRot), 0) * Matrix::CreateTranslation(hairs->pos.xPos, hairs->pos.yPos, hairs->pos.zPos);
            matrices[i + 1] = world;
        }
        memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 7);
        m_cbItem.updateData(m_stItem, m_context.Get());
        m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
        m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

        for (int k = 0; k < hairsObj.ObjectMeshes.size(); k++)
        {
            RendererMesh *mesh = hairsObj.ObjectMeshes[k];

            for (int j = 0; j < 4; j++)
            {
                RendererBucket *bucket = &mesh->Buckets[j];

                if (bucket->Vertices.size() == 0)
                    continue;

                // Draw vertices
                m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                m_numDrawCalls++;
            }
        }

    }

    void Renderer11::renderTitleImage()
{
        wchar_t introFileChars[255];

        std::mbstowcs(introFileChars, g_GameFlow->Intro, 255);
        std::wstring titleStringFileName(introFileChars);
        Texture2D texture = Texture2D(m_device.Get(), titleStringFileName);

        float currentFade = 0;
        while (currentFade <= 1.0f)
        {
            drawFullScreenImage(texture.ShaderResourceView.Get(), currentFade, m_backBufferRTV, m_depthStencilView);
            SyncRenderer();
            currentFade += FADE_FACTOR;
            m_swapChain->Present(0, 0);
        }

        for (int i = 0; i < 30 * 1.5f; i++)
        {
            drawFullScreenImage(texture.ShaderResourceView.Get(), 1.0f, m_backBufferRTV, m_depthStencilView);
            SyncRenderer();
            m_swapChain->Present(0, 0);
        }

        currentFade = 1.0f;
        while (currentFade >= 0.0f)
        {
            drawFullScreenImage(texture.ShaderResourceView.Get(), currentFade, m_backBufferRTV, m_depthStencilView);
            SyncRenderer();
            currentFade -= FADE_FACTOR;
            m_swapChain->Present(0, 0);
        }
    }

    void Renderer11::drawGunShells(RenderView& view)
{
        RendererRoom &const room = m_rooms[LaraItem->roomNumber];
        RendererItem *item = &m_items[Lara.itemNumber];

        m_stItem.AmbientLight = room.AmbientLight;
        memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

        m_stLights.NumLights = item->Lights.size();
        for (int j = 0; j < item->Lights.size(); j++)
            memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
        m_cbLights.updateData(m_stLights, m_context.Get());
        m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());

        m_stMisc.AlphaTest = true;
        m_cbMisc.updateData(m_stMisc, m_context.Get());
        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

        for (int i = 0; i < 24; i++)
        {
            GUNSHELL_STRUCT *gunshell = &Gunshells[i];

            if (gunshell->counter > 0)
            {
                OBJECT_INFO *obj = &Objects[gunshell->objectNumber];
                RendererObject &moveableObj = *m_moveableObjects[gunshell->objectNumber];

                Matrix translation = Matrix::CreateTranslation(gunshell->pos.xPos, gunshell->pos.yPos, gunshell->pos.zPos);
                Matrix rotation = Matrix::CreateFromYawPitchRoll(TO_RAD(gunshell->pos.yRot), TO_RAD(gunshell->pos.xRot), TO_RAD(gunshell->pos.zRot));
                Matrix world = rotation * translation;

                m_stItem.World = world;
                m_cbItem.updateData(m_stItem, m_context.Get());
                m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());

                RendererMesh *mesh = moveableObj.ObjectMeshes[0];

                for (int b = 0; b < NUM_BUCKETS; b++)
                {
                    RendererBucket *bucket = &mesh->Buckets[b];

                    if (bucket->Vertices.size() == 0)
                        continue;

                    m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    m_numDrawCalls++;
                }
            }
        }

    }

    void Renderer11::renderTitleMenu()
    {
        char stringBuffer[255];
        int title_menu_to_display = getTitleMenu();
        __int64 title_selected_option = getTitleSelection();
        int y = 400;
        short lastY;
        RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];
        RendererDisplayMode* mode = &adapter->DisplayModes[CurrentSettings.videoMode];

        switch (title_menu_to_display)
        {
        case title_main_menu:
            a:
            if (title_selected_option & 1)
                drawString(400, y, g_GameFlow->GetString(STRING_NEW_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_NEW_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (title_selected_option & 2)
                drawString(400, y, g_GameFlow->GetString(STRING_LOAD_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_LOAD_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (title_selected_option & 4)
                drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (title_selected_option & 8)
                drawString(400, y, g_GameFlow->GetString(STRING_EXIT_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_EXIT_GAME), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);
            break;

        case title_select_level:
            lastY = 50;

            drawString(400, 26, g_GameFlow->GetString(STRING_SELECT_LEVEL), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            for (int i = 1; i < g_GameFlow->GetNumLevels(); i++)
            {
                int i2 = i - 1;
                InventoryRing* ring = g_Inventory.GetRing(INV_TYPE_TITLE);
                GameScriptLevel* levelScript = g_GameFlow->GetLevel(i);

                drawString(400, lastY, g_GameFlow->GetString(levelScript->NameStringIndex), D3DCOLOR_ARGB(255, 255, 255, 255),
                    PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_selected_option & (1 << i2) ? PRINTSTRING_BLINK : 0));

                lastY += 24;
            }
            break;

        case title_load_game:
            if (g_GameFlow->EnableLoadSave)
            {
                y = 44;
                LoadSavegameInfos();

                for (int n = 1; n < MAX_SAVEGAMES + 1; n++)
                {
                    int n2 = n - 1;

                    if (!g_NewSavegameInfos[n - 1].Present)
                        drawString(400, y, g_GameFlow->GetString(45), D3DCOLOR_ARGB(255, 255, 255, 255),
                            PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (title_selected_option & (1 << (n2 + 1)) ? PRINTSTRING_BLINK : 0));
                    else
                    {
                        sprintf(stringBuffer, "%05d", g_NewSavegameInfos[n-1].Count);
                        drawString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE | (title_selected_option & (1 << (n2 + 1)) ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

                        drawString(250, y, (char*)g_NewSavegameInfos[n-1].LevelName.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE | (title_selected_option & (1 << (n2 + 1)) ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

                        sprintf(stringBuffer, g_GameFlow->GetString(44), g_NewSavegameInfos[n-1].Days, g_NewSavegameInfos[n-1].Hours, g_NewSavegameInfos[n-1].Minutes, g_NewSavegameInfos[n-1].Seconds);
                        drawString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
                            PRINTSTRING_OUTLINE | (title_selected_option & (1 << (n2 + 1)) ? PRINTSTRING_BLINK : 0));
                    }

                    y += 24;
                }
            }
            else
                goto a;
            break;
        case title_options_menu:
            
            y = 350;

            if (title_selected_option & 1)
                drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (title_selected_option & 2)
                drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (title_selected_option & 4)
                drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            break;

        case title_display_menu:
            y = 200;

            drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            // Screen resolution
            drawString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & 1) ? PRINTSTRING_BLINK : 0));
            
            ZeroMemory(stringBuffer, 255);
            sprintf(stringBuffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

            drawString(400, y, stringBuffer, PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 0)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Windowed mode
            drawString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 1)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.Windowed ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 1)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable dynamic shadows
            drawString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 2)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableShadows ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 2)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable caustics
            drawString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 3)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableCaustics ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 3)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable volumetric fog
            drawString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 4)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableVolumetricFog ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 4)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Apply
            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 5)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            //cancel
            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 6)) ? PRINTSTRING_BLINK : 0));
            break;

        case title_controls_menu:
            y = 40;

            drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            for (int k = 0; k < 18; k++)
            {
                drawString(200, y, g_GameFlow->GetString(STRING_CONTROLS_MOVE_FORWARD + k),
                    PRINTSTRING_COLOR_WHITE,
                    PRINTSTRING_OUTLINE | ((title_selected_option & (1 << k)) ? PRINTSTRING_BLINK : 0) |
                    (CurrentSettings.waitingForkey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

                if (CurrentSettings.waitingForkey && (title_selected_option & (1 << k)))
                {
                    drawString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
                        PRINTSTRING_COLOR_YELLOW,
                        PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
                }
                else
                {
                    drawString(400, y, (char*)g_KeyNames[KeyboardLayout[1][k]],
                        PRINTSTRING_COLOR_ORANGE,
                        PRINTSTRING_OUTLINE);
                }

                y += 25;
            }

            // Apply and cancel
            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 18)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 19)) ? PRINTSTRING_BLINK : 0));
            break;

        case title_sounds_menu:
            y = 200;

            drawString(400, y, g_GameFlow->GetString(STRING_SOUND),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            // Enable sound
            drawString(200, y, g_GameFlow->GetString(STRING_ENABLE_SOUND),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 0)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableSound ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 0)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable sound special effects
            drawString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 1)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableAudioSpecialEffects ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 1)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Music volume
            drawString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 2)) ? PRINTSTRING_BLINK : 0));
            drawBar(CurrentSettings.conf.MusicVolume / 100.0f, g_MusicVolumeBar);

            y += 25;

            // Sound FX volume
            drawString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 3)) ? PRINTSTRING_BLINK : 0));
            drawBar(CurrentSettings.conf.SfxVolume / 100.0f, g_SFXVolumeBar);
            y += 25;

            // Apply and cancel
            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 4)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((title_selected_option & (1 << 5)) ? PRINTSTRING_BLINK : 0));
            break;
        }
    }

    void Renderer11::renderPauseMenu()
    {
        char stringBuffer[255];
        int pause_menu_to_display_ = GetPauseMenu();
        __int64 pause_selected_option_ = GetPauseSelection();
        int y;
        RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];
        RendererDisplayMode* mode = &adapter->DisplayModes[CurrentSettings.videoMode];

        switch (pause_menu_to_display_)
        {
        case pause_main_menu:
            y = 275;

            if (pause_selected_option_ & 1)
                drawString(400, y, "Statistics", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, "Statistics", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (pause_selected_option_ & 2)
                drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, "Options", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (pause_selected_option_ & 4)
                drawString(400, y, g_GameFlow->GetString(STRING_EXIT_TO_TITLE), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_EXIT_TO_TITLE), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            break;

        case pause_statistics:
            y = 200;
            drawString(400, y, "Statistics", PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER);
            y += 25;
            drawString(400, y, "todo, stats", PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            break;

        case pause_options_menu:
            y = 275;

            if (pause_selected_option_ & 1)
                drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (pause_selected_option_ & 2)
                drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            y += 25;

            if (pause_selected_option_ & 4)
                drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER | PRINTSTRING_BLINK);
            else
                drawString(400, y, g_GameFlow->GetString(STRING_SOUND), PRINTSTRING_COLOR_WHITE, PRINTSTRING_CENTER);

            break;

        case pause_display_menu:
            y = 200;

            drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            // Screen resolution
            drawString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & 1) ? PRINTSTRING_BLINK : 0));

            ZeroMemory(stringBuffer, 255);
            sprintf(stringBuffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

            drawString(400, y, stringBuffer, PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 0)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Windowed mode
            drawString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 1)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.Windowed ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 1)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable dynamic shadows
            drawString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 2)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableShadows ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 2)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable caustics
            drawString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 3)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableCaustics ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 3)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable volumetric fog
            drawString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 4)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableVolumetricFog ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 4)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Apply
            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 5)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            //cancel
            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 6)) ? PRINTSTRING_BLINK : 0));
            break;

        case pause_controls_menu:
            y = 40;

            drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            for (int k = 0; k < 18; k++)
            {
                drawString(200, y, g_GameFlow->GetString(STRING_CONTROLS_MOVE_FORWARD + k),
                    PRINTSTRING_COLOR_WHITE,
                    PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << k)) ? PRINTSTRING_BLINK : 0) |
                    (CurrentSettings.waitingForkey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

                if (CurrentSettings.waitingForkey && (pause_selected_option_ & (1 << k)))
                {
                    drawString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
                        PRINTSTRING_COLOR_YELLOW,
                        PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
                }
                else
                {
                    drawString(400, y, (char*)g_KeyNames[KeyboardLayout[1][k]],
                        PRINTSTRING_COLOR_ORANGE,
                        PRINTSTRING_OUTLINE);
                }

                y += 25;
            }

            break;

        case pause_sounds_menu:
            y = 200;

            drawString(400, y, g_GameFlow->GetString(STRING_SOUND),
                PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

            y += 25;

            // Enable sound
            drawString(200, y, g_GameFlow->GetString(STRING_ENABLE_SOUND),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 0)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableSound ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 0)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Enable sound special effects
            drawString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 1)) ? PRINTSTRING_BLINK : 0));
            drawString(400, y, g_GameFlow->GetString(CurrentSettings.conf.EnableAudioSpecialEffects ? STRING_ENABLED : STRING_DISABLED),
                PRINTSTRING_COLOR_WHITE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 1)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            // Music volume
            drawString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 2)) ? PRINTSTRING_BLINK : 0));
            drawBar(CurrentSettings.conf.MusicVolume / 100.0f, g_MusicVolumeBar);

            y += 25;

            // Sound FX volume
            drawString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 3)) ? PRINTSTRING_BLINK : 0));
            drawBar(CurrentSettings.conf.SfxVolume / 100.0f, g_SFXVolumeBar);
            y += 25;

            // Apply and cancel
            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 4)) ? PRINTSTRING_BLINK : 0));

            y += 25;

            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                PRINTSTRING_COLOR_ORANGE,
                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | ((pause_selected_option_ & (1 << 5)) ? PRINTSTRING_BLINK : 0));
            break;
        }
        drawAllStrings();
    }

    void Renderer11::renderNewInventory()
    {
        draw_current_object_list(RING_INVENTORY);
       
        handle_inventry_menu();

        if (rings[RING_AMMO]->ringactive)
            draw_current_object_list(RING_AMMO);

        draw_ammo_selector();
        fade_ammo_selector();
        drawAllStrings();
    }

    void Renderer11::renderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, ID3D11ShaderResourceView* background)
    {
        char stringBuffer[255];

        bool drawLogo = true;

        RECT guiRect;
        Vector4 guiColor = Vector4(0.0f, 0.0f, 0.25f, 0.5f);
        bool drawGuiRect = false;

        RECT rect;
        rect.left = 0;
        rect.top = 0;
        rect.right = ScreenWidth;
        rect.bottom = ScreenHeight;

        m_lines2DToDraw.clear();
        m_strings.clear();

        m_nextLine2D = 0;

        // Set basic render states
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

        // Bind and clear render target
        //m_context->ClearRenderTargetView(target, Colors::Black);
        //m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_context->OMSetRenderTargets(1, &target, depthTarget);
        m_context->RSSetViewports(1, &m_viewport);

        if (background != nullptr)
        {
            drawFullScreenImage(background, 0.5f, target, depthTarget);
        }

        m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        // Set vertex buffer
        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Set shaders
        m_context->VSSetShader(m_vsInventory.Get(), NULL, 0);
        m_context->PSSetShader(m_psInventory.Get(), NULL, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());

        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        InventoryRing *activeRing = g_Inventory.GetRing(g_Inventory.GetActiveRing());
        int lastRing = 0;

        float cameraX = INV_CAMERA_DISTANCE * cos(g_Inventory.GetCameraTilt() * RADIAN);
        float cameraY = g_Inventory.GetCameraY() - INV_CAMERA_DISTANCE * sin(g_Inventory.GetCameraTilt() * RADIAN);
        float cameraZ = 0.0f;
        CCameraMatrixBuffer inventoryCam;
        inventoryCam.ViewProjection = Matrix::CreateLookAt(Vector3(cameraX, cameraY, cameraZ),
                                                           Vector3(0.0f, g_Inventory.GetCameraY() - 512.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)) *
                                      Matrix::CreatePerspectiveFieldOfView(80.0f * RADIAN,
                                                                           g_Renderer.ScreenWidth / (float)g_Renderer.ScreenHeight, 1.0f, 200000.0f);

        m_cbCameraMatrices.updateData(inventoryCam, m_context.Get());
        m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());

#ifdef NEW_INV
        if (CurrentLevel == 0)//sorry
        {
            int menu = getTitleMenu();
            if (menu == title_main_menu || menu == title_options_menu)
                drawLogo = 1;
            else
                drawLogo = 0;

            if (drawLogo)
            {
                float factorX = (float)ScreenWidth / REFERENCE_RES_WIDTH;
                float factorY = (float)ScreenHeight / REFERENCE_RES_HEIGHT;

                RECT rect;
                rect.left = 250 * factorX;
                rect.right = 550 * factorX;
                rect.top = 50 * factorY;
                rect.bottom = 200 * factorY;

                m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->Additive());
                m_spriteBatch->Draw(m_logo.ShaderResourceView.Get(), rect, Vector4::One);
                m_spriteBatch->End();
            }

            renderTitleMenu();
            return;
        }
        
        if (GLOBAL_invMode == IM_INGAME)
        {
          renderNewInventory();
          return;
        }

        if (GLOBAL_invMode == IM_PAUSE)//sorry again
        {
            int menu = GetPauseMenu();
            bool drawThing;
            if (menu == pause_main_menu || menu == pause_options_menu)
                drawThing = 1;
            else
                drawThing = 0;

            if (drawThing)
            {
                guiRect.left = 345;
                guiRect.right = 115;
                guiRect.top = 265;
                guiRect.bottom = 100;
                drawColoredQuad(guiRect.left, guiRect.top, guiRect.right, guiRect.bottom, guiColor);
            }

            renderPauseMenu();
            return;
        }
#endif

        for (int k = 0; k < NUM_INVENTORY_RINGS; k++)
        {
            InventoryRing *ring = g_Inventory.GetRing(k);
            if (ring->draw == false || ring->numObjects == 0)
                continue;

            short numObjects = ring->numObjects;
            float deltaAngle = 360.0f / numObjects;
            int objectIndex = 0;
            objectIndex = ring->currentObject;

            // Yellow title
            if (ring->focusState == INV_FOCUS_STATE_NONE && g_Inventory.GetType() != INV_TYPE_TITLE)
                drawString(400, 20, g_GameFlow->GetString(activeRing->titleStringIndex), PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER);

            for (int i = 0; i < numObjects; i++)
            {
                short inventoryObject = ring->objects[objectIndex].inventoryObject;
                short objectNumber = g_Inventory.GetInventoryObject(ring->objects[objectIndex].inventoryObject)->objectNumber;

                //if (ring->focusState != INV_FOCUS_STATE_NONE && (k != g_Inventory->GetActiveRing() || inventoryObject != ring->objects[i].inventoryObject))
                //	continue;

                // Calculate the inventory object position and rotation
                float currentAngle = 0.0f;
                short steps = -objectIndex + ring->currentObject;
                if (steps < 0)
                    steps += numObjects;
                currentAngle = steps * deltaAngle;
                currentAngle += ring->rotation;

                if (ring->focusState == INV_FOCUS_STATE_NONE && k == g_Inventory.GetActiveRing())	// Not focused on item AND is active ring.
                {
                    if (objectIndex == ring->currentObject)
                    {
                        ring->objects[objectIndex].rotation += 45 * 360 / 30;
                    }
                    else if (ring->objects[objectIndex].rotation != 0)
                    {
                        if (ring->objects[objectIndex].rotation - INV_NUM_FRAMES_POPUP < 0)
                        {
                            ring->objects[objectIndex].rotation = 0;
                        }
                        else if (ring->objects[objectIndex].rotation + INV_NUM_FRAMES_POPUP > 65536)
                        {
                            ring->objects[objectIndex].rotation = 0;
                        }
                        else
                        {
                            if (ring->objects[objectIndex].rotation < 65536 / 2)
                            {
                                ring->objects[objectIndex].rotation -= ring->objects[objectIndex].rotation / INV_NUM_FRAMES_POPUP;
                            }
                            else
                            {
                                ring->objects[objectIndex].rotation += (65536 - ring->objects[objectIndex].rotation) / INV_NUM_FRAMES_POPUP;
                            }
                        }
                    }
                }
                else if (ring->focusState != INV_FOCUS_STATE_POPUP && ring->focusState != INV_FOCUS_STATE_POPOVER)
                    g_Inventory.GetRing(k)->objects[objectIndex].rotation = 0;

                if (ring->objects[objectIndex].rotation > 65536)
                    ring->objects[objectIndex].rotation = 0;

                int x = ring->distance * cos(currentAngle * RADIAN);
                int y = ring->y;
                int z = ring->distance * sin(currentAngle * RADIAN);

                int localAxis = 65536 / numObjects * (objectIndex - ring->currentObject) - (65536 / 360 * ring->rotation);

                // Prepare the object transform
                Matrix scale = Matrix::CreateScale(ring->objects[objectIndex].scale, ring->objects[objectIndex].scale, ring->objects[objectIndex].scale);
                Matrix translation = Matrix::CreateTranslation(x, y, z);
                Matrix rotation = Matrix::CreateRotationY(TO_RAD(ring->objects[objectIndex].rotation + 16384 + g_Inventory.GetInventoryObject(inventoryObject)->rotY + localAxis));
                Matrix transform = (scale * rotation) * translation;

                OBJECT_INFO *obj = &Objects[objectNumber];
                if (!m_moveableObjects[objectNumber].has_value())
                    continue;
                RendererObject &moveableObj = *m_moveableObjects[objectNumber];

                // Build the object animation matrices
                if (ring->focusState == INV_FOCUS_STATE_FOCUSED && obj->animIndex != -1 &&
                    objectIndex == ring->currentObject && k == g_Inventory.GetActiveRing())
                {
                    ANIM_FRAME *framePtr[2];
                    int rate = 0;
                    getFrame(obj->animIndex, ring->framePtr, framePtr, &rate);
                    updateAnimation(NULL, moveableObj, framePtr, 0, 1, 0xFFFFFFFF);
                }
                else
                {
                    if (obj->animIndex != -1)
                    {
                        ANIM_FRAME *framePtr = &g_Level.Frames[g_Level.Anims[obj->animIndex].framePtr];
                        updateAnimation(NULL, moveableObj, &framePtr, 0, 1, 0xFFFFFFFF);
                    }
                }

                for (int n = 0; n < moveableObj.ObjectMeshes.size(); n++)
                {
                    RendererMesh *mesh = moveableObj.ObjectMeshes[n];

                    // HACK: revolver and crossbow + lasersight
                    if (moveableObj.Id == ID_REVOLVER_ITEM && !Lara.Weapons[WEAPON_REVOLVER].HasLasersight && n > 0)
                        break;

                    if (moveableObj.Id == ID_CROSSBOW_ITEM && !Lara.Weapons[WEAPON_CROSSBOW].HasLasersight && n > 0)
                        break;

                    // Finish the world matrix
                    if (obj->animIndex != -1)
                        m_stItem.World = (moveableObj.AnimationTransforms[n] * transform);
                    else
                        m_stItem.World = (moveableObj.BindPoseTransforms[n] * transform);
                    m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
                    m_cbItem.updateData(m_stItem, m_context.Get());
                    m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
                    m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

                    for (int m = 0; m < NUM_BUCKETS; m++)
                    {
                        RendererBucket *bucket = &mesh->Buckets[m];
                        if (bucket->Vertices.size() == 0)
                            continue;

                        if (m < 2)
                            m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
                        else
                            m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

                        m_stMisc.AlphaTest = (m < 2);
                        m_cbMisc.updateData(m_stMisc, m_context.Get());
                        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

                        m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    }
                }

                short inventoryItem = ring->objects[objectIndex].inventoryObject;

                // Draw special stuff if needed
                if (objectIndex == ring->currentObject && k == g_Inventory.GetActiveRing())
                {
                    if (g_Inventory.GetActiveRing() == INV_RING_OPTIONS)
                    {
                        /* **************** PASSAPORT ************* */
                        if (inventoryItem == _INV_OBJECT_PASSPORT && ring->focusState == INV_FOCUS_STATE_FOCUSED)
                        {
                            /* **************** LOAD AND SAVE MENU ************* */
                            if (ring->passportAction == INV_WHAT_PASSPORT_LOAD_GAME || ring->passportAction == INV_WHAT_PASSPORT_SAVE_GAME)
                            {
                                y = 44;

                                for (int n = 0; n < MAX_SAVEGAMES; n++)
                                {
                                    if (!g_NewSavegameInfos[n].Present)
                                        drawString(400, y, g_GameFlow->GetString(45), D3DCOLOR_ARGB(255, 255, 255, 255),
                                                    PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
                                    else
                                    {
                                        sprintf(stringBuffer, "%05d", g_NewSavegameInfos[n].Count);
                                        drawString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

                                        drawString(250, y, (char *)g_NewSavegameInfos[n].LevelName.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

                                        sprintf(stringBuffer, g_GameFlow->GetString(44), g_NewSavegameInfos[n].Days, g_NewSavegameInfos[n].Hours, g_NewSavegameInfos[n].Minutes, g_NewSavegameInfos[n].Seconds);
                                        drawString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
                                                    PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
                                    }

                                    y += 24;
                                }

                                drawLogo = false;

                                drawGuiRect = true;
                                guiRect.left = 180;
                                guiRect.right = 440;
                                guiRect.top = 24;
                                guiRect.bottom = y + 20 - 24;

                                //drawColoredQuad(180, 24, 440, y + 20 - 24, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
                            }
                            /* **************** SELECT LEVEL ************* */
                            else if (ring->passportAction == INV_WHAT_PASSPORT_SELECT_LEVEL)
                            {
                                drawLogo = false;

                                drawGuiRect = true;
                                guiRect.left = 200;
                                guiRect.right = 400;
                                guiRect.top = 24;
                                guiRect.bottom = 24 * (g_GameFlow->GetNumLevels() - 1) + 40;

                                //drawColoredQuad(200, 24, 400, 24 * (g_GameFlow->GetNumLevels() - 1) + 40, Vector4(0.0f, 0.0f, 0.25f, 0.5f));

                                short lastY = 50;

                                for (int n = 1; n < g_GameFlow->GetNumLevels(); n++)
                                {
                                    GameScriptLevel *levelScript = g_GameFlow->GetLevel(n);
                                    drawString(400, lastY, g_GameFlow->GetString(levelScript->NameStringIndex), D3DCOLOR_ARGB(255, 255, 255, 255),
                                                PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n - 1 ? PRINTSTRING_BLINK : 0));

                                    lastY += 24;
                                }
                            }
                            char *string = (char *)"";
                            switch (ring->passportAction)
                            {
                            case INV_WHAT_PASSPORT_NEW_GAME:
                                string = g_GameFlow->GetString(STRING_NEW_GAME);
                                break;
                            case INV_WHAT_PASSPORT_SELECT_LEVEL:
                                string = g_GameFlow->GetString(STRING_SELECT_LEVEL);
                                break;
                            case INV_WHAT_PASSPORT_LOAD_GAME:
                                string = g_GameFlow->GetString(STRING_LOAD_GAME);
                                break;
                            case INV_WHAT_PASSPORT_SAVE_GAME:
                                string = g_GameFlow->GetString(STRING_SAVE_GAME);
                                break;
                            case INV_WHAT_PASSPORT_EXIT_GAME:
                                string = g_GameFlow->GetString(STRING_EXIT_GAME);
                                break;
                            case INV_WHAT_PASSPORT_EXIT_TO_TITLE:
                                string = g_GameFlow->GetString(STRING_EXIT_TO_TITLE);
                                break;
                            }

                            drawString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
                        }
                        /* **************** GRAPHICS SETTINGS ************* */
                        else if (inventoryItem == _INV_OBJECT_SUNGLASSES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
                        {
                            // Draw settings menu
                            RendererVideoAdapter *adapter = &m_adapters[g_Configuration.Adapter];

                            int y = 200;

                            drawString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
                                        PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

                            y += 25;

                            // Screen resolution
                            drawString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

                            RendererDisplayMode *mode = &adapter->DisplayModes[ring->SelectedVideoMode];
                            char buffer[255];
                            ZeroMemory(buffer, 255);
                            sprintf(buffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

                            drawString(400, y, buffer, PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Windowed mode
                            drawString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.Windowed ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Enable dynamic shadows
                            drawString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.EnableShadows ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Enable caustics
                            drawString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.EnableCaustics ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Enable volumetric fog
                            drawString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.EnableVolumetricFog ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Apply and cancel
                            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 6 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawLogo = false;

                            drawGuiRect = true;
                            guiRect.left = 180;
                            guiRect.right = 440;
                            guiRect.top = 180;
                            guiRect.bottom = y + 20 - 180;

                            //drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
                        }
                        /* **************** AUDIO SETTINGS ************* */
                        else if (inventoryItem == _INV_OBJECT_HEADPHONES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
                        {
                            // Draw sound menu

                            y = 200;

                            drawString(400, y, g_GameFlow->GetString(STRING_SOUND),
                                        PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

                            y += 25;

                            // Enable sound
                            drawString(200, y, g_GameFlow->GetString(STRING_ENABLE_SOUND),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.EnableSound ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Enable sound special effects
                            drawString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
                            drawString(400, y, g_GameFlow->GetString(ring->Configuration.EnableAudioSpecialEffects ? STRING_ENABLED : STRING_DISABLED),
                                        PRINTSTRING_COLOR_WHITE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            // Music volume
                            drawString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
                            //DrawBar(400, y + 4, 150, 18, ring->Configuration.MusicVolume, 0x0000FF, 0x0000FF);
                            drawBar(ring->Configuration.MusicVolume / 100.0f, g_MusicVolumeBar);

                            y += 25;

                            // Sound FX volume
                            drawString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
                            //DrawBar(400, y + 4, 150, 18, ring->Configuration.SfxVolume, 0x0000FF, 0x0000FF);
                            drawBar(ring->Configuration.SfxVolume / 100.0f, g_SFXVolumeBar);
                            y += 25;

                            // Apply and cancel
                            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawLogo = false;

                            drawGuiRect = true;
                            guiRect.left = 180;
                            guiRect.right = 440;
                            guiRect.top = 180;
                            guiRect.bottom = y + 20 - 180;

                            //drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
                        }
                        /* **************** CONTROLS SETTINGS ************* */
                        else if (inventoryItem == _INV_OBJECT_KEYS && ring->focusState == INV_FOCUS_STATE_FOCUSED)
                        {
                            // Draw sound menu
                            y = 40;

                            drawString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
                                        PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

                            y += 25;

                            for (int k = 0; k < 18; k++)
                            {
                                drawString(200, y, g_GameFlow->GetString(STRING_CONTROLS_MOVE_FORWARD + k),
                                            PRINTSTRING_COLOR_WHITE,
                                            PRINTSTRING_OUTLINE | (ring->selectedIndex == k ? PRINTSTRING_BLINK : 0) |
                                                (ring->waitingForKey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

                                if (ring->waitingForKey && k == ring->selectedIndex)
                                {
                                    drawString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
                                                PRINTSTRING_COLOR_YELLOW,
                                                PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
                                }
                                else
                                {
                                    drawString(400, y, (char *)g_KeyNames[KeyboardLayout[1][k]],
                                                PRINTSTRING_COLOR_ORANGE,
                                                PRINTSTRING_OUTLINE);
                                }

                                y += 25;
                            }

                            // Apply and cancel
                            drawString(400, y, g_GameFlow->GetString(STRING_APPLY),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 0 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawString(400, y, g_GameFlow->GetString(STRING_CANCEL),
                                        PRINTSTRING_COLOR_ORANGE,
                                        PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 1 ? PRINTSTRING_BLINK : 0));

                            y += 25;

                            drawLogo = false;

                            drawGuiRect = true;
                            guiRect.left = 180;
                            guiRect.right = 440;
                            guiRect.top = 20;
                            guiRect.bottom = y + 20 - 20;

                            //drawColoredQuad(180, 20, 440, y + 20 - 20, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
                        }
                        else
                        {
                            // Draw the description below the object
                            char *string = g_GameFlow->GetString(g_Inventory.GetInventoryObject(inventoryItem)->objectName); // (char*)g_NewStrings[g_Inventory->GetInventoryObject(inventoryItem)->objectName].c_str(); // &AllStrings[AllStringsOffsets[g_Inventory->GetInventoryObject(inventoryItem)->objectName]];
                            drawString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
                        }
                    }
                    else
                    {
                        short inventoryItem = g_Inventory.GetRing(k)->objects[objectIndex].inventoryObject;
                        char *string = g_GameFlow->GetString(g_Inventory.GetInventoryObject(inventoryItem)->objectName); // &AllStrings[AllStringsOffsets[InventoryObjectsList[inventoryItem].objectName]];

                        if (/*g_Inventory->IsCurrentObjectWeapon() &&*/ ring->focusState == INV_FOCUS_STATE_FOCUSED)
                        {
                            y = 100;

                            for (int a = 0; a < ring->numActions; a++)
                            {
                                int stringIndex = 0;
                                if (ring->actions[a] == INV_ACTION_USE)
                                    stringIndex = STRING_USE;
                                if (ring->actions[a] == INV_ACTION_COMBINE)
                                    stringIndex = STRING_COMBINE;
                                if (ring->actions[a] == INV_ACTION_SEPARE)
                                    stringIndex = STRING_SEPARE;
                                if (ring->actions[a] == INV_ACTION_SELECT_AMMO)
                                    stringIndex = STRING_CHOOSE_AMMO;

                                // Apply and cancel
                                drawString(400, y, g_GameFlow->GetString(stringIndex),
                                            PRINTSTRING_COLOR_WHITE,
                                            PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == a ? PRINTSTRING_BLINK : 0));

                                y += 25;
                            }

                            drawLogo = false;

                            drawGuiRect = true;
                            guiRect.left = 300;
                            guiRect.right = 200;
                            guiRect.top = 80;
                            guiRect.bottom = y + 20 - 80;

                            //drawColoredQuad(300, 80, 200, y + 20 - 80, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
                        }

                        int quantity = -1;
                        switch (objectNumber)
                        {
                        case ID_BIGMEDI_ITEM:
                            quantity = Lara.NumLargeMedipacks;
                            break;
                        case ID_SMALLMEDI_ITEM:
                            quantity = Lara.NumSmallMedipacks;
                            break;
                        case ID_FLARE_INV_ITEM:
                            quantity = Lara.NumFlares;
                            break;
                        case ID_SHOTGUN_AMMO1_ITEM:
                            quantity = Lara.Weapons[WEAPON_SHOTGUN].Ammo[0];
                            if (quantity != -1)
                                quantity /= 6;
                            break;
                        case ID_SHOTGUN_AMMO2_ITEM:
                            quantity = Lara.Weapons[WEAPON_SHOTGUN].Ammo[1];
                            if (quantity != -1)
                                quantity /= 6;
                            break;
                        case ID_HK_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_HK].Ammo[0];
                            break;
                        case ID_CROSSBOW_AMMO1_ITEM:
                            quantity = Lara.Weapons[WEAPON_CROSSBOW].Ammo[0];
                            break;
                        case ID_CROSSBOW_AMMO2_ITEM:
                            quantity = Lara.Weapons[WEAPON_CROSSBOW].Ammo[1];
                            break;
                        case ID_CROSSBOW_AMMO3_ITEM:
                            quantity = Lara.Weapons[WEAPON_CROSSBOW].Ammo[2];
                            break;
                        case ID_REVOLVER_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_REVOLVER].Ammo[0];
                            break;
                        case ID_UZI_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_UZI].Ammo[0];
                            break;
                        case ID_PISTOLS_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_PISTOLS].Ammo[0];
                            break;
                        case ID_GRENADE_AMMO1_ITEM:
                            quantity = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0];
                            break;
                        case ID_GRENADE_AMMO2_ITEM:
                            quantity = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1];
                            break;
                        case ID_GRENADE_AMMO3_ITEM:
                            quantity = Lara.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2];
                            break;
                        case ID_HARPOON_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[0];
                            break;
                        case ID_ROCKET_LAUNCHER_AMMO_ITEM:
                            quantity = Lara.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0];
                            break;
                        case ID_PICKUP_ITEM4:
                            quantity = Savegame.Level.Secrets;
                            break;
                        default:
                            if (objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8)
                                quantity = Lara.Puzzles[objectNumber - ID_PUZZLE_ITEM1];

                            else if (objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2)
                                quantity = Lara.PuzzlesCombo[objectNumber - ID_PUZZLE_ITEM1_COMBO1];

                            else if (objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8)
                                quantity = Lara.Keys[objectNumber - ID_KEY_ITEM1];

                            else if (objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2)
                                quantity = Lara.KeysCombo[objectNumber - ID_KEY_ITEM1_COMBO1];

                            else if (objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM3)
                                quantity = Lara.Pickups[objectNumber - ID_PICKUP_ITEM1];

                            else if (objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM3_COMBO2)
                                quantity = Lara.PickupsCombo[objectNumber - ID_PICKUP_ITEM1_COMBO1];

                            else if (objectNumber >= ID_EXAMINE1 && objectNumber <= ID_EXAMINE3)
                                quantity = Lara.Pickups[objectNumber - ID_EXAMINE1];

                            else if (objectNumber >= ID_EXAMINE1_COMBO1 && objectNumber <= ID_EXAMINE3_COMBO2)
                                quantity = Lara.PickupsCombo[objectNumber - ID_EXAMINE1_COMBO1];
                        }

                        if (quantity < 1)
                            drawString(400, 550, string, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
                        else
                        {
                            sprintf(stringBuffer, "%d x %s", quantity, string);
                            drawString(400, 550, stringBuffer, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
                        }
                    }
                }

                objectIndex++;
                if (objectIndex == numObjects)
                    objectIndex = 0;
            }

            lastRing++;
        }

        if (drawGuiRect)
        {
            // Draw blu box
            drawColoredQuad(guiRect.left, guiRect.top, guiRect.right, guiRect.bottom, guiColor);
        }

        drawAllStrings();

        if (g_Inventory.GetType() == INV_TYPE_TITLE && g_GameFlow->TitleType == TITLE_FLYBY && drawLogo)
        {
            // Draw main logo
            float factorX = (float)ScreenWidth / REFERENCE_RES_WIDTH;
            float factorY = (float)ScreenHeight / REFERENCE_RES_HEIGHT;

            RECT rect;
            rect.left = 250 * factorX;
            rect.right = 550 * factorX;
            rect.top = 50 * factorY;
            rect.bottom = 200 * factorY;

            m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->Additive());
            m_spriteBatch->Draw(m_logo.ShaderResourceView.Get(), rect, Vector4::One);
            m_spriteBatch->End();
        }

    }

    void Renderer11::drawFullScreenQuad(ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector3 color, bool cinematicBars)
    {
        RendererVertex vertices[4];

        if (!cinematicBars)
        {
            vertices[0].Position.x = -1.0f;
            vertices[0].Position.y = 1.0f;
            vertices[0].Position.z = 0.0f;
            vertices[0].UV.x = 0.0f;
            vertices[0].UV.y = 0.0f;
            vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[1].Position.x = 1.0f;
            vertices[1].Position.y = 1.0f;
            vertices[1].Position.z = 0.0f;
            vertices[1].UV.x = 1.0f;
            vertices[1].UV.y = 0.0f;
            vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[2].Position.x = 1.0f;
            vertices[2].Position.y = -1.0f;
            vertices[2].Position.z = 0.0f;
            vertices[2].UV.x = 1.0f;
            vertices[2].UV.y = 1.0f;
            vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[3].Position.x = -1.0f;
            vertices[3].Position.y = -1.0f;
            vertices[3].Position.z = 0.0f;
            vertices[3].UV.x = 0.0f;
            vertices[3].UV.y = 1.0f;
            vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
        }
        else
        {
            float cinematicFactor = 0.12f;

            vertices[0].Position.x = -1.0f;
            vertices[0].Position.y = 1.0f - cinematicFactor * 2;
            vertices[0].Position.z = 0.0f;
            vertices[0].UV.x = 0.0f;
            vertices[0].UV.y = cinematicFactor;
            vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[1].Position.x = 1.0f;
            vertices[1].Position.y = 1.0f - cinematicFactor * 2;
            vertices[1].Position.z = 0.0f;
            vertices[1].UV.x = 1.0f;
            vertices[1].UV.y = cinematicFactor;
            vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[2].Position.x = 1.0f;
            vertices[2].Position.y = -(1.0f - cinematicFactor * 2);
            vertices[2].Position.z = 0.0f;
            vertices[2].UV.x = 1.0f;
            vertices[2].UV.y = 1.0f - cinematicFactor;
            vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

            vertices[3].Position.x = -1.0f;
            vertices[3].Position.y = -(1.0f - cinematicFactor * 2);
            vertices[3].Position.z = 0.0f;
            vertices[3].UV.x = 0.0f;
            vertices[3].UV.y = 1.0f - cinematicFactor;
            vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
        }

        m_context->VSSetShader(m_vsFullScreenQuad.Get(), NULL, 0);
        m_context->PSSetShader(m_psFullScreenQuad.Get(), NULL, 0);

        m_context->PSSetShaderResources(0, 1, &texture);
        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());

        m_primitiveBatch->Begin();
        m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
        m_primitiveBatch->End();

    }

    void Renderer11::drawRopes(RenderView& view)
{
        for (int n = 0; n < NumRopes; n++)
        {
            ROPE_STRUCT *rope = &Ropes[n];

            if (rope->active)
            {
                // Original algorithm:
                // 1) Transform segment coordinates from 3D to 2D + depth
                // 2) Get dx, dy and the segment length
                // 3) Get sine and cosine from dx / length and dy / length
                // 4) Calculate a scale factor
                // 5) Get the coordinates of the 4 corners of each sprite iteratively
                // 6) Last step only for us, unproject back to 3D coordinates

                // Tranform rope points
                Vector3 projected[24];
                Matrix world = Matrix::Identity;

                for (int i = 0; i < 24; i++)
                {
                    Vector3 absolutePosition = Vector3(rope->position.x + rope->segment[i].x / 65536.0f,
                                                       rope->position.y + rope->segment[i].y / 65536.0f,
                                                       rope->position.z + rope->segment[i].z / 65536.0f);

                    projected[i] = m_viewportToolkit.Project(absolutePosition, Projection, View, world);
                }

                // Now each rope point is transformed in screen X, Y and Z depth
                // Let's calculate dx, dy corrections and scaling
                float dx = projected[1].x - projected[0].x;
                float dy = projected[1].y - projected[0].y;
                float length = sqrt(dx * dx + dy * dy);
                float s = 0;
                float c = 0;

                if (length != 0)
                {
                    s = -dy / length;
                    c = dx / length;
                }

                float w = 6.0f;
                if (projected[0].z)
                {
                    w = 6.0f * PhdPerspective / projected[0].z / 65536.0f;
                    if (w < 3)
                        w = 3;
                }

                float sdx = s * w;
                float sdy = c * w;

                float x1 = projected[0].x - sdx;
                float y1 = projected[0].y - sdy;

                float x2 = projected[0].x + sdx;
                float y2 = projected[0].y + sdy;

                float depth = projected[0].z;

                for (int j = 0; j < 24; j++)
                {
                    Vector3 p1 = m_viewportToolkit.Unproject(Vector3(x1, y1, depth), Projection, View, world);
                    Vector3 p2 = m_viewportToolkit.Unproject(Vector3(x2, y2, depth), Projection, View, world);

                    dx = projected[j].x - projected[j - 1].x;
                    dy = projected[j].y - projected[j - 1].y;
                    length = sqrt(dx * dx + dy * dy);
                    s = 0;
                    c = 0;

                    if (length != 0)
                    {
                        s = -dy / length;
                        c = dx / length;
                    }

                    w = 6.0f;
                    if (projected[j].z)
                    {
                        w = 6.0f * PhdPerspective / projected[j].z / 65536.0f;
                        if (w < 3)
                            w = 3;
                    }

                    float sdx = s * w;
                    float sdy = c * w;

                    float x3 = projected[j].x - sdx;
                    float y3 = projected[j].y - sdy;

                    float x4 = projected[j].x + sdx;
                    float y4 = projected[j].y + sdy;

                    depth = projected[j].z;

                    Vector3 p3 = m_viewportToolkit.Unproject(Vector3(x3, y3, depth), Projection, View, world);
                    Vector3 p4 = m_viewportToolkit.Unproject(Vector3(x4, y4, depth), Projection, View, world);

                    addSprite3D(&m_sprites[20],
                                Vector3(p1.x, p1.y, p1.z),
                                Vector3(p2.x, p2.y, p2.z),
                                Vector3(p3.x, p3.y, p3.z),
                                Vector3(p4.x, p4.y, p4.z),
                                Vector4(0.5f, 0.5f, 0.5f, 1.0f), 0, 1, { 0, 0 }, BLENDMODE_OPAQUE,view);

                    x1 = x4;
                    y1 = y4;
                    x2 = x3;
                    y2 = y3;
                }
            }
        }

    }

    void Renderer11::drawLines2D(RenderView& view)
{
        m_context->RSSetState(m_states->CullNone());
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

        m_context->VSSetShader(m_vsSolid.Get(), NULL, 0);
        m_context->PSSetShader(m_psSolid.Get(), NULL, 0);
        Matrix world = Matrix::CreateOrthographicOffCenter(0, ScreenWidth, ScreenHeight, 0, m_viewport.MinDepth, m_viewport.MaxDepth);

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());

        m_primitiveBatch->Begin();

        for (int i = 0; i < m_lines2DToDraw.size(); i++)
        {
            RendererLine2D *line = m_lines2DToDraw[i];

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

        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

    }

    void Renderer11::drawSpiders(RenderView& view)
{
        /*XMMATRIX world;
        UINT cPasses = 1;

        if (Objects[ID_SPIDERS_EMITTER].loaded)
        {
            OBJECT_INFO* obj = &Objects[ID_SPIDERS_EMITTER];
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

            effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
            effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPE_MOVEABLE);

            if (bucketIndex == RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKET_SOLID_DS)
                effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_OPAQUE);
            else
                effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_ALPHATEST);

            for (int i = 0; i < NUM_SPIDERS; i++)
            {
                SPIDER_STRUCT* spider = &Spiders[i];

                if (spider->on)
                {
                    XMMATRIXTranslation(&m_tempTranslation, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);
                    XMMATRIXRotationYawPitchRoll(&m_tempRotation, spider->pos.yRot, spider->pos.xRot, spider->pos.zRot);
                    XMMATRIXMultiply(&m_tempWorld, &m_tempRotation, &m_tempTranslation);
                    effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_tempWorld);

                    effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[spider->roomNumber]->AmbientLight);

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

    void Renderer11::drawRats(RenderView& view)
{
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        if (Objects[ID_RATS_EMITTER].loaded)
        {
            OBJECT_INFO *obj = &Objects[ID_RATS_EMITTER];
            RendererObject &moveableObj = *m_moveableObjects[ID_RATS_EMITTER];

            for (int m = 0; m < 32; m++)
                memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

            for (int i = 0; i < NUM_RATS; i++)
            {
                RAT_STRUCT *rat = &Rats[i];

                if (rat->on)
                {
                    RendererMesh *mesh = getMesh(Objects[ID_RATS_EMITTER].meshIndex + (rand() % 8));
                    Matrix translation = Matrix::CreateTranslation(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
                    Matrix rotation = Matrix::CreateFromYawPitchRoll(rat->pos.yRot, rat->pos.xRot, rat->pos.zRot);
                    Matrix world = rotation * translation;

                    m_stItem.World = world;
                    m_stItem.Position = Vector4(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, 1.0f);
                    m_stItem.AmbientLight = m_rooms[rat->roomNumber].AmbientLight;
                    m_cbItem.updateData(m_stItem, m_context.Get());

                    for (int b = 0; b < 2; b++)
                    {
                        RendererBucket *bucket = &mesh->Buckets[b];

                        if (bucket->Vertices.size() == 0)
                            continue;

                        m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                        m_numDrawCalls++;
                    }
                }
            }
        }

    }

    void Renderer11::drawBats(RenderView& view)
{
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        if (Objects[ID_BATS_EMITTER].loaded)
        {
            OBJECT_INFO *obj = &Objects[ID_BATS_EMITTER];
            RendererObject &moveableObj = *m_moveableObjects[ID_BATS_EMITTER];
            RendererMesh *mesh = getMesh(Objects[ID_BATS_EMITTER].meshIndex + (-GlobalCounter & 3));

            for (int m = 0; m < 32; m++)
                memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

            for (int b = 0; b < 2; b++)
            {
                RendererBucket *bucket = &mesh->Buckets[b];

                if (bucket->Vertices.size() == 0)
                    continue;

                for (int i = 0; i < NUM_BATS; i++)
                {
                    BAT_STRUCT *bat = &Bats[i];

                    if (bat->on)
                    {
                        Matrix translation = Matrix::CreateTranslation(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos);
                        Matrix rotation = Matrix::CreateFromYawPitchRoll(bat->pos.yRot, bat->pos.xRot, bat->pos.zRot);
                        Matrix world = rotation * translation;

                        m_stItem.World = world;
                        m_stItem.Position = Vector4(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 1.0f);
                        m_stItem.AmbientLight = m_rooms[bat->roomNumber].AmbientLight;
                        m_cbItem.updateData(m_stItem, m_context.Get());

                        m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                        m_numDrawCalls++;
                    }
                }
            }
        }

    }

	void Renderer11::drawLittleBeetles(RenderView& view)
{
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;

		m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		if (Objects[ID_LITTLE_BEETLE].loaded)
		{
			OBJECT_INFO* obj = &Objects[ID_LITTLE_BEETLE];
			RendererObject& moveableObj = *m_moveableObjects[ID_LITTLE_BEETLE];

			for (int m = 0; m < 32; m++)
				memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

			for (int i = 0; i < NUM_LITTLE_BETTLES; i++)
			{
				BEETLE_INFO* beetle = &LittleBeetles[i];

				if (beetle->on)
				{
					RendererMesh* mesh = getMesh(Objects[ID_LITTLE_BEETLE].meshIndex);
					Matrix translation = Matrix::CreateTranslation(beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(beetle->pos.yRot, beetle->pos.xRot, beetle->pos.zRot);
					Matrix world = rotation * translation;

					m_stItem.World = world;
					m_stItem.Position = Vector4(beetle->pos.xPos, beetle->pos.yPos, beetle->pos.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[beetle->roomNumber].AmbientLight;
                    m_cbItem.updateData(m_stItem,m_context.Get());

					for (int b = 0; b < 2; b++)
					{
						RendererBucket* bucket = &mesh->Buckets[b];

						if (bucket->Vertices.size() == 0)
							continue;

						m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
						m_numDrawCalls++;
					}
				}
			}
		}
	}


    void Renderer11::drawLines3D(RenderView& view)
{
        m_context->RSSetState(m_states->CullNone());
        m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

        m_context->VSSetShader(m_vsSolid.Get(), NULL, 0);
        m_context->PSSetShader(m_psSolid.Get(), NULL, 0);

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());

        m_primitiveBatch->Begin();

        for (int i = 0; i < m_lines3DToDraw.size(); i++)
        {
            RendererLine3D *line = m_lines3DToDraw[i];

            RendererVertex v1;
            v1.Position = line->start;
            v1.Color = line->color;

            RendererVertex v2;
            v2.Position = line->end;
            v2.Color = line->color;
            m_primitiveBatch->DrawLine(v1, v2);
        }

        m_primitiveBatch->End();

        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

    }

    void Renderer11::addLine3D(Vector3 start, Vector3 end, Vector4 color)
    {
        if (m_nextLine3D >= MAX_LINES_3D)
            return;

        RendererLine3D *line = &m_lines3DBuffer[m_nextLine3D++];

        line->start = start;
        line->end = end;
        line->color = color;

        m_lines3DToDraw.push_back(line);
    }

    void Renderer11::renderLoadingScreen(std::wstring& fileName)
    {
        return;
        /*

        Texture2D texture = Texture2D(m_device, fileName);

        m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
        m_fadeFactor = 0.0f;

        while (true) {
            if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor < 1.0f)
                m_fadeFactor += FADE_FACTOR;

            if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor > 0.0f)
                m_fadeFactor -= FADE_FACTOR;

            // Set basic render states
            m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
            m_context->RSSetState(m_states->CullCounterClockwise());
            m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

            // Clear screen
            m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
            m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            // Bind the back buffer
            m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
            m_context->RSSetViewports(1, &m_viewport);

            // Draw the full screen background
            drawFullScreenQuad(texture.ShaderResourceView.GetAddressOf(), Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
            m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            m_swapChain->Present(0, 0);
            m_context->ClearState();
            if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor >= 1.0f) {
                m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;
                m_fadeFactor = 1.0f;
            }

            if (m_fadeStatus == RENDERER_FADE_STATUS::NO_FADE && m_progress == 100) {
                m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
                m_fadeFactor = 1.0f;
            }

            if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.0f) {
                break;
            }
        }
        */
    }

    void Renderer11::addDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
    {
        if (m_nextLight >= MAX_LIGHTS)
            return;

        RendererLight *dynamicLight = &m_lights[m_nextLight++];

        dynamicLight->Position = Vector3(float(x), float(y), float(z));
        dynamicLight->Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f);
        dynamicLight->Out = falloff * 256.0f;
        dynamicLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
        dynamicLight->Dynamic = true;
        dynamicLight->Intensity = falloff / 2;

        m_dynamicLights.push_back(dynamicLight);
        //NumDynamics++;
    }

    void Renderer11::clearDynamicLights()
{
        m_dynamicLights.clear();
    }

    void Renderer11::drawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget)
    {
        // Reset GPU state
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
        m_context->OMSetRenderTargets(1, &target, depthTarget);
        m_context->RSSetViewports(1, &m_viewport);
        drawFullScreenQuad(texture, Vector3(fade, fade, fade), false);
    }

    void Renderer11::renderInventory()
{
        m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
        m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
        renderInventoryScene(m_backBufferRTV, m_depthStencilView, m_dumpScreenRenderTarget.ShaderResourceView.Get());
        m_swapChain->Present(0, 0);
    }

    void Renderer11::renderTitle()
{
        m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);
        m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);

        renderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
        m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_STENCIL | D3D11_CLEAR_DEPTH, 1.0f, 0);

        renderInventoryScene(m_backBufferRTV, m_depthStencilView, nullptr);
#if _DEBUG
        drawString(0, 0, commit.c_str(), D3DCOLOR_ARGB(255, 255,255, 255), 0);
        drawAllStrings();
#endif
        m_swapChain->Present(0, 0);
    }

    void Renderer11::renderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
    {
        using ns = std::chrono::nanoseconds;
        using get_time = std::chrono::steady_clock;
        m_timeUpdate = 0;
        m_timeDraw = 0;
        m_timeFrame = 0;
        m_numDrawCalls = 0;
        m_nextLight = 0;
        m_nextSprite = 0;
        m_nextLine3D = 0;
        m_nextLine2D = 0;

        m_currentCausticsFrame++;
        m_currentCausticsFrame %= 32;

        m_strings.clear();

        GameScriptLevel *level = g_GameFlow->GetLevel(CurrentLevel);

        m_stLights.CameraPosition = view.camera.WorldPosition;

        // Prepare the scene to draw
        auto time1 = std::chrono::high_resolution_clock::now();
        //prepareCameraForFrame();
        clearSceneItems();
        collectRooms(view);
        updateLaraAnimations(false);
        updateItemsAnimations(view);
        updateEffects(view);
        if (g_Configuration.EnableShadows)
            renderShadowMap(view);
        m_items[Lara.itemNumber].Item = LaraItem;
        collectLightsForItem(LaraItem->roomNumber, &m_items[Lara.itemNumber], view);

        // Update animated textures every 2 frames
        if (GnFrameCounter % 2 == 0)
            updateAnimatedTextures();

        auto time2 = std::chrono::high_resolution_clock::now();
        m_timeUpdate = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
        time1 = time2;

        // Draw shadow map

        // Reset GPU state
        m_context->OMSetBlendState(m_states->NonPremultiplied(), NULL, 0xFFFFFFFF);
        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

        // Bind and clear render target

        m_context->ClearRenderTargetView(target, Colors::Black);
        m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_context->OMSetRenderTargets(1, &target, depthTarget);

        m_context->RSSetViewports(1, &view.viewport);

        // Opaque geometry
        m_context->OMSetBlendState(m_states->NonPremultiplied(), NULL, 0xFFFFFFFF);
        CCameraMatrixBuffer cameraConstantBuffer;
        view.fillConstantBuffer(cameraConstantBuffer);
        cameraConstantBuffer.Frame = GnFrameCounter;
        cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;
        m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
        m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());
        drawHorizonAndSky(depthTarget);
        drawRooms(false, false, view);
        drawRooms(false, true, view);
        drawStatics(false, view);
        drawLara(view,false, false);
        drawItems(false, false, view);
        drawItems(false, true, view);
        drawEffects(view,false);
        drawGunFlashes(view);
        drawGunShells(view);
        drawBaddieGunflashes(view);
        drawDebris(view,false);
        drawBats(view);
        drawRats(view);
        drawSpiders(view);
		drawLittleBeetles(view);

        // Transparent geometry
        m_context->OMSetBlendState(m_states->NonPremultiplied(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

        drawRooms(true, false, view);
        drawRooms(true, true, view);
        drawStatics(true, view);
        drawLara(view,true, false);
        drawItems(true, false, view);
        drawItems(true, true, view);
        drawEffects(view,true);
        drawDebris(view,true);

        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

        // Do special effects and weather
        drawFires(view);
        drawSmokes(view);
        drawSmokeParticles(view);
        drawSimpleParticles(view);
        drawSparkParticles(view);
        drawExplosionParticles(view);
        drawFootprints(view);
        drawDripParticles(view);
        drawBlood(view);
        drawSparks(view);
        drawBubbles(view);
        drawDrips(view);
        drawRipples(view);
        drawUnderwaterDust(view);
        drawSplahes(view);
        drawShockwaves(view);
        drawEnergyArcs(view);

        drawRopes(view);
        drawSprites(view);
        drawLines3D(view);

        time2 = std::chrono::high_resolution_clock::now();
        m_timeFrame = (std::chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
        time1 = time2;

        // Bars
        int flash = FlashIt();
        if (DashTimer < 120)
            drawBar(DashTimer / 120.0f, g_DashBar);
        UpdateHealthBar(flash);
        UpdateAirBar(flash);
        DrawAllPickups();

        drawLines2D(view);

        if (CurrentLevel != 0)
        {
            // Draw binoculars or lasersight
            drawOverlays(view);

            m_currentY = 60;
#ifndef _DEBUG
            ROOM_INFO *r = &g_Level.Rooms[LaraItem->roomNumber];

            printDebugMessage("Update time: %d", m_timeUpdate);
            printDebugMessage("Frame time: %d", m_timeFrame);
            printDebugMessage("Draw calls: %d", m_numDrawCalls);
            printDebugMessage("Lara.roomNumber: %d", LaraItem->roomNumber);
			printDebugMessage("Lara.location: %d %d", LaraItem->location.roomNumber, LaraItem->location.yNumber);
			printDebugMessage("LaraItem.boxNumber: %d",/* canJump: %d, canLongJump: %d, canMonkey: %d,*/ LaraItem->boxNumber);
            printDebugMessage("Lara.pos: %d %d %d", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
            printDebugMessage("Lara.rot: %d %d %d", LaraItem->pos.xRot, LaraItem->pos.yRot, LaraItem->pos.zRot);
            printDebugMessage("Lara.animNumber: %d", LaraItem->animNumber);
            printDebugMessage("Lara.frameNumber: %d", LaraItem->frameNumber);
            printDebugMessage("Lara.currentAnimState: %d", LaraItem->currentAnimState);
            printDebugMessage("Lara.requiredAnimState: %d", LaraItem->requiredAnimState);
            printDebugMessage("Lara.goalAnimState: %d", LaraItem->goalAnimState);
            printDebugMessage("Lara.weaponItem: %d", Lara.weaponItem);
            printDebugMessage("Lara.gunType: %d", Lara.gunType);
            printDebugMessage("Lara.gunStatus: %d", Lara.gunStatus);
            printDebugMessage("Lara.speed, fallspeed: %d %d", LaraItem->speed, LaraItem->fallspeed);
            printDebugMessage("Lara.climbStatus: %d", Lara.climbStatus);
            printDebugMessage("Room: %d %d %d %d", r->x, r->z, r->x + r->xSize * WALL_SIZE, r->z + r->ySize * WALL_SIZE);
            printDebugMessage("Room.y, minFloor, maxCeiling: %d %d %d ", r->y, r->minfloor, r->maxceiling);
            printDebugMessage("Camera.pos: %d %d %d", Camera.pos.x, Camera.pos.y, Camera.pos.z);
            printDebugMessage("Camera.target: %d %d %d", Camera.target.x, Camera.target.y, Camera.target.z);
			printDebugMessage("target hitPoints: %d", Lara.target ? Lara.target->hitPoints : NULL);
#endif
        }

        drawAllStrings();

    }

    void Renderer11::renderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view)
    {
        GameScriptLevel *level = g_GameFlow->GetLevel(CurrentLevel);

        collectRooms(view);
        // Draw shadow map

        // Reset GPU state
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        m_context->RSSetState(m_states->CullCounterClockwise());
        m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

        // Bind and clear render target

        m_context->ClearRenderTargetView(target, Colors::Black);
        m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_context->OMSetRenderTargets(1, &target, depthTarget);

        m_context->RSSetViewports(1, &view.viewport);

        // Opaque geometry
        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
        CCameraMatrixBuffer cameraConstantBuffer;
        view.fillConstantBuffer(cameraConstantBuffer);
        cameraConstantBuffer.Frame = GnFrameCounter;
        cameraConstantBuffer.CameraUnderwater = g_Level.Rooms[cameraConstantBuffer.RoomNumber].flags & ENV_FLAG_WATER;
        m_cbCameraMatrices.updateData(cameraConstantBuffer, m_context.Get());
        m_context->VSSetConstantBuffers(0, 1, m_cbCameraMatrices.get());
        drawHorizonAndSky(depthTarget);
        drawRooms(false, false, view);
    }

    void Renderer11::DumpGameScene()
{
        renderScene(m_dumpScreenRenderTarget.RenderTargetView.Get(), m_dumpScreenRenderTarget.DepthStencilView.Get(), gameCamera);
    }

    void Renderer11::drawItems(bool transparent, bool animated, RenderView& view)
    {
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

		int firstBucket = (transparent ? 1 : 0);
		int lastBucket = (transparent ? 2 : 1);

        m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        RendererItem *item = &m_items[Lara.itemNumber];

        // Set shaders
        m_context->VSSetShader(m_vsItems.Get(), NULL, 0);
        m_context->PSSetShader(m_psItems.Get(), NULL, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(1, 1, m_reflectionCubemap.ShaderResourceView.GetAddressOf());
        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        m_stMisc.AlphaTest = !transparent;
        m_cbMisc.updateData(m_stMisc, m_context.Get());
        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

        for (int i = 0; i < view.itemsToDraw.size(); i++)
        {
            RendererItem *item = view.itemsToDraw[i];
            RendererRoom &const room = m_rooms[item->Item->roomNumber];
            RendererObject &moveableObj = *m_moveableObjects[item->Item->objectNumber];

            short objectNumber = item->Item->objectNumber;
            if (moveableObj.DoNotDraw)
            {
                continue;
            }
            else if (objectNumber == ID_TEETH_SPIKES || objectNumber == ID_RAISING_BLOCK1 || objectNumber == ID_RAISING_BLOCK2 
                || objectNumber == ID_JOBY_SPIKES)
            {
                // Raising blocks and teeth spikes are normal animating objects but scaled on Y direction
                drawScaledSpikes(view,item, transparent, animated);
            }
            else if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
            {
                // We'll draw waterfalls later
                continue;
            }
			else if (objectNumber >= ID_WRAITH1 && objectNumber <= ID_WRAITH3)
			{
				// Wraiths have some additional special effects
				drawAnimatingItem(view,item, transparent, animated);
				drawWraithExtra(view,item, transparent, animated);
			}
            else
            {
                drawAnimatingItem(view,item, transparent, animated);
            }
        }

    }

    void Renderer11::drawAnimatingItem(RenderView& view,RendererItem* item, bool transparent, bool animated)
    {
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

        int firstBucket = (transparent ? 2 : 0);
        int lastBucket = (transparent ? 4 : 2);
        if (m_rooms.size() <= item->Item->roomNumber)
        {
            return;
        }
        RendererRoom &const room = m_rooms[item->Item->roomNumber];
        RendererObject &moveableObj = *m_moveableObjects[item->Item->objectNumber];
        OBJECT_INFO *obj = &Objects[item->Item->objectNumber];

        m_stItem.World = item->World;
        m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
        m_stItem.AmbientLight = room.AmbientLight;
        memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
        m_cbItem.updateData(m_stItem, m_context.Get());
        m_context->VSSetConstantBuffers(1, 1, m_cbItem.get());
        m_context->PSSetConstantBuffers(1, 1, m_cbItem.get());

        m_stLights.NumLights = item->Lights.size();
        for (int j = 0; j < item->Lights.size(); j++)
            memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
        m_cbLights.updateData(m_stLights, m_context.Get());
        m_context->PSSetConstantBuffers(2, 1, m_cbLights.get());

        m_stMisc.AlphaTest = !transparent;
        m_cbMisc.updateData(m_stMisc, m_context.Get());
        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

        for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
        {
            if (!(item->Item->meshBits & (1 << k)))
                continue;

            RendererMesh *mesh;
            if (obj->meshSwapSlot != -1 && ((item->Item->swapMeshFlags >> k) & 1))
            {
                RendererObject &swapMeshObj = *m_moveableObjects[obj->meshSwapSlot];
                mesh = swapMeshObj.ObjectMeshes[k];
            }
            else
            {
                mesh = moveableObj.ObjectMeshes[k];
            }

            for (int j = firstBucket; j < lastBucket; j++)
            {
                RendererBucket *bucket = &mesh->Buckets[j];

                if (bucket->Vertices.size() == 0)
                    continue;

                // Draw vertices
                m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                m_numDrawCalls++;
            }
        }

    }

    void Renderer11::drawScaledSpikes(RenderView& view,RendererItem* item, bool transparent, bool animated)
    {
        short objectNumber = item->Item->objectNumber;
        if ((item->Item->objectNumber != ID_TEETH_SPIKES || item->Item->itemFlags[1]) && (item->Item->objectNumber != ID_RAISING_BLOCK1 || item->Item->triggerFlags > -1))
        {
            item->Scale = Matrix::CreateScale(1.0f, item->Item->itemFlags[1] / 4096.0f, 1.0f);
            item->World = item->Scale * item->Rotation * item->Translation;

            return drawAnimatingItem(view,item, transparent, animated);
        }
    }

	void Renderer11::drawWraithExtra(RenderView& view,RendererItem* item, bool transparent, bool animated)
	{
		ITEM_INFO* nativeItem = item->Item;
		WRAITH_INFO* info = (WRAITH_INFO*)nativeItem->data;
		
		if (transparent || animated)
			return ;

		for (int j = 0; j <= 4; j++)
		{
			Matrix rotation;

			switch (j)
			{
			case 0:
				rotation = Matrix::CreateRotationY(TO_RAD(-1092));
				break;
			case 1:
				rotation = Matrix::CreateRotationY(TO_RAD(1092));
				break;
			case 2:
				rotation = Matrix::CreateRotationZ(TO_RAD(-1092));
				break;
			case 3:
				rotation = Matrix::CreateRotationZ(TO_RAD(1092));
				break;
			default:
				rotation = Matrix::Identity;
				break;
			}

			Matrix world = rotation * item->World;

			/*for (int i = 0; i < 7; i++)
			{
				Vector3 p1 = Vector3(info[i].xPos - nativeItem->pos.xPos, info[i].yPos - nativeItem->pos.yPos, info[i].zPos - nativeItem->pos.zPos);
				Vector3 p2 = Vector3(info[i+1].xPos - info[i  ].xPos, info[i + 1].yPos - info[i  ].yPos, info[i + 1].zPos - info[i ].zPos);

				p1 = Vector3::Transform(p1, world);
				p2 = Vector3::Transform(p2, world);

				AddLine3D(p1, p2, Vector4(info[i].r / 255.0f, info[i].g / 255.0f, info[i].b / 255.0f, 1.0f));
			}*/

			for (int i = 0; i < 7; i++)
			{
				Vector3 p1 = Vector3(info[i].xPos, info[i].yPos, info[i].zPos);
				Vector3 p2 = Vector3(info[i + 1].xPos , info[i + 1].yPos, info[i + 1].zPos);

				addLine3D(p1, p2, Vector4(info[i].r / 255.0f, info[i].g / 255.0f, info[i].b / 255.0f, 1.0f));
			}
		}

	}

    void Renderer11::drawStatics(bool transparent, RenderView& view)
    {
        //return true;
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

		int firstBucket = (transparent ? 1 : 0);
		int lastBucket = (transparent ? 2 : 1);

        m_context->IASetVertexBuffers(0, 1, m_staticsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->IASetIndexBuffer(m_staticsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Set shaders
        m_context->VSSetShader(m_vsStatics.Get(), NULL, 0);
        m_context->PSSetShader(m_psStatics.Get(), NULL, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_staticsTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, (std::get<1>(m_staticsTextures[0])).ShaderResourceView.GetAddressOf());
        ID3D11SamplerState *sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        for (int i = 0; i < view.staticsToDraw.size(); i++)
        {
            MESH_INFO *msh = view.staticsToDraw[i]->Mesh;

            /*if (!(msh->flags & 1))
                continue;*/

            RendererRoom &const room = m_rooms[view.staticsToDraw[i]->RoomIndex];

            RendererObject &staticObj = *m_staticObjects[msh->staticNumber];

            if (staticObj.ObjectMeshes.size() > 0)
            {
                RendererMesh *mesh = staticObj.ObjectMeshes[0];

                m_stStatic.World = (Matrix::CreateRotationY(TO_RAD(msh->yRot)) * Matrix::CreateTranslation(msh->x, msh->y, msh->z));
                m_stStatic.Position = Vector4(msh->x,msh->y,msh->z,1);
                m_stStatic.Color = msh->color;
                m_cbStatic.updateData(m_stStatic, m_context.Get());
                m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());

                for (int j = firstBucket; j < lastBucket; j++)
                {
                    RendererBucket *bucket = &mesh->Buckets[j];

                    if (bucket->Vertices.size() == 0)
                        continue;

                    // Draw vertices
                    m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    m_numDrawCalls++;
                }
            }
        }
    }

    void Renderer11::drawRooms(bool transparent, bool animated, RenderView& view)
    {
        UINT stride = sizeof(RendererVertex);
        UINT offset = 0;

		int firstBucket = (transparent ? 1 : 0);
		int lastBucket = (transparent ? 2 : 1);

        if (!animated)
        {
            // Set vertex buffer
            m_context->IASetVertexBuffers(0, 1, m_roomsVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
            m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_context->IASetInputLayout(m_inputLayout.Get());
            m_context->IASetIndexBuffer(m_roomsIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        }

        // Set shaders
        m_context->VSSetShader(m_vsRooms.Get(), NULL, 0);
        m_context->PSSetShader(m_psRooms.Get(), NULL, 0);

        // Set texture
        m_context->PSSetShaderResources(0, 1, (std::get<0>(m_roomTextures[0])).ShaderResourceView.GetAddressOf());
        m_context->PSSetShaderResources(3, 1, (std::get<1>(m_roomTextures[0])).ShaderResourceView.GetAddressOf());
        ID3D11SamplerState *sampler = m_states->AnisotropicWrap();
        m_context->PSSetSamplers(0, 1, &sampler);
        m_context->PSSetShaderResources(1, 1, m_caustics[m_currentCausticsFrame / 2].ShaderResourceView.GetAddressOf());
        m_context->PSSetSamplers(1, 1, m_shadowSampler.GetAddressOf());
        m_context->PSSetShaderResources(2, 1, m_shadowMap.ShaderResourceView.GetAddressOf());

        // Set shadow map data
        if (m_shadowLight != NULL)
        {

            memcpy(&m_stShadowMap.Light, m_shadowLight, sizeof(ShaderLight));
            m_stShadowMap.CastShadows = true;
            //m_stShadowMap.ViewProjectionInverse = ViewProjection.Invert();
        }
        else
        {
            m_stShadowMap.CastShadows = false;
        }
        m_cbShadowMap.updateData(m_stShadowMap, m_context.Get());
        m_context->VSSetConstantBuffers(4, 1, m_cbShadowMap.get());
        m_context->PSSetConstantBuffers(4, 1, m_cbShadowMap.get());

        if (animated)
            m_primitiveBatch->Begin();
        for (int i = 0; i < view.roomsToDraw.size(); i++)
        {
            //Draw transparent back-to-front
            int index = i;
            if (transparent) {
                index = view.roomsToDraw.size() - 1 - index;
            }
            RendererRoom *room = view.roomsToDraw[index];

            m_stLights.NumLights = view.lightsToDraw.size();
            for (int j = 0; j < view.lightsToDraw.size(); j++)
                memcpy(&m_stLights.Lights[j], view.lightsToDraw[j], sizeof(ShaderLight));
            m_cbLights.updateData(m_stLights, m_context.Get());
            m_context->PSSetConstantBuffers(1, 1, m_cbLights.get());

            m_stMisc.Caustics = (room->Room->flags & ENV_FLAG_WATER);
            m_stMisc.AlphaTest = transparent;
            m_cbMisc.updateData(m_stMisc, m_context.Get());
            m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());
            m_stRoom.AmbientColor = room->AmbientLight;
            m_stRoom.water = (room->Room->flags & ENV_FLAG_WATER) != 0 ? 1 : 0;
            m_cbRoom.updateData(m_stRoom, m_context.Get());
            m_context->VSSetConstantBuffers(5, 1, m_cbRoom.get());
            m_context->PSSetConstantBuffers(5, 1, m_cbRoom.get());
            for (int j = firstBucket; j < lastBucket; j++)
            {
                RendererBucket *bucket;
                if (!animated)
                    bucket = &room->Buckets[j];
                else
                    bucket = &room->AnimatedBuckets[j];

                if (bucket->Vertices.size() == 0)
                    continue;

                if (!animated)
                {
                    m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    m_numDrawCalls++;
                }
                /*else
                {
                    for (int k = 0; k < bucket->Polygons.size(); k++)
                    {
                        RendererPolygon *poly = &bucket->Polygons[k];

                        if (poly->Shape == SHAPE_RECTANGLE)
                        {
                            m_primitiveBatch->DrawQuad(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
                                                       bucket->Vertices[poly->Indices[2]], bucket->Vertices[poly->Indices[3]]);
                        }
                        else
                        {
                            m_primitiveBatch->DrawTriangle(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
                                                           bucket->Vertices[poly->Indices[2]]);
                        }
                    }
                }*/
            }
        }

        if (animated)
            m_primitiveBatch->End();
    }

    void Renderer11::drawHorizonAndSky(ID3D11DepthStencilView* depthTarget)
    {
        // Update the sky
        GameScriptLevel *level = g_GameFlow->GetLevel(CurrentLevel);
        Vector4 color = Vector4(SkyColor1.r / 255.0f, SkyColor1.g / 255.0f, SkyColor1.b / 255.0f, 1.0f);

        if (!level->Horizon)
            return ;

        if (BinocularRange)
            AlterFOV(14560 - BinocularRange);

        // Storm
        if (level->Storm)
        {
            if (LightningCount || LightningRand)
            {
                UpdateStorm();
                if (StormTimer > -1)
                    StormTimer--;
                if (!StormTimer)
                    SoundEffect(SFX_THUNDER_RUMBLE, NULL, 0);
            }
            else if (!(rand() & 0x7F))
            {
                LightningCount = (rand() & 0x1F) + 16;
                dLightningRand = rand() + 256;
                StormTimer = (rand() & 3) + 12;
            }

            color = Vector4((SkyStormColor[0]) / 255.0f, SkyStormColor[1] / 255.0f, SkyStormColor[2] / 255.0f, 1.0f);
        }

        ID3D11SamplerState *sampler;
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

        m_context->VSSetShader(m_vsSky.Get(), NULL, 0);
        m_context->PSSetShader(m_psSky.Get(), NULL, 0);

        m_stMisc.AlphaTest = true;
        m_cbMisc.updateData(m_stMisc, m_context.Get());
        m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

        m_context->PSSetShaderResources(0, 1, m_skyTexture.ShaderResourceView.GetAddressOf());
        sampler = m_states->AnisotropicClamp();
        m_context->PSSetSamplers(0, 1, &sampler);

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->IASetInputLayout(m_inputLayout.Get());
        for (int i = 0; i < 2; i++)
        {
            Matrix translation = Matrix::CreateTranslation(Camera.pos.x + SkyPos1 - i * 9728.0f, Camera.pos.y - 1536.0f, Camera.pos.z);
            Matrix world = rotation * translation;

            m_stStatic.World = (rotation * translation);
            m_stStatic.Color = color;
            m_cbStatic.updateData(m_stStatic, m_context.Get());
            m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());
            m_context->PSSetConstantBuffers(1, 1, m_cbStatic.get());

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

            m_context->PSSetShaderResources(0, 1, (std::get<0>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
            m_context->PSSetShaderResources(2, 1, (std::get<1>(m_moveablesTextures[0])).ShaderResourceView.GetAddressOf());
            sampler = m_states->AnisotropicClamp();
            m_context->PSSetSamplers(0, 1, &sampler);

            RendererObject &moveableObj = *m_moveableObjects[ID_HORIZON];

            m_stStatic.World = Matrix::CreateTranslation(Camera.pos.x, Camera.pos.y, Camera.pos.z);
            m_stStatic.Position = Vector4::Zero;
            m_stStatic.Color = Vector4::One;
            m_cbStatic.updateData(m_stStatic, m_context.Get());
            m_context->VSSetConstantBuffers(1, 1, m_cbStatic.get());
            m_context->PSSetConstantBuffers(1, 1, m_cbStatic.get());

            m_stMisc.AlphaTest = true;
            m_cbMisc.updateData(m_stMisc, m_context.Get());
            m_context->PSSetConstantBuffers(3, 1, m_cbMisc.get());

            for (int k = 0; k < moveableObj.ObjectMeshes.size(); k++)
            {
                RendererMesh *mesh = moveableObj.ObjectMeshes[k];

                for (int j = 0; j < NUM_BUCKETS; j++)
                {
                    RendererBucket *bucket = &mesh->Buckets[j];

                    if (bucket->Vertices.size() == 0)
                        continue;

                    if (j == RENDERER_BUCKET_TRANSPARENT)
                        m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
                    else
                        m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

                    // Draw vertices
                    m_context->DrawIndexed(bucket->Indices.size(), bucket->StartIndex, 0);
                    m_numDrawCalls++;
                }
            }
        }

        // Clear just the Z-buffer so we can start drawing on top of the horizon
        m_context->ClearDepthStencilView(depthTarget, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    }

    void Renderer11::Draw()
{

        renderToCubemap(m_reflectionCubemap, Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos - 1024, LaraItem->pos.zPos), LaraItem->roomNumber);
        renderScene(m_backBufferRTV, m_depthStencilView, gameCamera);
        m_context->ClearState();
        //drawFinalPass();
        m_swapChain->Present(0, 0);
    }
} // namespace T5M::Renderer
