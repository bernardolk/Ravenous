#pragma once
#include "engine/entities/EEntity.h"
#include "engine/entities/traits/EntityTraits.h"

struct SpotLight : EEntity, T_EntityTypeBase<SpotLight>
{
	//Reflected()
	
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float innercone = 1;
	float outercone = 0.5;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct Entity(PointLight)
{
	Reflected()
	
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
	float intensity_constant = 0.5f;
	float intensity_linear = 0.4f;
	float intensity_quadratic = 0.032f;
};

struct Entity(DirectionalLight)
{
	Reflected()
	
	vec3 direction = vec3(0, -1, 0);
	vec3 diffuse = vec3(1);
	vec3 specular = vec3(1);
};
