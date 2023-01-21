struct Triangle
{
	vec3 a;
	vec3 b;
	vec3 c;
};

inline vec3 get_triangle_normal(Triangle t)
{
	return triangleNormal(t.a, t.b, t.c);
}

inline vec3 get_barycenter(Triangle t)
{
	auto bx = (t.a.x + t.b.x + t.c.x) / 3;
	auto by = (t.a.y + t.b.y + t.c.y) / 3;
	auto bz = (t.a.z + t.b.z + t.c.z) / 3;
	return vec3(bx, by, bz);
}

inline bool is_equal(Triangle t1, Triangle t2)
{
	return t1.a == t2.a && t1.b == t2.b && t1.c == t2.c;
}

inline bool is_valid(Triangle t)
{
	// checks if vertices are not in a single point
	return t.a != t.b && t.a != t.c && t.b != t.c;
}
