#include "framework.h"
#include "Game/effects/lightning.h"

#include "Game/animation.h"
#include "Game/effects/bubble.h"
#include "Game/effects/drip.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

using std::vector;
using TEN::Renderer::g_Renderer;

namespace TEN::Effects::Lightning
{
	constexpr auto MAX_ENERGYARCS = 32;
	
	int LightningRandomSeed = 0x0D371F947;
	float FloatSinCosTable[8192];
	Vector3Int LightningPos[6];
	short LightningBuffer[1024];
	
	std::vector<LIGHTNING_INFO> Lightning;

	void InitialiseFloatSinCosTable()
	{
		for (int i = 0; i < 8192; i++)
		{
			FloatSinCosTable[i] = sin(i * 0.000095873802f);
		}
	}

	void UpdateLightning()
	{
		for (int i = 0; i < Lightning.size(); i++)
		{
			LIGHTNING_INFO* arc = &Lightning[i];

			if (arc->life > 0)
			{
				// If/when this behaviour is changed, please modify AddLightningArc accordingly
				arc->life -= 2;
				if (arc->life)
				{
					int* positions = (int*)&arc->pos2;
					for (int j = 0; j < 9; j++)
					{
						*positions += 2 * arc->interpolation[j];
						arc->interpolation[j] = (signed char)(arc->interpolation[j] - (arc->interpolation[j] >> 4));
						positions++;
					}
				}
			}
		}

		if (Lightning.size() > 0)
		{
			Lightning.erase(
				std::remove_if(Lightning.begin(), Lightning.end(),
					[](const LIGHTNING_INFO& o) { return o.life == 0; }),
				Lightning.end());
		}
	}

	void TriggerLightning(Vector3Int* src, Vector3Int* dest, unsigned char amplitude, unsigned char r, unsigned char g, unsigned char b, unsigned char life, char flags, char width, char segments)
	{
		LIGHTNING_INFO arc;

		arc.pos1 = *src;
		arc.pos2.x = (dest->x + 3 * src->x) >> 2;
		arc.pos2.y = (dest->y + 3 * src->y) >> 2;
		arc.pos2.z = (dest->z + 3 * src->z) >> 2;
		arc.pos3.x = (src->x + 3 * dest->x) >> 2;
		arc.pos3.y = (src->y + 3 * dest->y) >> 2;
		arc.pos3.z = (src->z + 3 * dest->z) >> 2;
		arc.pos4 = *dest;
		arc.flags = flags;

		for (int i = 0; i < 9; i++)
		{
			if (arc.flags & 2 || i < 6)
				arc.interpolation[i] = ((unsigned char)(GetRandomControl() % amplitude) - (unsigned char)(amplitude >> 1));
			else
				arc.interpolation[i] = 0;
		}

		arc.r = r;
		arc.g = g;
		arc.b = b;
		arc.life = life;
		arc.segments = segments;
		arc.amplitude = amplitude;
		arc.width = width;

		Lightning.push_back(arc);
	}

