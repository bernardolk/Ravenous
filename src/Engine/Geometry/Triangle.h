#pragma once
#include "Engine/Core/Core.h"

struct RTriangle
{
	vec3 A = vec3{0.f};
	vec3 B = vec3{0.f};
	vec3 C = vec3{0.f};

	vec3 GetNormal() { return triangleNormal(A, B, C); }

	vec3 GetBarycenter()
	{
		auto Bx = (A.x + B.x + C.x) / 3;
		auto By = (A.y + B.y + C.y) / 3;
		auto Bz = (A.z + B.z + C.z) / 3;
		return vec3(Bx, By, Bz);
	}

	bool IsValid()
	{
		// checks if vertices are not in a single point
		return A != B && A != C && B != C;
	}

	bool operator ==(const RTriangle& Other)
	{
		return A == Other.A && B == Other.B && C == Other.C;
	}
};
