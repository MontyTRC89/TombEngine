#pragma once

#define RGB555(r, g, b) ((r << 7) & 0x7C00 | (g << 2) & 0x3E0 | (b >> 3) & 0x1F)
#define RED5(rgb) (((rgb >> 10) & 0x1F) << 3)
#define GREEN5(rgb) (((rgb >> 5) & 0x1F) << 3)
#define BLUE5(rgb) ((rgb & 0x1F) << 3)
#define WHITE555 RGB555(255, 255, 255)
#define GRAY555  RGB555(128, 128, 128)
#define BLACK555 RGB555(  0,   0,   0)
#define XZ_GET_SECTOR(room, x, z) (r->floor[(int)((z) / 1024 + r->x_size * ((x) / 1024))])

#define ADD_PTR(p, t, n) p = (t*)((char*)(p) + (ptrdiff_t)(n)); 

#define GF(a,b)	(Anims[a].frameBase+b)
#define GF2(a,b,c)	(Anims[Objects[a].animIndex + b].frameBase+c)