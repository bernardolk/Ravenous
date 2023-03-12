#pragma once

#include "engine/core/core.h"

#include <ctime>

inline void PrintVec(vec3 vec, const std::string& prefix)
{
	std::cout << prefix << ": (" << vec.x << ", " << vec.y << ", " << vec.z << ") \n";
}

inline std::string FormatFloatTostr(float num, int precision = 3)
{
	std::string temp = std::to_string(num);
	return temp.substr(0, temp.find(".") + precision);
}

inline std::string FmtTostr(float num, int precision)
{
	return FormatFloatTostr(num, precision);
}

inline std::string ToString(vec3 vec)
{
	return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
}

inline std::string ToString(vec2 vec)
{
	return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ")";
}


enum FloatTolerance
{
	FloatTolerance_3_001    = 0,
	FloatTolerance_4_0001   = 1,
	FloatTolerance_5_00001  = 2,
	FloatTolerance_6_000001 = 3,
};

inline bool AreEqualFloats(float x, float y, FloatTolerance tolerance = FloatTolerance_5_00001)
{
	/* checks if two floats are equal within a certain tolerance level */
	float tolerances[] = {0.001f, 0.0001f, 0.00001f, 0.000001f};
	return abs(x - y) < tolerances[tolerance];
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
	float dot = glm::dot(a, b);
	float len_a = length(a);
	float len_b = length(b);
	float theta = acos(dot / (len_a * len_b));
	return theta;
}

inline float VectorAngle(vec3 a, vec3 b)
{
	float dot = glm::dot(a, b);
	float len_a = length(a);
	float len_b = length(b);
	float theta = acos(dot / (len_a * len_b));
	return theta;
}

inline float VectorAngleSigned(vec2 a, vec2 b)
{
	return atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
}

inline float VectorCos(vec2 a, vec2 b)
{
	float dot = glm::dot(a, b);
	float len_a = length(a);
	float len_b = length(b);
	float cos = dot / (len_a * len_b);
	return cos;
}

// VECTOR DIMENSION CONVERSION
inline vec3 ToXz(vec3 vector)
{
	return vec3(vector.x, 0, vector.z);
}


inline vec3 Cross(vec3 a, vec3 b)
{
	return glm::cross(a, b);
}

inline vec3 ProjectVecIntoRef(vec3 vec, vec3 ref)
{
	auto proj = dot(vec, ref) / (Abs(ref) * Abs(ref)) * ref;
	return proj;
}

inline vec3 ProjectVecOntoPlane(vec3 v, vec3 n)
{
	auto v_proj_n = ProjectVecIntoRef(v, n);
	auto v_proj_plane = v - v_proj_n;
	return v_proj_plane;
}


// OTHER MATHS
inline vec3 RotMatToEulerAnglesXyz(mat4& m)
{
	/*
	   This is equivalent to calling glm::extractEulerAngleXYZ
	*/

	float sy = sqrt(m[0][0] * m[0][0] + m[1][0] * m[1][0]);

	bool singular = sy < 1e-6; // If

	float x, y, z;
	if (!singular)
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

inline int GetRandomInt(int min, int max)
{
	static bool first = true;
	if (first)
	{
		std::srand(std::time(nullptr)); //seeding for the first time only!
		first = false;
	}
	return min + rand() % ((max + 1) - min);
}

inline float GetRandomFloat(int min, int max)
{
	return GetRandomInt(min * 1000, max * 1000) / 1000.f;
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
// #TODO: remove this include at some point ... 
#include <cctype>

inline void Tolower(std::string* data)
{
	std::transform(data->begin(), data->end(), data->begin(), [](unsigned char c) { return std::tolower(c); });
}
