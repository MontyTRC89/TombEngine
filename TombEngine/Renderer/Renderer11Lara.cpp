#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Game/animation.h"
#include "Game/effects/Hair.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/control/control.h"
#include "Game/spotcam.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Scripting/Include/ScriptInterfaceLevel.h"
#include "Specific/level.h"

using namespace TEN::Effects::Hair;
using namespace TEN::Math;
using namespace TEN::Renderer;

extern ScriptInterfaceFlowHandler *g_GameFlow;

bool shouldAnimateUpperBody(const LaraWeaponType& weapon)
{
	const auto& nativeItem = *LaraItem;
	auto& player = Lara;

	switch (weapon)
	{
	case LaraWeaponType::RocketLauncher:
	case LaraWeaponType::HarpoonGun:
	case LaraWeaponType::GrenadeLauncher:
	case LaraWeaponType::Crossbow:
	case LaraWeaponType::Shotgun:
		if (nativeItem.Animation.ActiveState == LS_IDLE ||
			nativeItem.Animation.ActiveState == LS_TURN_LEFT_FAST ||
			nativeItem.Animation.ActiveState == LS_TURN_RIGHT_FAST ||
			nativeItem.Animation.ActiveState == LS_TURN_LEFT_SLOW ||
			nativeItem.Animation.ActiveState == LS_TURN_RIGHT_SLOW)
		{
			return true;
		}

		return false;

	case LaraWeaponType::HK:
	{
		// Animate upper body if Lara is shooting from shoulder OR if Lara is standing still/turning
		int baseAnim = Objects[GetWeaponObjectID(weapon)].animIndex;
		if (player.RightArm.AnimNumber - baseAnim == 0 ||
			player.RightArm.AnimNumber - baseAnim == 2 ||
			player.RightArm.AnimNumber - baseAnim == 4)
		{
			return true;
		}
		else
		{
			if (nativeItem.Animation.ActiveState == LS_IDLE ||
				nativeItem.Animation.ActiveState == LS_TURN_LEFT_FAST ||
				nativeItem.Animation.ActiveState == LS_TURN_RIGHT_FAST ||
				nativeItem.Animation.ActiveState == LS_TURN_LEFT_SLOW ||
				nativeItem.Animation.ActiveState == LS_TURN_RIGHT_SLOW)
			{
				return true;
			}

			return false;
		}
	}
	break;

	default:
		return false;
		break;
	}
}

