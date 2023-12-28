#include "engine/serialization/sr_Light.h"
#include "engine/entities/Lights.h"
#include "engine/world/World.h"
#include "engine/serialization/parsing/parser.h"


void LightSerializer::Parse(Parser& Parse)
{
	Parse.ParseToken();
	const auto Type = GetParsed<string>(Parse);

	if (Type == "point")
		ParsePointLight(Parse);

	else if (Type == "spot")
		ParseSpotLight(Parse);

	else if (Type == "directional")
		ParseDirectionalLight(Parse);

	else
		FatalError("FATAL: Unrecognized Light source in scene file '%s', line %i.", Parse.Filepath.c_str(), Parse.LineCount);
}

void LightSerializer::ParsePointLight(Parser& Parse)
{
	//@TODO: Deal with this with a memory pool (?)
	auto NewLight = new EPointLight;
	auto& PointLight = *NewLight;

	while (Parse.NextLine())
	{
		Parse.ParseToken();
		const string Property = GetParsed<string>(Parse);

		if (Property == "position")
		{
			Parse.ParseVec3();
			PointLight.Position = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "diffuse")
		{
			Parse.ParseVec3();
			PointLight.Diffuse = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "specular")
		{
			Parse.ParseVec3();
			PointLight.Specular = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "constant")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			PointLight.IntensityConstant = GetParsed<float>(Parse);
		}

		else if (Property == "linear")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			PointLight.IntensityLinear = GetParsed<float>(Parse);
		}

		else if (Property == "quadratic")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			PointLight.IntensityQuadratic = GetParsed<float>(Parse);
		}

		else
			break;
	}

	RWorld::Get()->PointLights.push_back(&PointLight);
}

void LightSerializer::ParseSpotLight(Parser& Parse)
{
	//@TODO: Deal with this with a memory pool (?)
	ESpotLight& SpotLight = *(new ESpotLight);

	while (Parse.NextLine())
	{
		Parse.ParseToken();
		const auto Property = GetParsed<string>(Parse);

		if (Property == "position")
		{
			Parse.ParseVec3();
			SpotLight.Position = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "direction")
		{
			Parse.ParseVec3();
			SpotLight.Direction = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "diffuse")
		{
			Parse.ParseVec3();
			SpotLight.Diffuse = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "specular")
		{
			Parse.ParseVec3();
			SpotLight.Specular = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "constant")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			SpotLight.IntensityConstant = GetParsed<float>(Parse);
		}
		else if (Property == "linear")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			SpotLight.IntensityLinear = GetParsed<float>(Parse);
		}
		else if (Property == "quadratic")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			SpotLight.IntensityQuadratic = GetParsed<float>(Parse);
		}
		else if (Property == "innercone")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			SpotLight.Innercone = GetParsed<float>(Parse);
		}
		else if (Property == "outercone")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseFloat();
			SpotLight.Outercone = GetParsed<float>(Parse);
		}
		else
			break;
	}

	RWorld::Get()->SpotLights.push_back(&SpotLight);
}

void LightSerializer::ParseDirectionalLight(Parser& Parse)
{
	//@TODO: Deal with this with a memory pool (?)
	EDirectionalLight& Light = *(new EDirectionalLight());

	while (Parse.NextLine())
	{
		Parse.ParseToken();
		const auto Property = GetParsed<string>(Parse);

		if (Property == "direction")
		{
			Parse.ParseVec3();
			Light.Direction = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "diffuse")
		{
			Parse.ParseVec3();
			Light.Diffuse = GetParsed<glm::vec3>(Parse);
		}
		else if (Property == "specular")
		{
			Parse.ParseVec3();
			Light.Specular = GetParsed<glm::vec3>(Parse);
		}
		else
			break;
	}

	RWorld::Get()->DirectionalLights.push_back(&Light);
}

void LightSerializer::Save(std::ofstream& Writer, const EPointLight* Light)
{
	Writer << "\n$point\n"
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

void LightSerializer::Save(std::ofstream& Writer, const ESpotLight* Light)
{
	Writer << "\n$spot\n"
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


void LightSerializer::Save(std::ofstream& Writer, const EDirectionalLight* Light)
{
	Writer << "\n$directional\n"
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
