#pragma once

struct RBoundingBox
{
	float minx;
	float maxx;
	float minz;
	float maxz;
	float miny;
	float maxy;

	auto Bounds()
	{
		struct
		{
			vec3 min, max;
		} bounds;

		bounds.min = vec3(minx, miny, minz);
		bounds.max = vec3(maxx, maxy, maxz);
		return bounds;
	}

	void Set(vec3 min, vec3 max)
	{
		minx = min.x;
		maxx = max.x;
		miny = min.y;
		maxy = max.y;
		minz = min.z;
		maxz = max.z;
	}

	auto GetPosAndScale()
	{
		struct
		{
			vec3 pos;
			vec3 scale;
		} result;

		result.pos = vec3(minx, miny, minz);
		result.scale = vec3(maxx - minx, maxy - miny, maxz - minz);

		return result;
	}

	vec3 GetCentroid()
	{
		return {
		(maxx + minx) / 2,
		(maxy + miny) / 2,
		(maxz + minz) / 2,
		};
	}

	/** Performs a collision test between this and another BoundingBox.*/
	bool Test(RBoundingBox other)
	{
		// Exit with no intersection if separated along an axis
		if (this->maxx < other.minx || this->minx > other.maxx)
			return false;
		if (this->maxy < other.miny || this->miny > other.maxy)
			return false;
		if (this->maxz < other.minz || this->minz > other.maxz)
			return false;
		// Overlapping on all axes means AABBs are intersecting
		return true;
	}

	void Translate(mat4 trans_mat)
	{
		vec4 trans_min = vec4(minx, miny, minz, 1) * trans_mat;
		vec4 trans_max = vec4(maxx, maxy, maxz, 1) * trans_mat;

		minx = trans_min.x;
		miny = trans_min.y;
		minz = trans_min.z;
		maxx = trans_max.x;
		maxy = trans_max.y;
		maxz = trans_max.z;
	}

	void Translate(vec3 offset)
	{
		minx += offset.x;
		miny += offset.y;
		minz += offset.z;
		maxx += offset.x;
		maxy += offset.y;
		maxz += offset.z;
	}
};
