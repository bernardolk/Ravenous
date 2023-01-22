// --------------
// > CL_Results
// --------------
#pragma once

struct Entity;

struct ClResults
{
	bool collision = false;
	Entity* entity = nullptr;
	float penetration = 0.f;
	vec3 normal{};
};

struct ClResultsArray
{
	ClResults results[10];
	int count = 0;
};
