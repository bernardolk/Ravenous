#include <engine/core/types.h>
#include "engine/rvn.h"
#include <map>
#include <engine/serialization/sr_entity.h>
#include <engine/collision/CollisionMesh.h>
#include <glm/gtx/quaternion.hpp>

#include "engine/entities/StaticMesh.h"
#include "engine/geometry/mesh.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/io/loaders.h"
#include "engine/render/Shader.h"
#include "engine/world/World.h"

const std::string SrLoadEntityTypeNotSetErrorMsg = "Need to load entity type before loading type-specific data.";

void EntitySerializer::Parse(Parser& Parse)
{
	auto* World = RWorld::Get();
	auto* NewEntityPtr = World->SpawnEntity<EStaticMesh>();
	if (!NewEntityPtr)
		fatal_error("Couldnt create entity.")

	auto& NewEntity = *NewEntityPtr;

	Parse.ParseName();
	NewEntity.Name = GetParsed<std::string>(Parse);

	while (Parse.NextLine())
	{
		Parse.ParseToken();
		const auto Property = GetParsed<std::string>(Parse);

		if (Property == "id")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseU64();
			uint64 Id = GetParsed<uint64>(Parse);
			NewEntity.ID = Id;
		}

		else if (Property == "position")
		{
			Parse.ParseVec3();
			NewEntity.Position = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "rotation")
		{
			Parse.ParseVec3();
			NewEntity.Rotation = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "scale")
		{
			Parse.ParseVec3();
			const auto Scale = GetParsed<glm::vec3>(Parse);

			if (Scale.x < 0 || Scale.y < 0 || Scale.z < 0)
			                 fatal_error("FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '%s' LINE NUMBER %i", Parse.Filepath.c_str(), Parse.LineCount);

				NewEntity.Scale = Scale;
		}

		else if (Property == "shader")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseToken();
			const auto ShaderName = GetParsed<std::string>(Parse);

			auto Find = ShaderCatalogue.find(ShaderName);
			if (Find != ShaderCatalogue.end())
			{
				if (ShaderName == "tiledTextureModel")
				{
					NewEntity.Flags |= EntityFlags_RenderTiledTexture;
					for (int i = 0; i < 6; i++)
					{
						Parse.ParseAllWhitespace();
						Parse.ParseInt();
						if (!Parse.HasToken())
							fatal_error("Scene description contain an entity with box tiled shader without full tile quantity description.");

						NewEntity.UvTileWrap[i] = GetParsed<int>(Parse);
					}
				}
				NewEntity.Shader = Find->second;
			}
			else
				fatal_error("SHADER '%s' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE.", ShaderName.c_str());
		}

		else if (Property == "mesh")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseToken();
			const auto ModelName = GetParsed<std::string>(Parse);

			auto FindMesh = GeometryCatalogue.find(ModelName);
			if (FindMesh != GeometryCatalogue.end())
				NewEntity.Mesh = FindMesh->second;
			else
				NewEntity.Mesh = LoadWavefrontObjAsMesh(Paths::Models, ModelName);

			// @TODO: For now collision mesh is loaded from the same model as regular mesh.
			auto FindCMesh = CollisionGeometryCatalogue.find(ModelName);
			if (FindCMesh != CollisionGeometryCatalogue.end())
				NewEntity.CollisionMesh = FindCMesh->second;
			else
				NewEntity.CollisionMesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, ModelName);

			NewEntity.Collider = *NewEntity.CollisionMesh;
		}

		else if (Property == "texture")
		{
			Parse.ParseAllWhitespace();
			Parse.ParseToken();
			const auto TextureName = GetParsed<std::string>(Parse);

			// > texture definition error handling
			// >> check for missing info
			if (TextureName.empty())
				fatal_error("Fatal: Texture for entity '%s' is missing name.", NewEntity.Name.c_str())

			// fetches texture in catalogue
			auto Texture = TextureCatalogue.find(TextureName);
			if (Texture == TextureCatalogue.end())
				fatal_error("Fatal: %s was not found (not pre-loaded) inside Texture Catalogue.", TextureName.c_str())

			NewEntity.Textures.clear();
			NewEntity.Textures.push_back(Texture->second);

			// fetches texture normal in catalogue, if any
			auto Normal = TextureCatalogue.find(TextureName + "_normal");
			if (Normal != TextureCatalogue.end())
			{
				NewEntity.Textures.push_back(Normal->second);
			}
		}

		else if (Property == "hidden")
		{
			NewEntity.Flags |= EntityFlags_HiddenEntity;
		}

		else if (Property == "trigger")
		{
			Parse.ParseVec3();
			NewEntity.TriggerScale = GetParsed<glm::vec3>(Parse);
		}

		else if (Property == "slidable")
		{
			NewEntity.Slidable = true;
		}

		else
		{
			break;
		}
	}
}

