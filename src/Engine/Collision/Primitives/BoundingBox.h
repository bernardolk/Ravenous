#pragma once

struct RBoundingBox
{
	float MinX;
	float MaxX;
	float MinZ;
	float MaxZ;
	float MinY;
	float MaxY;

	auto Bounds()
	{
		struct
		{
			vec3 Min, Max;
		} Bounds;

		Bounds.Min = vec3(MinX, MinY, MinZ);
		Bounds.Max = vec3(MaxX, MaxY, MaxZ);
		return Bounds;
	}

	void Set(vec3 Min, vec3 Max)
	{
		MinX = Min.x;
		MaxX = Max.x;
		MinY = Min.y;
		MaxY = Max.y;
		MinZ = Min.z;
		MaxZ = Max.z;
	}

	auto GetPosAndScale()
	{
		struct
		{
			vec3 Pos;
			vec3 Scale;
		} Result;

		Result.Pos = vec3(MinX, MinY, MinZ);
		Result.Scale = vec3(MaxX - MinX, MaxY - MinY, MaxZ - MinZ);

		return Result;
	}

	vec3 GetCentroid()
	{
		return
		{
			(MaxX + MinX) / 2.f,
			(MaxY + MinY) / 2.f,
			(MaxZ + MinZ) / 2.f,
		};
	}

	/** Performs a collision test between this and another BoundingBox.*/
	bool Test(RBoundingBox Other)
	{
		// Exit with no intersection if separated along an axis
		if (this->MaxX < Other.MinX || this->MinX > Other.MaxX)
			return false;
		if (this->MaxY < Other.MinY || this->MinY > Other.MaxY)
			return false;
		if (this->MaxZ < Other.MinZ || this->MinZ > Other.MaxZ)
			return false;
		// Overlapping on all axes means AABBs are intersecting
		return true;
	}

	void Translate(mat4 TransMat)
	{
		vec4 TransMin = vec4(MinX, MinY, MinZ, 1) * TransMat;
		vec4 TransMax = vec4(MaxX, MaxY, MaxZ, 1) * TransMat;

		MinX = TransMin.x;
		MinY = TransMin.y;
		MinZ = TransMin.z;
		MaxX = TransMax.x;
		MaxY = TransMax.y;
		MaxZ = TransMax.z;
	}

	void Translate(vec3 Offset)
	{
		MinX += Offset.x;
		MinY += Offset.y;
		MinZ += Offset.z;
		MaxX += Offset.x;
		MaxY += Offset.y;
		MaxZ += Offset.z;
	}

	vector<vec3> GetVertexPositions()
	{
		// This vector respects the winding order / indices of the aabb .obj asset
		return
		{
			{MaxX, MaxY, MaxZ},
			{MaxX, MinY, MaxZ},
			{MinX, MaxY, MaxZ},
			{MinX, MinY, MaxZ},
			{MaxX, MaxY, MinZ},
			{MaxX, MinY, MinZ},
			{MinX, MaxY, MinZ},
			{MinX, MinY, MinZ},
		};
	}
};