void Renderer11::UpdateLaraAnimations(bool force)
{
	auto& rItem = m_items[LaraItem->Index];
	rItem.ItemNumber = LaraItem->Index;

	if (!force && rItem.DoneAnimations)
		return;

	auto& playerObject = *m_moveableObjects[ID_LARA];

	// Clear extra rotations.
	for (auto& bone : playerObject.LinearizedBones)
		bone->ExtraRotation = Quaternion::Identity;

	// Player world matrix.
	auto tMatrix = Matrix::CreateTranslation(LaraItem->Pose.Position.ToVector3());
	auto rotMatrix = LaraItem->Pose.Orientation.ToRotationMatrix();

	m_LaraWorldMatrix = rotMatrix * tMatrix;
	rItem.World = m_LaraWorldMatrix;

	// Update extra head and torso rotations.
	playerObject.LinearizedBones[LM_TORSO]->ExtraRotation = Lara.ExtraTorsoRot.ToQuaternion();
	playerObject.LinearizedBones[LM_HEAD]->ExtraRotation = Lara.ExtraHeadRot.ToQuaternion();

	// First calculate matrices for legs, hips, head, and torso.
	int mask = MESH_BITS(LM_HIPS) | MESH_BITS(LM_LTHIGH) | MESH_BITS(LM_LSHIN) | MESH_BITS(LM_LFOOT) | MESH_BITS(LM_RTHIGH) | MESH_BITS(LM_RSHIN) | MESH_BITS(LM_RFOOT) | MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
	
	auto frameData = GetFrameInterpData(*LaraItem);
	UpdateAnimation(&rItem, playerObject, frameData, mask);

	// Then the arms, based on current weapon status.
	if (Lara.Control.Weapon.GunType != LaraWeaponType::Flare &&
		(Lara.Control.HandStatus == HandStatus::Free || Lara.Control.HandStatus == HandStatus::Busy) ||
		Lara.Control.Weapon.GunType == LaraWeaponType::Flare && !Lara.Flare.ControlLeft)
	{
		// Both arms
		mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND) | MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
		auto frameData = GetFrameInterpData(*LaraItem);
		UpdateAnimation(&rItem, playerObject, frameData, mask);
	}
	else
	{
		// While handling weapon, extra rotation may be applied to arms.
		if (Lara.Control.Weapon.GunType == LaraWeaponType::Pistol ||
			Lara.Control.Weapon.GunType == LaraWeaponType::Uzi)
		{
			playerObject.LinearizedBones[LM_LINARM]->ExtraRotation *= Lara.LeftArm.Orientation.ToQuaternion();
			playerObject.LinearizedBones[LM_RINARM]->ExtraRotation *= Lara.RightArm.Orientation.ToQuaternion();
		}
		else
		{
			playerObject.LinearizedBones[LM_LINARM]->ExtraRotation =
			playerObject.LinearizedBones[LM_RINARM]->ExtraRotation *= Lara.RightArm.Orientation.ToQuaternion();
		}

		ArmInfo* leftArm = &Lara.LeftArm;
		ArmInfo* rightArm = &Lara.RightArm;

		// HACK: Back guns are handled differently.
		switch (Lara.Control.Weapon.GunType)
		{
		case LaraWeaponType::Shotgun:
		case LaraWeaponType::HK:
		case LaraWeaponType::Crossbow:
		case LaraWeaponType::GrenadeLauncher:
		case LaraWeaponType::RocketLauncher:
		case LaraWeaponType::HarpoonGun:
		{
			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);

			if (shouldAnimateUpperBody(Lara.Control.Weapon.GunType))
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);

			auto shotgunFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber],
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber],
				0.0f
			};

			UpdateAnimation(&rItem, playerObject, shotgunFrameData, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			if (shouldAnimateUpperBody(Lara.Control.Weapon.GunType))
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);

			shotgunFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber],
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber],
				0.0f
			};

			UpdateAnimation(&rItem, playerObject, shotgunFrameData, mask);
		}

		break;

		case LaraWeaponType::Revolver:
		{
			// Left arm
			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			auto revolverFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - GetAnimData(Lara.LeftArm.AnimNumber).frameBase],
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - GetAnimData(Lara.LeftArm.AnimNumber).frameBase],
				0.0f
			};

			UpdateAnimation(&rItem, playerObject, revolverFrameData, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			revolverFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - GetAnimData(Lara.RightArm.AnimNumber).frameBase],
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - GetAnimData(Lara.RightArm.AnimNumber).frameBase],
				0.0f
			};

			UpdateAnimation(&rItem, playerObject, revolverFrameData, mask);
		}

		break;

		case LaraWeaponType::Pistol:
		case LaraWeaponType::Uzi:
		default:
		{
			// Left arm
			int upperArmMask = MESH_BITS(LM_LINARM);
			mask = MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);
			auto pistolFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - GetAnimData(Lara.LeftArm.AnimNumber).frameBase],
				&g_Level.Frames[Lara.LeftArm.FrameBase + Lara.LeftArm.FrameNumber - GetAnimData(Lara.LeftArm.AnimNumber).frameBase],
				0.0f
			};

			UpdateAnimation(&rItem, playerObject, pistolFrameData, upperArmMask, true);
			UpdateAnimation(&rItem, playerObject, pistolFrameData, mask);

			// Right arm
			upperArmMask = MESH_BITS(LM_RINARM);
			mask = MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			pistolFrameData = AnimFrameInterpData
			{
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - GetAnimData(Lara.RightArm.AnimNumber).frameBase],
				&g_Level.Frames[Lara.RightArm.FrameBase + Lara.RightArm.FrameNumber - GetAnimData(Lara.RightArm.AnimNumber).frameBase],
				0.0f
			};
			
			UpdateAnimation(&rItem, playerObject, pistolFrameData, upperArmMask, true);
			UpdateAnimation(&rItem, playerObject, pistolFrameData, mask);
		}

		break;

		case LaraWeaponType::Flare:
		case LaraWeaponType::Torch:
			// Left arm
			ItemInfo tempItem;
			tempItem.Animation.AnimNumber = Lara.LeftArm.AnimNumber;
			tempItem.Animation.FrameNumber = Lara.LeftArm.FrameNumber;

			mask = MESH_BITS(LM_LINARM) | MESH_BITS(LM_LOUTARM) | MESH_BITS(LM_LHAND);

			// HACK: Mask head and torso when taking out a flare.
			if (!Lara.Control.IsLow &&
				tempItem.Animation.AnimNumber > (Objects[ID_FLARE_ANIM].animIndex + 1) &&
				tempItem.Animation.AnimNumber < (Objects[ID_FLARE_ANIM].animIndex + 4))
			{
				mask |= MESH_BITS(LM_TORSO) | MESH_BITS(LM_HEAD);
			}

			auto frameData = GetFrameInterpData(tempItem);
			UpdateAnimation(&rItem, playerObject, frameData, mask);

			// Right arm
			mask = MESH_BITS(LM_RINARM) | MESH_BITS(LM_ROUTARM) | MESH_BITS(LM_RHAND);
			frameData = GetFrameInterpData(*LaraItem);
			UpdateAnimation(&rItem, playerObject, frameData, mask);
			break;
		}
	}

	// Copy matrices in player object.
	for (int m = 0; m < NUM_LARA_MESHES; m++)
		playerObject.AnimationTransforms[m] = rItem.AnimationTransforms[m];

	// Copy meshswap indices.
	rItem.MeshIndex = LaraItem->Model.MeshIndex;
	rItem.DoneAnimations = true;
}