void EntitySerializer::Save(std::ofstream& Writer, const EEntity& Entity)
{
	Writer << "\n#" << Entity.Name << "\n";
	Writer << "id " << Entity.ID << "\n";
	Writer << "position "
	<< Entity.Position.x << " "
	<< Entity.Position.y << " "
	<< Entity.Position.z << "\n";
	Writer << "rotation "
	<< Entity.Rotation.x << " "
	<< Entity.Rotation.y << " "
	<< Entity.Rotation.z << "\n";
	Writer << "scale "
	<< Entity.Scale.x << " "
	<< Entity.Scale.y << " "
	<< Entity.Scale.z << "\n";
	Writer << "mesh " << Entity.Mesh->Name << "\n";
	Writer << "shader " << Entity.Shader->Name;

	// shader: If entity.s using tiled texture fragment shader, also writes number of tiles since we can change it through the editor
	if (Entity.Flags & EntityFlags_RenderTiledTexture)
	{
		for (int i = 0; i < 6; i++) {
			Writer << " " << Entity.UvTileWrap[i];
		}
	}

	Writer << "\n";

	for (auto& Texture : Entity.Textures)
	{
		if (Texture.Type == "texture_diffuse") {
			Writer << "texture " << Texture.Name << "\n";
		}
	}

	if (Entity.Flags & EntityFlags_RenderWireframe) {
		Writer << "hidden\n";
	}

	/*
	switch (entity.type)
	{
		case EntityType_Static:
		{
			Writer << "type static\n";
			break;
		}

		case EntityType_Checkpoint:
		{
			Writer << "type checkpoint\n";
			Writer << "trigger "
			<< entity.TriggerScale.x << " "
			<< entity.TriggerScale.y << " "
			<< entity.TriggerScale.z << "\n";
			break;
		}

		case EntityType_TimerTrigger:
		{
			Writer << "type timer_trigger\n";
			Writer << "trigger "
			<< entity.TriggerScale.x << " "
			<< entity.TriggerScale.y << " "
			<< entity.TriggerScale.z << "\n";
			if (entity.timer_trigger_data.timer_target != nullptr)
				Writer << "timer_target " << entity.timer_trigger_data.timer_target->id << "\n";
			Writer << "timer_duration " << entity.timer_trigger_data.timer_duration << "\n";

			For(entity.timer_trigger_data.size)
			{
				const auto marking = entity.timer_trigger_data.markings[i];
				const u32 time_checkpoint = entity.timer_trigger_data.time_checkpoints[i];
				if (marking != nullptr)
					Writer << "timer_marking " << marking->id << " " << time_checkpoint << "\n";
			}

			break;
		}

		case EntityType_TimerTarget:
		{
			Writer << "type timer_target\n";
			Writer << "timer_target_type " << entity.timer_target_data.timer_target_type << "\n";

			if (entity.timer_target_data.timer_start_animation != 0)
				Writer << "timer_start_animation " << entity.timer_target_data.timer_start_animation << "\n";

			if (entity.timer_target_data.timer_stop_animation != 0)
				Writer << "timer_stop_animation " << entity.timer_target_data.timer_stop_animation << "\n";

			break;
		}

		case EntityType_TimerMarking:
		{
			Writer << "type timer_marking\n";
			Writer << "timer_marking_color_on "
			<< entity.timer_marking_data.color_on.x << " "
			<< entity.timer_marking_data.color_on.y << " "
			<< entity.timer_marking_data.color_on.z << "\n";

			Writer << "timer_marking_color_off "
			<< entity.timer_marking_data.color_off.x << " "
			<< entity.timer_marking_data.color_off.y << " "
			<< entity.timer_marking_data.color_off.z << "\n";

			break;
		}
	}
	*/

	if (Entity.Slidable) {
		Writer << "slidable \n";
	}
}

void EntitySerializer::ClearBuffer()
{
	Relations = DeferredEntityRelationBuffer{};
}
