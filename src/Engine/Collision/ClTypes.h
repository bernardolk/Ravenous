// --------------
// > CL_Results
// --------------
#pragma once

struct RCollisionResults
{
	bool Collision = false;
	struct EEntity* Entity = nullptr;
	float Penetration = 0.f;
	vec3 Normal;
};