	// New function used in TR5, we'll decompile it in the future
	void DrawLightningNew()
	{
		/*Vector4 positions[32];
		Vector3 transformed[32];
		short clipBuffer[32];
		byte r, g, b;
		ENERGY_ARC_VBUFFER vbuffer[32];

		for (int i = 0; i < 16; i++)
		{
			ENERGY_ARC* arc = &EnergyArcs[i];

			if (arc->life)
			{
				float dx = arc->pos0.x - LaraItem->pos.Position.x;
				float dy = arc->pos0.y - LaraItem->pos.Position.y;
				float dz = arc->pos0.z - LaraItem->pos.Position.z;

				float x1 = arc->pos1.x - arc->pos0.x;
				float y1 = arc->pos1.y - arc->pos0.y;
				float z1 = arc->pos1.z - arc->pos0.z;

				float x2 = arc->pos2.x - arc->pos0.x;
				float y2 = arc->pos2.y - arc->pos0.y;
				float z2 = arc->pos2.z - arc->pos0.z;

				float x3 = arc->pos3.x - arc->pos0.x;
				float y3 = arc->pos3.y - arc->pos0.y;
				float z3 = arc->pos3.z - arc->pos0.z;

				double factor = 0;

				int seed = LightningRandomSeed;

				for (int j = 0; j < 32; j++)
				{
					float rx = 0;
					float ry = 0;
					float rz = 0;

					if (j > 0 && j < 31)
					{
						// random number generator
						int rndX = 1103515245 * seed + 12345;
						int rndY = 1103515245 * rndX + 12345;
						int rndZ = 1103515245 * rndY + 12345;

						float rx = (float)(((rndX >> 10) & 0xF) - 8);
						float ry = (float)(((rndY >> 10) & 0xF) - 8);
						float rz = (float)(((rndZ >> 10) & 0xF) - 8);

						LightningRandomSeed = rndZ;
					}

					float x = (1.0 - factor) * SQUARE(factor) * 4.0;
					float y = (2 * factor - 1.0) * SQUARE(factor);
					float z = SQUARE(1.0 - factor) * factor * 4.0;

					positions[j].x = x * x1 + y * x2 + z * x3 + 0.0 + rx + dx;
					positions[j].y = x * y1 + y * y2 + z * y3 + 0.0 + ry + dy;
					positions[j].z = x * z1 + y * z2 + z * z3 + 0.0 + rz + dz;
					positions[j].w = 1.0;

					factor += 0.03125f;
				}

				LightningRandomSeed = seed;

				for (int j = 0; j < 32; j++)
				{
					Vector3 transformed = Vector4::Transform()
					pPosY = *(pPosZ - 1);
					pPosX = *(pPosZ - 2);
					pPosY2 = pPosY;
					v36 = transform_view._21 * pPosY;
					v37 = 0;
					tx = transform_view._31 * *pPosZ + v36 + transform_view._11 * pPosX + transform_view._41;
					ty = transform_view._22 * pPosY2 + transform_view._12 * pPosX + transform_view._32 * *pPosZ + transform_view._42;
					c_sz = transform_view._33 * *pPosZ
						+ transform_view._23 * pPosY2
						+ transform_view._13 * pPosX
						+ transform_view._43;
					if (c_sz < znear)
						v37 = -128;
					t_persp = f_persp / c_sz;
					c_sx = t_persp * tx + f_centerx;
					*(D3DTLVERTEX_cy - 1) = c_sx;
					*D3DTLVERTEX_cy = t_persp * ty + f_centery;
					D3DTLVERTEX_cy[1] = t_persp * f_oneopersp;
					if (c_sx >= winX)
					{
						if ((double)phd_winxmax < *(D3DTLVERTEX_cy - 1))
							v37 += 2;
					}
					else
					{
						++v37;
					}
					if (*D3DTLVERTEX_cy >= winY)
					{
						if ((double)phd_winymax < *D3DTLVERTEX_cy)
							v37 += 8;
					}
					else
					{
						v37 += 4;
					}
					D3DTLVERTEX_cy[2] = c_sz;
					*(_WORD*)someFlag = v37;
					D3DTLVERTEX_cy[3] = tx;
					D3DTLVERTEX_cy[4] = ty;
					someFlag += 2;
					pPosZ += 4;
					D3DTLVERTEX_cy += 8;
					--v32;
				}
				}

				if (arc->life >= 16)
				{
					r = arc->r;
					g = arc->g;
					b = arc->b;
				}
				else
				{
					r = arc->r >> 4;
					g = arc->g >> 4;
					b = arc->b >> 4;
				}

				float length = 0.0;
				float width = (float)(arc->width >> 1);

				if (arc->flags & 8)
				{
					length = width * 0.125;
					width = 0.0;
				}
				else if (arc->flags & 4)
				{
					length = -(width * 0.03125);
				}

				for (int j = 0; j < 32; j++)
				{
					short angle = -phd_atan(vbuffer);
					float sinAngle = FloatSinCosTable[angle + 0x4000];
					width = max(width, 0);

				}

				v45 = 0;
				v46 = (float*)&v125;
				do
				{
					v47 = -j_phd_atan((signed __int64)(v46[7] - *(v46 - 1)), (signed __int64)(v46[8] - *v46));
					v48 = FloatSinCosTable[(unsigned __int16)(v47 + 0x4000)];
					v49 = v88;
					if (v88 <= 0.0)
						v49 = 2.0;
					v50 = flt_51D15C / v46[2] * v49;
					v51 = (*v42 & 8) == 0;
					*v86 = FloatSinCosTable[(unsigned __int16)v47] * v50;
					v86[1] = v48 * v50;
					v88 = v95 + v88;
					if (!v51 && v45 == 8)
					{
						if (*v42 & 4)
							v95 = (double)(unsigned __int8)(*(_BYTE*)(arc + 53) >> 1) * -0.035714287;
						else
							v95 = 0.0;
						*v42 &= 0xF7u;
					}
					v46 += 8;
					++v45;
					v86 += 4;
				}       while (v45 < 32);
			}
		}*/
	}

