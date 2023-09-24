#pragma once

struct RTriangle
{
	vec3 a;
	vec3 b;
	vec3 c;

	vec3 GetNormal() { return triangleNormal(a, b, c); }

	vec3 GetBarycenter()
	{
		auto bx = (a.x + b.x + c.x) / 3;
		auto by = (a.y + b.y + c.y) / 3;
		auto bz = (a.z + b.z + c.z) / 3;
		return vec3(bx, by, bz);
	}

	bool IsValid()
	{
		// checks if vertices are not in a single point
		return a != b && a != c && b != c;
	}

	bool operator ==(const RTriangle& other)
	{
		return a == other.a && b == other.b && c == other.c;
	}
};
