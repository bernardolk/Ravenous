#include "engine/serialization/sr_light.h"
#include "engine/entities/lights.h"
#include "engine/world/World.h"
#include "engine/serialization/parsing/parser.h"


void LightSerializer::Parse(Parser& P)
{
	P.ParseToken();
	const auto Type = GetParsed<std::string>(p);

	if (type == "point")
		ParsePointLight(P);

	else if (type == "spot")
		ParseSpotLight(P);

	else if (type == "directional")
		ParseDirectionalLight(P);

	else
		fatal_error("FATAL: Unrecognized light source in scene file '%s', line %i.", P.Filepath.c_str(), P.LineCount);
}

void LightSerializer::ParsePointLight(Parser& P)
{
	//@TODO: Deal with this with a memory pool (?)
	auto NewLight = new PointLight;
	auto& PointLight = *NewLight;

	while (P.NextLine())
	{
		P.ParseToken();
		const std::string Property = GetParsed<std::string>(p);

		if (property == "position")
		{
			P.ParseVec3();
			point_light.position = GetParsed<glm::vec3>(p);
		}

		else if (property == "diffuse")
		{
			P.ParseVec3();
			point_light.diffuse = GetParsed<glm::vec3>(p);
		}

		else if (property == "specular")
		{
			P.ParseVec3();
			point_light.specular = GetParsed<glm::vec3>(p);
		}

		else if (property == "constant")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			PointLight.IntensityConstant = GetParsed<float>(P);
		}

		else if (property == "linear")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			PointLight.IntensityLinear = GetParsed<float>(P);
		}

		else if (property == "quadratic")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			PointLight.IntensityQuadratic = GetParsed<float>(P);
		}

		else
			break;
	}

	RWorld::Get()->PointLights.push_back(&PointLight);
}

void LightSerializer::ParseSpotLight(Parser& P)
{
	//@TODO: Deal with this with a memory pool (?)
	SpotLight& Spotlight = *(new SpotLight());

	while (P.NextLine())
	{
		P.ParseToken();
		const auto Property = GetParsed<std::string>(p);

		if (property == "position")
		{
			P.ParseVec3();
			spotlight.position = GetParsed<glm::vec3>(p);
		}
		else if (property == "direction")
		{
			P.ParseVec3();
			spotlight.direction = GetParsed<glm::vec3>(p);
		}
		else if (property == "diffuse")
		{
			P.ParseVec3();
			spotlight.diffuse = GetParsed<glm::vec3>(p);
		}
		else if (property == "specular")
		{
			P.ParseVec3();
			spotlight.specular = GetParsed<glm::vec3>(p);
		}
		else if (property == "constant")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Spotlight.IntensityConstant = GetParsed<float>(P);
		}
		else if (property == "linear")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Spotlight.IntensityLinear = GetParsed<float>(P);
		}
		else if (property == "quadratic")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Spotlight.IntensityQuadratic = GetParsed<float>(P);
		}
		else if (property == "innercone")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Spotlight.Innercone = GetParsed<float>(P);
		}
		else if (property == "outercone")
		{
			P.ParseAllWhitespace();
			P.ParseFloat();
			Spotlight.Outercone = GetParsed<float>(P);
		}
		else
			break;
	}

	RWorld::Get()->SpotLights.push_back(&Spotlight);
}

void LightSerializer::ParseDirectionalLight(Parser& P)
{
	//@TODO: Deal with this with a memory pool (?)
	DirectionalLight& Light = *(new DirectionalLight());

	while (P.NextLine())
	{
		P.ParseToken();
		const auto Property = GetParsed<std::string>(p);

		if (property == "direction")
		{
			P.ParseVec3();
			light.direction = GetParsed<glm::vec3>(p);
		}
		else if (property == "diffuse")
		{
			P.ParseVec3();
			light.diffuse = GetParsed<glm::vec3>(p);
		}
		else if (property == "specular")
		{
			P.ParseVec3();
			light.specular = GetParsed<glm::vec3>(p);
		}
		else
			break;
	}

	RWorld::Get()->DirectionalLights.push_back(&Light);
}

void LightSerializer::Save(std::ofstream& Writer, const PointLight* Light)
{
	writer << "\n$point\n"
	<< "position "
	<< Light->Position.x << " "
	<< Light->Position.y << " "
	<< Light->Position.z << "\n"
	<< "diffuse "
	<< Light->Diffuse.x << " "
	<< Light->Diffuse.y << " "
	<< Light->Diffuse.z << "\n"
	<< "specular "
	<< Light->Specular.x << " "
	<< Light->Specular.y << " "
	<< Light->Specular.z << "\n"
	<< "constant "
	<< Light->IntensityConstant << "\n"
	<< "linear "
	<< Light->IntensityLinear << "\n"
	<< "quadratic "
	<< Light->IntensityQuadratic << "\n";
}

void LightSerializer::Save(std::ofstream& Writer, const SpotLight* Light)
{
	writer << "\n$spot\n"
	<< "position "
	<< Light->Position.x << " "
	<< Light->Position.y << " "
	<< Light->Position.z << "\n"
	<< "direction "
	<< Light->Direction.x << " "
	<< Light->Direction.y << " "
	<< Light->Direction.z << "\n"
	<< "diffuse "
	<< Light->Diffuse.x << " "
	<< Light->Diffuse.y << " "
	<< Light->Diffuse.z << "\n"
	<< "specular "
	<< Light->Specular.x << " "
	<< Light->Specular.y << " "
	<< Light->Specular.z << "\n"
	<< "innercone "
	<< Light->Innercone << "\n"
	<< "outercone "
	<< Light->Outercone << "\n"
	<< "constant "
	<< Light->IntensityConstant << "\n"
	<< "linear "
	<< Light->IntensityLinear << "\n"
	<< "quadratic "
	<< Light->IntensityQuadratic << "\n";
}


void LightSerializer::Save(std::ofstream& Writer, const DirectionalLight* Light)
{
	writer << "\n$directional\n"
	<< "direction "
	<< Light->Direction.x << " "
	<< Light->Direction.y << " "
	<< Light->Direction.z << "\n"
	<< "diffuse "
	<< Light->Diffuse.x << " "
	<< Light->Diffuse.y << " "
	<< Light->Diffuse.z << "\n"
	<< "specular "
	<< Light->Specular.x << " "
	<< Light->Specular.y << " "
	<< Light->Specular.z << "\n";
}