	int LSpline(int x, int* knots, int nk)
	{
		int* k;
		int c1, c2, c3, ret, span;

		x *= nk - 3;
		span = x >> 16;

		if (span >= nk - 3)
			span = nk - 4;

		x -= 65536 * span;
		k = &knots[3 * span];
		c1 = k[3] + (k[3] >> 1) - (k[6] >> 1) - k[6] + (k[9] >> 1) + ((-k[0] - 1) >> 1);
		ret = (long long)c1 * x >> 16;
		c2 = ret + 2 * k[6] - 2 * k[3] - (k[3] >> 1) - (k[9] >> 1) + k[0];
		ret = (long long)c2 * x >> 16;
		c3 = ret + (k[6] >> 1) + ((-k[0] - 1) >> 1);
		ret = (long long)c3 * x >> 16;

		return ret + k[3];
	}

	void CalcLightningSpline(Vector3Int* pos, short* buffer, LIGHTNING_INFO* arc)
	{
		buffer[0] = pos->x;
		buffer[1] = pos->y;
		buffer[2] = pos->z;

		buffer += 4;

		if (arc->flags & 1)
		{
			int dp = 65536 / (3 * arc->segments - 1);
			int x = dp;

			if (3 * arc->segments - 2 > 0)
			{
				for (int i = 3 * arc->segments - 2; i > 0; i--)
				{
					short sx = LSpline(x, &pos->x, 6);
					buffer[0] = sx + (GetRandomControl() & 0xF) - 8;
					short sy = LSpline(x, &pos->y, 6);
					buffer[1] = sy + (GetRandomControl() & 0xF) - 8;
					short sz = LSpline(x, &pos->z, 6);
					buffer[2] = sz + (GetRandomControl() & 0xF) - 8;

					x += dp;
					buffer += 4;
				}
			}
		}
		else
		{
			int segments = 3 * arc->segments - 1;

			int dx = (pos[5].x - pos->x) / segments;
			int dy = (pos[5].y - pos->y) / segments;
			int dz = (pos[5].z - pos->z) / segments;

			int x = dx + (GetRandomControl() % (2 * arc->amplitude)) - arc->amplitude + pos->x;
			int y = dy + (GetRandomControl() % (2 * arc->amplitude)) - arc->amplitude + pos->y;
			int z = dz + (GetRandomControl() % (2 * arc->amplitude)) - arc->amplitude + pos->z;

			if (3 * arc->segments - 2 > 0)
			{
				for (int i = 3 * arc->segments - 2; i > 0; i--)
				{
					buffer[0] = x;
					buffer[1] = y;
					buffer[2] = z;

					x += dx + GetRandomControl() % (2 * arc->amplitude) - arc->amplitude;
					y += dy + GetRandomControl() % (2 * arc->amplitude) - arc->amplitude;
					z += dz + GetRandomControl() % (2 * arc->amplitude) - arc->amplitude;

					buffer += 4;
				}
			}
		}

		buffer[0] = pos[5].x;
		buffer[1] = pos[5].y;
		buffer[2] = pos[5].z;
	}


	void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b)
	{
		auto* spark = GetFreeParticle();

		spark->dG = g;
		spark->sG = g;
		spark->life = 4;
		spark->sLife = 4;
		spark->dR = r;
		spark->sR = r;
		spark->colFadeSpeed = 2;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->on = 1;
		spark->dB = b;
		spark->sB = b;
		spark->fadeToBlack = 0;
		spark->x = x;
		spark->y = y;
		spark->z = z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->flags = SP_DEF | SP_SCALE;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->spriteIndex = Objects[ID_MISC_SPRITES].meshIndex;
		spark->gravity = 0;
		spark->dSize = spark->sSize = spark->size = size + (GetRandomControl() & 3);
	}
}
