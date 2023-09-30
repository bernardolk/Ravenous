#pragma once

#include <ctime>
#include "engine/core/core.h"

inline string FormatFloatTostr(float Num, int Precision = 3)
{
	string Temp = std::to_string(Num);
	return Temp.substr(0, Temp.find(".") + Precision);
}

inline string FmtTostr(float Num, int Precision)
{
	return FormatFloatTostr(Num, Precision);
}

inline string ToString(vec3 Vec)
{
	return "(" + std::to_string(Vec.x) + ", " + std::to_string(Vec.y) + ", " + std::to_string(Vec.z) + ")";
}

inline string ToString(vec2 Vec)
{
	return "(" + std::to_string(Vec.x) + ", " + std::to_string(Vec.y) + ")";
}


enum FloatTolerance
{
	FloatTolerance_3_001    = 0,
	FloatTolerance_4_0001   = 1,
	FloatTolerance_5_00001  = 2,
	FloatTolerance_6_000001 = 3,
};

inline bool AreEqualFloats(float x, float y, FloatTolerance Tolerance = FloatTolerance_5_00001)
{
	/* checks if two floats are equal within a certain tolerance level */
	float Tolerances[] = {0.001f, 0.0001f, 0.00001f, 0.000001f};
	return abs(x - y) < Tolerances[Tolerance];
}

// SIGN COMPARISON
inline bool CompSign(float a, float b)
{
	return a * b >= 0.f;
}

inline float Sign(float a)
{
	return a < 0 ? -1 : 1;
}

// VECTOR LENGTH COMPARISON
inline bool SquareEq(vec3 v, float n)
{
	return v.x * v.x + v.y * v.y + v.z * v.z == n * n;
}

inline bool SquareLt(vec3 v, float n)
{
	return v.x * v.x + v.y * v.y + v.z * v.z < n * n;
}

inline bool SquareGt(vec3 v, float n)
{
	return v.x * v.x + v.y * v.y + v.z * v.z > n * n;
}

inline bool SquareLe(vec3 v, float n)
{
	return v.x * v.x + v.y * v.y + v.z * v.z <= n * n;
}

inline bool SquareGe(vec3 v, float n)
{
	return v.x * v.x + v.y * v.y + v.z * v.z >= n * n;
}

inline float Abs(vec3 v)
{
	return length(v);
}

// VECTOR ANGLE
inline float VectorAngle(vec2 a, vec2 b)
{
	float Dot = glm::dot(a, b);
	float LenA = length(a);
	float LenB = length(b);
	float Theta = acos(Dot / (LenA * LenB));
	return Theta;
}

inline float VectorAngle(vec3 a, vec3 b)
{
	float Dot = glm::dot(a, b);
	float LenA = length(a);
	float LenB = length(b);
	float Theta = acos(Dot / (LenA * LenB));
	return Theta;
}

inline float VectorAngleSigned(vec2 a, vec2 b)
{
	return atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

inline float VectorCos(vec2 a, vec2 b)
{
	float Dot = glm::dot(a, b);
	float LenA = length(a);
	float LenB = length(b);
	float Cos = Dot / (LenA * LenB);
	return Cos;
}

// VECTOR DIMENSION CONVERSION
inline vec3 ToXz(vec3 Vector)
{
	return vec3(Vector.x, 0, Vector.z);
}


inline vec3 Cross(vec3 a, vec3 b)
{
	return glm::cross(a, b);
}

inline vec3 ProjectVecIntoRef(vec3 Vec, vec3 Ref)
{
	auto Proj = dot(Vec, Ref) / (Abs(Ref) * Abs(Ref)) * Ref;
	return Proj;
}

inline vec3 ProjectVecOntoPlane(vec3 v, vec3 n)
{
	auto VProjN = ProjectVecIntoRef(v, n);
	auto VProjPlane = v - VProjN;
	return VProjPlane;
}


// OTHER MATHS
inline vec3 RotMatToEulerAnglesXyz(mat4& m)
{
	/*
	   This is equivalent to calling glm::extractEulerAngleXYZ
	*/

	float sy = sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);

	bool Singular = sy < 1e-6; // If

	float x, y, z;
	if (!Singular)
	{
		x = atan2(m[2][1], m[2][2]);
		y = atan2(-m[2][0], sy);
		z = atan2(m[1][0], m[0][0]);
	}
	else
	{
		x = atan2(-m[1][2], m[1][1]);
		y = atan2(-m[2][0], sy);
		z = 0;
	}
	return vec3(x, y, z);
}


// OTHER

inline int GetRandomInt(int Min, int Max)
{
	//@std
	static bool bInitialized = false;
	if (!bInitialized)
	{
		std::srand(std::time(nullptr)); //seeding for the first time only!
		bInitialized = true;
	}
	return Min + rand() % ((Max + 1) - Min);
}

inline float GetRandomFloat(int Min, int Max)
{
	return GetRandomInt(Min * 1000, Max * 1000) / 1000.f;
}

// colors
inline vec3 GetRandomColor()
{
	return vec3(
		GetRandomFloat(0, 1),
		GetRandomFloat(0, 1),
		GetRandomFloat(0, 1)
	);
}


// text
void Tolower(string* Data);

inline float Min(float a, float b)
{
	return a <= b ? a : b;
}

inline float Min(float a, float b, float c)
{
	return Min(a, Min(b, c));
}

inline float Max(float a, float b)
{
	return a >= b ? a : b;
}

inline float Max(float a, float b, float c)
{
	return Max(a, Max(b, c));
}
