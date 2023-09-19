#include "engine/serialization/sr_light.h"
#include "engine/entities/lights.h"
#include "engine/world/world.h"
#include "engine/serialization/parsing/parser.h"


void LightSerializer::Parse(Parser& p)
{
	p.ParseToken();
	const auto type = GetParsed<std::string>(p);

	if (type == "point")
		ParsePointLight(p);

	else if (type == "spot")
		ParseSpotLight(p);

	else if (type == "directional")
		ParseDirectionalLight(p);

	else
		fatal_error("FATAL: Unrecognized light source in scene file '%s', line %i.", p.filepath.c_str(), p.line_count);
}

void LightSerializer::ParsePointLight(Parser& p)
{
	//@TODO: Deal with this with a memory pool (?)
	PointLight* new_light = new PointLight;
	auto& point_light = *new_light;

	while (p.NextLine())
	{
		p.ParseToken();
		const std::string property = GetParsed<std::string>(p);

		if (property == "position")
		{
			p.ParseVec3();
			point_light.position = GetParsed<glm::vec3>(p);
		}

		else if (property == "diffuse")
		{
			p.ParseVec3();
			point_light.diffuse = GetParsed<glm::vec3>(p);
		}

		else if (property == "specular")
		{
			p.ParseVec3();
			point_light.specular = GetParsed<glm::vec3>(p);
		}

		else if (property == "constant")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			point_light.intensity_constant = GetParsed<float>(p);
		}

		else if (property == "linear")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			point_light.intensity_linear = GetParsed<float>(p);
		}

		else if (property == "quadratic")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			point_light.intensity_quadratic = GetParsed<float>(p);
		}

		else
			break;
	}

	T_World::Get()->point_lights.push_back(&point_light);
}

void LightSerializer::ParseSpotLight(Parser& p)
{
	//@TODO: Deal with this with a memory pool (?)
	SpotLight& spotlight = *(new SpotLight());

	while (p.NextLine())
	{
		p.ParseToken();
		const auto property = GetParsed<std::string>(p);

		if (property == "position")
		{
			p.ParseVec3();
			spotlight.position = GetParsed<glm::vec3>(p);
		}
		else if (property == "direction")
		{
			p.ParseVec3();
			spotlight.direction = GetParsed<glm::vec3>(p);
		}
		else if (property == "diffuse")
		{
			p.ParseVec3();
			spotlight.diffuse = GetParsed<glm::vec3>(p);
		}
		else if (property == "specular")
		{
			p.ParseVec3();
			spotlight.specular = GetParsed<glm::vec3>(p);
		}
		else if (property == "constant")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			spotlight.intensity_constant = GetParsed<float>(p);
		}
		else if (property == "linear")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			spotlight.intensity_linear = GetParsed<float>(p);
		}
		else if (property == "quadratic")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			spotlight.intensity_quadratic = GetParsed<float>(p);
		}
		else if (property == "innercone")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			spotlight.innercone = GetParsed<float>(p);
		}
		else if (property == "outercone")
		{
			p.ParseAllWhitespace();
			p.ParseFloat();
			spotlight.outercone = GetParsed<float>(p);
		}
		else
			break;
	}

	T_World::Get()->spot_lights.push_back(&spotlight);
}

void LightSerializer::ParseDirectionalLight(Parser& p)
{
	//@TODO: Deal with this with a memory pool (?)
	DirectionalLight& light = *(new DirectionalLight());

	while (p.NextLine())
	{
		p.ParseToken();
		const auto property = GetParsed<std::string>(p);

		if (property == "direction")
		{
			p.ParseVec3();
			light.direction = GetParsed<glm::vec3>(p);
		}
		else if (property == "diffuse")
		{
			p.ParseVec3();
			light.diffuse = GetParsed<glm::vec3>(p);
		}
		else if (property == "specular")
		{
			p.ParseVec3();
			light.specular = GetParsed<glm::vec3>(p);
		}
		else
			break;
	}

	T_World::Get()->directional_lights.push_back(&light);
}

void LightSerializer::Save(std::ofstream& writer, const PointLight* light)
{
	writer << "\n$point\n"
	<< "position "
	<< light->position.x << " "
	<< light->position.y << " "
	<< light->position.z << "\n"
	<< "diffuse "
	<< light->diffuse.x << " "
	<< light->diffuse.y << " "
	<< light->diffuse.z << "\n"
	<< "specular "
	<< light->specular.x << " "
	<< light->specular.y << " "
	<< light->specular.z << "\n"
	<< "constant "
	<< light->intensity_constant << "\n"
	<< "linear "
	<< light->intensity_linear << "\n"
	<< "quadratic "
	<< light->intensity_quadratic << "\n";
}

void LightSerializer::Save(std::ofstream& writer, const SpotLight* light)
{
	writer << "\n$spot\n"
	<< "position "
	<< light->position.x << " "
	<< light->position.y << " "
	<< light->position.z << "\n"
	<< "direction "
	<< light->direction.x << " "
	<< light->direction.y << " "
	<< light->direction.z << "\n"
	<< "diffuse "
	<< light->diffuse.x << " "
	<< light->diffuse.y << " "
	<< light->diffuse.z << "\n"
	<< "specular "
	<< light->specular.x << " "
	<< light->specular.y << " "
	<< light->specular.z << "\n"
	<< "innercone "
	<< light->innercone << "\n"
	<< "outercone "
	<< light->outercone << "\n"
	<< "constant "
	<< light->intensity_constant << "\n"
	<< "linear "
	<< light->intensity_linear << "\n"
	<< "quadratic "
	<< light->intensity_quadratic << "\n";
}


void LightSerializer::Save(std::ofstream& writer, const DirectionalLight* light)
{
	writer << "\n$directional\n"
	<< "direction "
	<< light->direction.x << " "
	<< light->direction.y << " "
	<< light->direction.z << "\n"
	<< "diffuse "
	<< light->diffuse.x << " "
	<< light->diffuse.y << " "
	<< light->diffuse.z << "\n"
	<< "specular "
	<< light->specular.x << " "
	<< light->specular.y << " "
	<< light->specular.z << "\n";
}
