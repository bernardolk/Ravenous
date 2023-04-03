#pragma once
#include "engine/entities/e_entity.h"
#include "engine/entities/traits/entity_traits.h"

struct EntityDecl(SpotLight)
{
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float innercone = 1;
	float outercone = 0.5;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct EntityDecl(PointLight)
{
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float intensity_constant = 0.5f;
	float intensity_linear = 0.4f;
	float intensity_quadratic = 0.032f;
};

struct EntityDecl(DirectionalLight)
{
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
};
