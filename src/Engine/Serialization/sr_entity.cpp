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

void EntitySerializer::Parse(Parser& Parser)
{
	auto* World = RWorld::Get();
	auto* NewEntityPtr = World->SpawnEntity<EStaticMesh>();
	if (!NewEntityPtr)
		fatal_error("Couldnt create entity.")

	auto& NewEntity = *NewEntityPtr;

	auto& P = Parser;
	P.ParseName();
	new_entity.name = GetParsed<std::string>(parser);

	while (Parser.NextLine())
	{
		P.ParseToken();
		const auto Property = GetParsed<std::string>(parser);

		if (property == "id")
		{
			P.ParseAllWhitespace();
			P.ParseU64();
			uint64 Id = GetParsed<uint64>(Parser);
			NewEntity.ID = Id;
		}

		else if (property == "position")
		{
			P.ParseVec3();
			new_entity.position = GetParsed<glm::vec3>(parser);
		}

		else if (property == "rotation")
		{
			P.ParseVec3();
			new_entity.rotation = GetParsed<glm::vec3>(parser);
		}

		else if (property == "scale")
		{
			P.ParseVec3();
			const auto S = GetParsed<glm::vec3>(parser);

			if (s.x<0 || s.y<0 || s.z < 0)
			                 fatal_error("FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '%s' LINE NUMBER %i", Parser.Filepath.c_str(), Parser.LineCount);

				NewEntity.Scale = s;
		}

		else if (property == "shader")
		{
			P.ParseAllWhitespace();
			P.ParseToken();
			const auto ShaderName = GetParsed<std::string>(parser);

			auto Find = ShaderCatalogue.find(shader_name);
			if (find != ShaderCatalogue.end())
			{
				if (shader_name == "tiledTextureModel")
				{
					NewEntity.Flags |= EntityFlags_RenderTiledTexture;
					For(6)
					{
						P.ParseAllWhitespace();
						P.ParseInt();
						if (!P.HasToken())
							fatal_error("Scene description contain an entity with box tiled shader without full tile quantity description.");

						NewEntity.UvTileWrap[i] = GetParsed<int>(Parser);
					}
				}
				NewEntity.Shader = find->second;
			}
			else
				fatal_error("SHADER '%s' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE.", shader_name.c_str());
		}

		else if (property == "mesh")
		{
			P.ParseAllWhitespace();
			P.ParseToken();
			const auto ModelName = GetParsed<std::string>(parser);

			auto FindMesh = GeometryCatalogue.find(model_name);
			if (find_mesh != GeometryCatalogue.end())
				NewEntity.Mesh = find_mesh->second;
			else
				new_entity.mesh = LoadWavefrontObjAsMesh(Paths::Models, model_name);

			// @TODO: For now collision mesh is loaded from the same model as regular mesh.
			auto FindCMesh = CollisionGeometryCatalogue.find(model_name);
			if (find_c_mesh != CollisionGeometryCatalogue.end())
				NewEntity.CollisionMesh = find_c_mesh->second;
			else
				new_entity.collision_mesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, model_name);

			NewEntity.Collider = *NewEntity.CollisionMesh;
		}

		else if (property == "texture")
		{
			P.ParseAllWhitespace();
			P.ParseToken();
			const auto TextureName = GetParsed<std::string>(parser);

			// > texture definition error handling
			// >> check for missing info
			if (texture_name.empty())
				fatal_error("Fatal: Texture for entity '%s' is missing name.", NewEntity.Name.c_str())

			// fetches texture in catalogue
			auto Texture = TextureCatalogue.find(texture_name);
			if (texture == TextureCatalogue.end())
				fatal_error("Fatal: %s was not found (not pre-loaded) inside Texture Catalogue.", texture_name.c_str())

			NewEntity.Textures.clear();
			NewEntity.Textures.push_back(texture->second);

			// fetches texture normal in catalogue, if any
			auto Normal = TextureCatalogue.find(texture_name + "_normal");
			if (normal != TextureCatalogue.end())
			{
				NewEntity.Textures.push_back(normal->second);
			}
		}

		else if (property == "hidden")
		{
			NewEntity.Flags |= EntityFlags_HiddenEntity;
		}

		else if (property == "trigger")
		{
			P.ParseVec3();
			new_entity.trigger_scale = GetParsed<glm::vec3>(parser);
		}

		else if (property == "slidable")
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
	writer << "\n#" << Entity.Name << "\n";
	writer << "id " << Entity.ID << "\n";
	writer << "position "
	<< Entity.Position.x << " "
	<< Entity.Position.y << " "
	<< Entity.Position.z << "\n";
	writer << "rotation "
	<< Entity.Rotation.x << " "
	<< Entity.Rotation.y << " "
	<< Entity.Rotation.z << "\n";
	writer << "scale "
	<< Entity.Scale.x << " "
	<< Entity.Scale.y << " "
	<< Entity.Scale.z << "\n";
	writer << "mesh " << Entity.Mesh->Name << "\n";
	writer << "shader " << Entity.Shader->Name;

	// shader: If entity.s using tiled texture fragment shader, also writes number of tiles since we can change it through the editor
	if (Entity.Flags & EntityFlags_RenderTiledTexture)
	{
		For(6)
		{
			writer << " " << Entity.UvTileWrap[i];
		}
	}

	writer << "\n";

	int Textures = Entity.Textures.size();
	For(Textures)
	{
		RTexture Texture = Entity.Textures[i];
		if (Texture.Type == "texture_diffuse")
			writer << "texture " << Texture.Name << "\n";
	}

	if (Entity.Flags & EntityFlags_RenderWireframe)
		writer << "hidden\n";

	/*
	switch (entity.type)
	{
		case EntityType_Static:
		{
			writer << "type static\n";
			break;
		}

		case EntityType_Checkpoint:
		{
			writer << "type checkpoint\n";
			writer << "trigger "
			<< entity.trigger_scale.x << " "
			<< entity.trigger_scale.y << " "
			<< entity.trigger_scale.z << "\n";
			break;
		}

		case EntityType_TimerTrigger:
		{
			writer << "type timer_trigger\n";
			writer << "trigger "
			<< entity.trigger_scale.x << " "
			<< entity.trigger_scale.y << " "
			<< entity.trigger_scale.z << "\n";
			if (entity.timer_trigger_data.timer_target != nullptr)
				writer << "timer_target " << entity.timer_trigger_data.timer_target->id << "\n";
			writer << "timer_duration " << entity.timer_trigger_data.timer_duration << "\n";

			For(entity.timer_trigger_data.size)
			{
				const auto marking = entity.timer_trigger_data.markings[i];
				const u32 time_checkpoint = entity.timer_trigger_data.time_checkpoints[i];
				if (marking != nullptr)
					writer << "timer_marking " << marking->id << " " << time_checkpoint << "\n";
			}

			break;
		}

		case EntityType_TimerTarget:
		{
			writer << "type timer_target\n";
			writer << "timer_target_type " << entity.timer_target_data.timer_target_type << "\n";

			if (entity.timer_target_data.timer_start_animation != 0)
				writer << "timer_start_animation " << entity.timer_target_data.timer_start_animation << "\n";

			if (entity.timer_target_data.timer_stop_animation != 0)
				writer << "timer_stop_animation " << entity.timer_target_data.timer_stop_animation << "\n";

			break;
		}

		case EntityType_TimerMarking:
		{
			writer << "type timer_marking\n";
			writer << "timer_marking_color_on "
			<< entity.timer_marking_data.color_on.x << " "
			<< entity.timer_marking_data.color_on.y << " "
			<< entity.timer_marking_data.color_on.z << "\n";

			writer << "timer_marking_color_off "
			<< entity.timer_marking_data.color_off.x << " "
			<< entity.timer_marking_data.color_off.y << " "
			<< entity.timer_marking_data.color_off.z << "\n";

			break;
		}
	}
	*/

	if (Entity.Slidable)
	{
		writer << "slidable \n";
	}
}

void EntitySerializer::ClearBuffer()
{
	relations = DeferredEntityRelationBuffer{};
}
