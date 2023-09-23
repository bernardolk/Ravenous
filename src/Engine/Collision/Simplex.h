#pragma once

struct Simplex
{
	vec3 points[4];
	u32 p_size;

	Simplex()
	{
		points[0] = vec3(0);
		points[1] = vec3(0);
		points[2] = vec3(0);
		points[3] = vec3(0);
		p_size = 0;
	}

	Simplex(vec3 a)
	{
		points[0] = a;
		p_size = 1;
	}

	Simplex(vec3 a, vec3 b)
	{
		points[0] = a;
		points[1] = b;
		p_size = 2;
	}

	Simplex(vec3 a, vec3 b, vec3 c)
	{
		points[0] = a;
		points[1] = b;
		points[2] = c;
		p_size = 3;
	}

	Simplex(vec3 a, vec3 b, vec3 c, vec3 d)
	{
		points[0] = a;
		points[1] = b;
		points[2] = c;
		points[3] = d;
		p_size = 4;
	}

	void PushFront(vec3 point);
	vec3& operator[](u32 i);
	u32 size() const;
};