void TEN::Renderer::Renderer11::DrawLara(RenderView& view, bool transparent)
{
	// Don't draw Lara if binoculars or sniper
	if (BinocularRange || SpotcamDontDrawLara)
		return;

	// Don't draw Lara if title level and disabled
	if (CurrentLevel == 0 && !g_GameFlow->IsLaraInTitleEnabled())
		return;

	auto* item = &m_items[LaraItem->Index];
	auto* nativeItem = &g_Level.Items[item->ItemNumber];

	if (nativeItem->Flags & IFLAG_INVISIBLE)
		return;

	unsigned int stride = sizeof(RendererVertex);
	unsigned int offset = 0;

	m_context->IASetVertexBuffers(0, 1, m_moveablesVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout.Get());
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	m_context->VSSetShader(m_vsItems.Get(), nullptr, 0);
	m_context->PSSetShader(m_psItems.Get(), nullptr, 0);

	// Set texture
	BindTexture(TEXTURE_COLOR_MAP, &std::get<0>(m_moveablesTextures[0]), SAMPLER_LINEAR_CLAMP);
	BindTexture(TEXTURE_NORMAL_MAP, &std::get<1>(m_moveablesTextures[0]), SAMPLER_NONE);

	SetAlphaTest(ALPHA_TEST_GREATER_THAN, ALPHA_TEST_THRESHOLD);

	RendererObject& laraObj = *m_moveableObjects[ID_LARA];
	RendererObject& laraSkin = GetRendererObject(GAME_OBJECT_ID::ID_LARA_SKIN);

	RendererRoom* room = &m_rooms[LaraItem->RoomNumber];

	m_stItem.World = m_LaraWorldMatrix;
	m_stItem.Color = item->Color;
	m_stItem.AmbientLight = item->AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj.AnimationTransforms.data(), laraObj.AnimationTransforms.size() * sizeof(Matrix));
	for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
	{
		m_stItem.BoneLightModes[k] = GetMesh(nativeItem->Model.MeshIndex[k])->LightMode;
	}
	BindMoveableLights(item->LightsToDraw, item->RoomNumber, item->PrevRoomNumber, item->LightFade);
	m_cbItem.updateData(m_stItem, m_context.Get());
	BindConstantBufferVS(CB_ITEM, m_cbItem.get());
	BindConstantBufferPS(CB_ITEM, m_cbItem.get());

	for (int k = 0; k < laraSkin.ObjectMeshes.size(); k++)
	{
		if (!nativeItem->MeshBits.Test(k))
			continue;

		DrawMoveableMesh(item, GetMesh(nativeItem->Model.MeshIndex[k]), room, k, transparent);
	}

	DrawLaraHolsters(item, room, transparent);
	DrawLaraJoints(item, room, transparent);
	DrawLaraHair(item, room, transparent);
}

