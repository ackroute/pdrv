#pragma once

#include "Vector.h"

#define WIDTH						1920
#define HEIGHT						1080

#define o_entitylist				0x183F118
#define o_locale					0x1D9AB98

#define o_viewmatrix				0x1A93D0
#define o_viewrender				0xD4138F0

#define o_glowenable				0x390
#define o_glowcontext				0x310
#define o_glowrange					0x2FC 
#define o_glowfade					0x2C8
#define o_glowcolors				0x1D0

#define o_health					0x3E0
#define o_shield					0x170
#define o_name						0x518

#define o_origin					0x14C
#define o_bones						0xEE0
#define o_aimpunch					0x20C0
#define o_camerapos					0x1B68
#define o_viewangles				0x2188

bool W2S(Vector location, Vector& out)
{

	uintptr_t view_render = *(uint64_t*)(ProcessBase + o_viewrender);
	uintptr_t pview_matrix = *(uint64_t*)(view_render + o_viewmatrix);
	D3DMATRIX viewmatrix = *(D3DMATRIX*)(pview_matrix);

	Vector vTransformed;

	vTransformed.x = (float)(location.y * viewmatrix.m[0][1]) + (float)(location.x * viewmatrix.m[0][0]) + (float)(location.z * viewmatrix.m[0][2]) + viewmatrix.m[0][3];
	vTransformed.y = (float)(location.y * viewmatrix.m[1][1]) + (float)(location.x * viewmatrix.m[1][0]) + (float)(location.z * viewmatrix.m[1][2]) + viewmatrix.m[1][3];
	vTransformed.z = (float)(location.y * viewmatrix.m[3][1]) + (float)(location.x * viewmatrix.m[3][0]) + (float)(location.z * viewmatrix.m[3][2]) + viewmatrix.m[3][3];

	if (vTransformed.z < 0.001)
		return 0;

	vTransformed.x *= 1.0 / vTransformed.z;
	vTransformed.y *= 1.0 / vTransformed.z;

	out.x = 1920 / 2.0f + vTransformed.x * (1920 / 2.0f);
	out.y = 1080 / 2.0f - vTransformed.y * (1080 / 2.0f);
	return 1;

}