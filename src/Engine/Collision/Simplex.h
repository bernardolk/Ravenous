#pragma once

struct RSimplex
{
	vec3 Points[4];
	uint PSize;

	RSimplex()
	{
		Points[0] = vec3(0);
		Points[1] = vec3(0);
		Points[2] = vec3(0);
		Points[3] = vec3(0);
		PSize = 0;
	}

	RSimplex(vec3 A)
	{
		Points[0] = A;
		PSize = 1;
	}

	RSimplex(vec3 A, vec3 B)
	{
		Points[0] = A;
		Points[1] = B;
		PSize = 2;
	}

	RSimplex(vec3 A, vec3 B, vec3 C)
	{
		Points[0] = A;
		Points[1] = B;
		Points[2] = C;
		PSize = 3;
	}

	RSimplex(vec3 A, vec3 B, vec3 C, vec3 D)
	{
		Points[0] = A;
		Points[1] = B;
		Points[2] = C;
		Points[3] = D;
		PSize = 4;
	}

	void PushFront(vec3 Point);
	vec3& operator[](uint i);
	uint size() const;
};
