#include "framework.h"
#include "Objects/TR4/Trap/tr4_chain.h"

#include "Game/collision/Sphere.h"
#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/collision/collide_item.h"
#include "Specific/level.h"
#include "Objects/Generic/Object/burning_torch.h"





namespace TEN::Entities::Traps
{

	constexpr auto PENDULUML_HARM_DAMAGE = 15;
	constexpr auto PENDULUML_FIRE_NODE = 4;
	const auto PendulumBite = CreatureBiteInfo(Vector3(0.0f, 0.0f, 0.0f), 4);

	void TriggerPendulumFlame(int itemNumber, Vector3i pos)
	{
		auto& item = g_Level.Items[itemNumber];
		int x, z;

		x = LaraItem->Pose.Position.x - item.Pose.Position.x;
		z = LaraItem->Pose.Position.z - item.Pose.Position.z;

		//if (x < -0x4000 || x > 0x4000 || z < -0x4000 || z > 0x4000)
			//return;

		auto* spark = GetFreeParticle();
		spark->on = 1;
		spark->sR = (GetRandomControl() & 0x1F) + 48;
		spark->sG = spark->sR >> 1;
		spark->sB = 0;
		spark->dR = (GetRandomControl() & 0x3F) + 192;
		spark->dG = (GetRandomControl() & 0x3F) + 128;
		spark->dB = 32;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 12;
		spark->fadeToBlack = 8;
		//spark->fxObj = itemNumber;
		//spark->nodeNumber = node;

		spark->extras = 0;
		spark->life = Random::GenerateInt(1,15);
		spark->sLife = spark->life;
		spark->dynamic = 1;

		spark->xVel = (GetRandomControl() & 0x3F) - 32;
		spark->yVel = -16 - (GetRandomControl() & 0xF);
		spark->zVel = (GetRandomControl() & 0x3F) - 32;
		spark->friction = 4;
		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_ITEM ;

		if (GetRandomControl() & 1)
		{
			spark->flags |= SP_ROTATE;
			spark->rotAng = GetRandomControl() & 0xFFF;
			spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		}



		// Set the position of the spark relative to the node

		spark->x = pos.x + (GetRandomControl() & 0x1F) - 16;
		spark->y = pos.y;
		spark->z = pos.z + (GetRandomControl() & 0x1F) - 16;

		spark->gravity = -16 - (GetRandomControl() & 0x1F);
		spark->maxYvel = -16 - (GetRandomControl() & 7);
		spark->scalar = spark->life < 32 ? 4 : 3;
		spark->size = (GetRandomControl() & 7) + 20;
		spark->sSize = spark->size;
		spark->dSize = static_cast<int>(spark->size) >> 2;

		spark->SpriteSeqID = ID_DEFAULT_SPRITES;
		spark->SpriteID = 0;
	}

	Vector3i GetNodePosition(const ItemInfo& item, unsigned char node)
	{
		// Berechne die Position des Knotens relativ zur Pose des Items
		// Dies ist ein Platzhalter. Die tatsächliche Berechnung hängt von der Struktur des Items und der Knoten ab.
		Vector3i nodePos;
		// Beispielberechnung (muss an die tatsächliche Struktur angepasst werden):
		nodePos.x = item.Pose.Position.x + node ; // Beispielwert
		nodePos.y = item.Pose.Position.y + node - 125;  // Beispielwert
		nodePos.z = item.Pose.Position.z + node ; // Beispielwert
		return nodePos;
	}




	void ControlChain(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item))
			return;

		if (item.TriggerFlags > 0)
		{
			item.ItemFlags[2] = 1;
			item.ItemFlags[3] = 75;
			item.ItemFlags[5] = 1;

			if (TriggerActive(&item))
			{
				*(int*)&item.ItemFlags[0] = 0x787E;
				AnimateItem(&item);

				auto pos = GetJointPosition(item, 4, Vector3i(0, 260, 0));
				byte r = 51 - ((GetRandomControl() / 16) & 6);
				byte g = 44 - ((GetRandomControl() / 64) & 6);
				byte b = GetRandomControl() & 10;
				SpawnDynamicLight(pos.x, pos.y, pos.z, 12, r, g, b);
				SpawnDynamicFogBulb(pos.x, pos.y, pos.z, 4, 0.08f, r+(115 - ((GetRandomControl() / 16) & 6)), g+(108- ((GetRandomControl() / 16) & 8)), b);

				//if (Wibble & 6)
				//{
					TriggerPendulumFlame(itemNumber, pos);
				//}

				return;
			}
		}
		else 
		{
			item.ItemFlags[3] = 25;

			if (TriggerActive(&item))
			{
				*(int*)&item.ItemFlags[0] = 0x780;
				AnimateItem(&item);
				return;
			}
		}

		*(int*)&item.ItemFlags[0] = 0;
	}

	void CollideChain(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];
		if (item.Status == ITEM_INVISIBLE)
			return;

		if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
			return;

		TEN::Collision::Sphere::HandleItemSphereCollision(item, *playerItem);
		if (!item.TouchBits.TestAny())
			return;

		short prevYOrient = item.Pose.Orientation.y;
		item.Pose.Orientation.y = 0;
		auto spheres = item.GetSpheres();
		item.Pose.Orientation.y = prevYOrient;

		int harmBits = *(int*)&item.ItemFlags[0]; // NOTE: Value spread across ItemFlags[0] and ItemFlags[1].

		auto collidedBits = item.TouchBits;
		if (item.ItemFlags[2] != 0)
			collidedBits.Clear(0);

		coll->Setup.EnableObjectPush = (item.ItemFlags[4] == 0);

		// Handle push and damage.
		for (int i = 0; i < spheres.size(); i++)
		{
			if (collidedBits.Test(i))
			{
				const auto& sphere = spheres[i];

				GlobalCollisionBounds.X1 = sphere.Center.x - sphere.Radius - item.Pose.Position.x;
				GlobalCollisionBounds.X2 = sphere.Center.x + sphere.Radius - item.Pose.Position.x;
				GlobalCollisionBounds.Y1 = sphere.Center.y - sphere.Radius - item.Pose.Position.y;
				GlobalCollisionBounds.Y2 = sphere.Center.y + sphere.Radius - item.Pose.Position.y;
				GlobalCollisionBounds.Z1 = sphere.Center.z - sphere.Radius - item.Pose.Position.z;
				GlobalCollisionBounds.Z2 = sphere.Center.z + sphere.Radius - item.Pose.Position.z;

				auto pos = playerItem->Pose.Position;
				if (ItemPushItem(&item, playerItem, coll, harmBits & 1, 3) && (harmBits & 1))
				{
					DoDamage(playerItem, item.ItemFlags[3]);

					if (item.ItemFlags[5])
						TEN::Effects::Items::ItemBurn(LaraItem);

					auto deltaPos = pos - playerItem->Pose.Position;
					if (deltaPos != Vector3i::Zero)
					{
						if (TriggerActive(&item))
							TriggerLaraBlood();
					}

					if (!coll->Setup.EnableObjectPush)
						playerItem->Pose.Position += deltaPos;
				}
			}

			harmBits >>= 1;
		}
	}

}