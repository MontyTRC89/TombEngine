#pragma once

#define RGB555(r, g, b) ((r << 7) & 0x7C00 | (g << 2) & 0x3E0 | (b >> 3) & 0x1F)
#define RED5(rgb) (((rgb >> 10) & 0x1F) << 3)
#define GREEN5(rgb) (((rgb >> 5) & 0x1F) << 3)
#define BLUE5(rgb) ((rgb & 0x1F) << 3)
#define WHITE555 RGB555(255, 255, 255)
#define GRAY555  RGB555(128, 128, 128)
#define BLACK555 RGB555(  0,   0,   0)
#define XZ_GET_SECTOR(room, x, z) (room->floor[((z) >> WALL_SHIFT) + ((x) >> WALL_SHIFT) * room->xSize])

#define ADD_PTR(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n)); 

///#define ZONE(A) (((A) >> (WALL_SIZE - 2)) - 1)
#define ZONE(A) FlipStatus + 2 * A
#define CHECK_CLICK(x) CLICK(x) / 2

#define RED(rgb) (((rgb) >> 16) & 0xFF)
#define GREEN(rgb) (((rgb) >> 8) & 0xFF)
#define BLUE(rgb) ((rgb) & 0xFF)
#define ALPHA(rgb) (((rgb) >> 24) & 0xFF)

#define MESHES(slot, mesh) Meshes[Objects[slot].meshIndex + mesh]
#define MESH_BITS(x) (1 << x)
#define LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] = MESHES(slot, mesh)
#define CHECK_LARA_MESHES(slot, mesh) Lara.meshPtrs[mesh] == MESHES(slot, mesh)
#define INIT_LARA_MESHES(mesh, to, from) Lara.meshPtrs[mesh] = LARA_MESHES(to, mesh) = LARA_MESHES(from, mesh)

#define HIDWORD(l) ((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))