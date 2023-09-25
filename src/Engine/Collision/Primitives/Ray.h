#pragma once

struct RRay
{
	vec3 Origin;
	vec3 Direction;
	vec3 Inverse;

	RRay() = default;
	RRay(vec3 Origin, vec3 Direction) : Origin(Origin), Direction(Direction), Inverse(){}
	
	vec3 GetInv()
	{
		if (!InverseIsSet)
		{
			Inverse = vec3(1.0 / Direction.x, 1.0 / Direction.y, 1.0 / Direction.z);
			InverseIsSet = true;
		}
		return Inverse;
	}

private:
	bool InverseIsSet = false;
};
