#pragma once

struct RRay
{
	vec3 Origin = vec3{0.f};
	vec3 Direction = vec3{0.f};

	RRay() = default;
	RRay(vec3 Origin, vec3 Direction) : Origin(Origin), Direction(Direction) {}
	vec3 GetInverse() const { return vec3(1.0 / Direction.x, 1.0 / Direction.y, 1.0 / Direction.z); }
private:
	bool InverseIsSet = false;
};
