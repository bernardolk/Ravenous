#pragma once

#include "engine/utils/utils.h"

struct RTriangle;

struct RFace
{
	RTriangle A;
	RTriangle B;
	vec3 Center;
};


// -----------------------------
// > Triangle / Face operations
// -----------------------------
inline RFace FaceFromAxisAlignedTriangle(RTriangle Triangle)
{
	// computes center
	float X0 = Min(Triangle.A.x, Triangle.B.x, Triangle.C.x);
	float X1 = Max(Triangle.A.x, Triangle.B.x, Triangle.C.x);
	float Y0 = Min(Triangle.A.y, Triangle.B.y, Triangle.C.y);
	float Y1 = Max(Triangle.A.y, Triangle.B.y, Triangle.C.y);
	float Z0 = Min(Triangle.A.z, Triangle.B.z, Triangle.C.z);
	float Z1 = Max(Triangle.A.z, Triangle.B.z, Triangle.C.z);

	float Mx, My, Mz;
	Mx = X0 == X1 ? X0 : ((X1 - X0) / 2.0f) + X0;
	My = Y0 == Y1 ? Y0 : ((Y1 - Y0) / 2.0f) + Y0;
	Mz = Z0 == Z1 ? Z0 : ((Z1 - Z0) / 2.0f) + Z0;
	auto Center = vec3{Mx, My, Mz};

	vec3 Normal = triangleNormal(Triangle.A, Triangle.B, Triangle.C);

	vec3 A2 = rotate(Triangle.A, glm::radians(180.0f), Normal);
	vec3 B2 = rotate(Triangle.B, glm::radians(180.0f), Normal);
	vec3 C2 = rotate(Triangle.C, glm::radians(180.0f), Normal);

	vec3 Translation;
	if (X0 == X1)
		Translation = vec3(0, Center.y, Center.z);
	if (Y0 == Y1)
		Translation = vec3(Center.x, 0, Center.z);
	if (Z0 == Z1)
		Translation = vec3(Center.x, Center.y, 0);

	A2 += Translation * 2.0f;
	B2 += Translation * 2.0f;
	C2 += Translation * 2.0f;

	auto T2 = RTriangle{A2, B2, C2};

	RFace Face;
	Face.A = Triangle;
	Face.B = T2;
	Face.Center = Center;

	return Face;
}
