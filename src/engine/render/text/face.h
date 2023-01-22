#pragma once

struct Triangle;

struct Face
{
	Triangle a;
	Triangle b;
	vec3 center;
};


// -----------------------------
// > Triangle / Face operations
// -----------------------------
inline Face face_from_axis_aligned_triangle(Triangle t)
{
	// computes center
	float x0 = std::min({t.a.x, t.b.x, t.c.x});
	float x1 = std::max({t.a.x, t.b.x, t.c.x});
	float y0 = std::min({t.a.y, t.b.y, t.c.y});
	float y1 = std::max({t.a.y, t.b.y, t.c.y});
	float z0 = std::min({t.a.z, t.b.z, t.c.z});
	float z1 = std::max({t.a.z, t.b.z, t.c.z});

	float mx, my, mz;
	mx = x0 == x1 ? x0 : ((x1 - x0) / 2.0f) + x0;
	my = y0 == y1 ? y0 : ((y1 - y0) / 2.0f) + y0;
	mz = z0 == z1 ? z0 : ((z1 - z0) / 2.0f) + z0;
	auto center = vec3{mx, my, mz};

	vec3 normal = triangleNormal(t.a, t.b, t.c);

	vec3 a2 = rotate(t.a, glm::radians(180.0f), normal);
	vec3 b2 = rotate(t.b, glm::radians(180.0f), normal);
	vec3 c2 = rotate(t.c, glm::radians(180.0f), normal);

	vec3 translation;
	if(x0 == x1)
		translation = vec3(0, center.y, center.z);
	if(y0 == y1)
		translation = vec3(center.x, 0, center.z);
	if(z0 == z1)
		translation = vec3(center.x, center.y, 0);

	a2 += translation * 2.0f;
	b2 += translation * 2.0f;
	c2 += translation * 2.0f;

	auto t2 = Triangle{a2, b2, c2};

	Face f;
	f.a = t;
	f.b = t2;
	f.center = center;

	return f;
}
