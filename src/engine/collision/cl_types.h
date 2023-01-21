// --------------
// > CL_Results
// --------------
struct Entity;

struct CL_Results
{
	bool collision = false;
	Entity* entity;
	float penetration;
	vec3 normal;
};

struct CL_ResultsArray
{
	CL_Results results[10];
	int count = 0;
};
