#pragma once

#include "Quad.h"
#include "Engine/Core/Core.h"

struct RPlane
{
	vec3 Normal;
	vec3 Center;
	float Size;

	RPlane() = default;

	RPlane(vec3 Normal, vec3 Center, float Size) : Normal(Normal), Center(Center), Size(Size)
	{
		V1 = normalize(Cross(Normal, UnitY));
		V2 = normalize(Cross(Normal, V1));

		/* =================
		 *		2 --- 0
		 *		|  \  |
		 *		3 --- 1
		 * ================= */
		Vertices[0] = Center + V1 * Size / 2.f + V2 * Size / 2.f;
		Vertices[1] = Center + V1 * Size / 2.f - V2 * Size / 2.f;
		Vertices[2] = Center - V1 * Size / 2.f + V2 * Size / 2.f;
		Vertices[3] = Center - V1 * Size / 2.f - V2 * Size / 2.f;
	}

	vec3 GetVertex(int Index)
	{
		if (Index < 0 || Index > 4) return {};
		return Vertices[Index];
	}

	RQuad GetQuad()
	{
		return RQuad{{Vertices[0], Vertices[1], Vertices[2]}, {Vertices[2], Vertices[3], Vertices[1]}};
	}
	
private:
	vec3 V1;
	vec3 V2;
	vec3 Vertices[4];
};
