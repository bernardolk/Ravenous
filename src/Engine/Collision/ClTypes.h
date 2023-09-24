// --------------
// > CL_Results
// --------------
#pragma once

struct Entity;

struct ClResults
{
	bool collision = false;
	EEntity* entity = nullptr;
	float penetration = 0.f;
	vec3 normal{};
};
