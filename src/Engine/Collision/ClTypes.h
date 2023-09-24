// --------------
// > CL_Results
// --------------
#pragma once

struct RCollisionResults
{
	bool collision = false;
	struct EEntity* entity = nullptr;
	float penetration = 0.f;
	vec3 normal;
};