void Renderer11::DrawLaraHair(RendererItem* itemToDraw, RendererRoom* room, bool transparent)
{
	if (!Objects[ID_HAIR].loaded)
		return;

	const auto& hairObject = *m_moveableObjects[ID_HAIR];

	// TODO
	bool isYoung = (g_GameFlow->GetLevel(CurrentLevel)->GetLaraType() == LaraType::Young);

	bool isHead = true;
	for (const auto& unit : HairEffect.Units)
	{
		if (!unit.IsEnabled)
			continue;

		// First matrix is Lara's head matrix, then all hair unit segment matrices.
		// Bones are adjusted at load time to account for this.
		m_stItem.World = Matrix::Identity;
		m_stItem.BonesMatrices[0] = itemToDraw->AnimationTransforms[LM_HEAD] * m_LaraWorldMatrix;

		for (int i = 0; i < unit.Segments.size(); i++)
		{
			const auto& segment = unit.Segments[i];
			auto worldMatrix = Matrix::CreateFromQuaternion(segment.Orientation) * Matrix::CreateTranslation(segment.Position);

			m_stItem.BonesMatrices[i + 1] = worldMatrix;
			m_stItem.BoneLightModes[i] = LIGHT_MODES::LIGHT_MODE_DYNAMIC;
		}

		m_cbItem.updateData(m_stItem, m_context.Get());
		BindConstantBufferVS(CB_ITEM, m_cbItem.get());
		BindConstantBufferPS(CB_ITEM, m_cbItem.get());

		for (int i = 0; i < hairObject.ObjectMeshes.size(); i++)
		{
			auto& rMesh = *hairObject.ObjectMeshes[i];
			DrawMoveableMesh(itemToDraw, &rMesh, room, i, transparent);
		}

		isHead = false;
	}
}

void Renderer11::DrawLaraJoints(RendererItem* itemToDraw, RendererRoom* room, bool transparent)
{
	if (!m_moveableObjects[ID_LARA_SKIN_JOINTS].has_value())
		return;

	RendererObject& laraSkinJoints = *m_moveableObjects[ID_LARA_SKIN_JOINTS];

	for (int k = 1; k < laraSkinJoints.ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = laraSkinJoints.ObjectMeshes[k];
		DrawMoveableMesh(itemToDraw, mesh, room, k, transparent);
	}
}

void Renderer11::DrawLaraHolsters(RendererItem* itemToDraw, RendererRoom* room, bool transparent)
{
	HolsterSlot leftHolsterID = Lara.Control.Weapon.HolsterInfo.LeftHolster;
	HolsterSlot rightHolsterID = Lara.Control.Weapon.HolsterInfo.RightHolster;
	HolsterSlot backHolsterID = Lara.Control.Weapon.HolsterInfo.BackHolster;

	if (m_moveableObjects[static_cast<int>(leftHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(leftHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_LTHIGH];
		DrawMoveableMesh(itemToDraw, mesh, room, LM_LTHIGH, transparent);
	}

	if (m_moveableObjects[static_cast<int>(rightHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(rightHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_RTHIGH];
		DrawMoveableMesh(itemToDraw, mesh, room, LM_RTHIGH, transparent);
	}

	if (backHolsterID != HolsterSlot::Empty && m_moveableObjects[static_cast<int>(backHolsterID)])
	{
		RendererObject& holsterSkin = *m_moveableObjects[static_cast<int>(backHolsterID)];
		RendererMesh* mesh = holsterSkin.ObjectMeshes[LM_TORSO];
		DrawMoveableMesh(itemToDraw, mesh, room, LM_TORSO, transparent);
	}
}
