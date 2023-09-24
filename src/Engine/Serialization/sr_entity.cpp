#include <engine/core/types.h>
#include "engine/rvn.h"
#include <map>
#include <engine/serialization/sr_entity.h>
#include <engine/collision/CollisionMesh.h>
#include <glm/gtx/quaternion.hpp>

#include "engine/entities/EStaticMesh.h"
#include "engine/geometry/mesh.h"
#include "engine/serialization/parsing/parser.h"
#include "engine/io/loaders.h"
#include "engine/render/shader.h"
#include "engine/world/world.h"

const std::string SrLoadEntity_TypeNotSetErrorMsg = "Need to load entity type before loading type-specific data.";

void EntitySerializer::Parse(Parser& parser)
{
	auto* world = World::Get();
	auto* new_entity_ptr = world->SpawnEntity<EStaticMesh>();
	if (!new_entity_ptr)
		fatal_error("Couldnt create entity.")

	auto& new_entity = *new_entity_ptr;

	auto& p = parser;
	p.ParseName();
	new_entity.name = GetParsed<std::string>(parser);

	while (parser.NextLine())
	{
		p.ParseToken();
		const auto property = GetParsed<std::string>(parser);

		if (property == "id")
		{
			p.ParseAllWhitespace();
			p.ParseU64();
			u64 id = GetParsed<u64>(parser);
			new_entity.id = id;
		}

		else if (property == "position")
		{
			p.ParseVec3();
			new_entity.position = GetParsed<glm::vec3>(parser);
		}

		else if (property == "rotation")
		{
			p.ParseVec3();
			new_entity.rotation = GetParsed<glm::vec3>(parser);
		}

		else if (property == "scale")
		{
			p.ParseVec3();
			const auto s = GetParsed<glm::vec3>(parser);

			if (s.x < 0 || s.y < 0 || s.z < 0)
				fatal_error("FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '%s' LINE NUMBER %i", parser.filepath.c_str(), parser.line_count);

			new_entity.scale = s;
		}

		else if (property == "shader")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto shader_name = GetParsed<std::string>(parser);

			auto find = ShaderCatalogue.find(shader_name);
			if (find != ShaderCatalogue.end())
			{
				if (shader_name == "tiledTextureModel")
				{
					new_entity.flags |= EntityFlags_RenderTiledTexture;
					For(6)
					{
						p.ParseAllWhitespace();
						p.ParseInt();
						if (!p.HasToken())
							fatal_error("Scene description contain an entity with box tiled shader without full tile quantity description.");

						new_entity.uv_tile_wrap[i] = GetParsed<int>(parser);
					}
				}
				new_entity.shader = find->second;
			}
			else
				fatal_error("SHADER '%s' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE.", shader_name.c_str());
		}

		else if (property == "mesh")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto model_name = GetParsed<std::string>(parser);

			auto find_mesh = GeometryCatalogue.find(model_name);
			if (find_mesh != GeometryCatalogue.end())
				new_entity.mesh = find_mesh->second;
			else
				new_entity.mesh = LoadWavefrontObjAsMesh(Paths::Models, model_name);

			// @TODO: For now collision mesh is loaded from the same model as regular mesh.
			auto find_c_mesh = CollisionGeometryCatalogue.find(model_name);
			if (find_c_mesh != CollisionGeometryCatalogue.end())
				new_entity.collision_mesh = find_c_mesh->second;
			else
				new_entity.collision_mesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, model_name);

			new_entity.collider = *new_entity.collision_mesh;
		}

		else if (property == "texture")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto texture_name = GetParsed<std::string>(parser);

			// > texture definition error handling
			// >> check for missing info
			if (texture_name.empty())
				fatal_error("Fatal: Texture for entity '%s' is missing name.", new_entity.name.c_str())

			// fetches texture in catalogue
			auto texture = TextureCatalogue.find(texture_name);
			if (texture == TextureCatalogue.end())
				fatal_error("Fatal: %s was not found (not pre-loaded) inside Texture Catalogue.", texture_name.c_str())

			new_entity.textures.clear();
			new_entity.textures.push_back(texture->second);

			// fetches texture normal in catalogue, if any
			auto normal = TextureCatalogue.find(texture_name + "_normal");
			if (normal != TextureCatalogue.end())
			{
				new_entity.textures.push_back(normal->second);
			}
		}

		else if (property == "hidden")
		{
			new_entity.flags |= EntityFlags_HiddenEntity;
		}

		else if (property == "trigger")
		{
			p.ParseVec3();
			new_entity.trigger_scale = GetParsed<glm::vec3>(parser);
		}

		else if (property == "slidable")
		{
			new_entity.slidable = true;
		}

		else
		{
			break;
		}
	}
}

void EntitySerializer::Save(std::ofstream& writer, const EEntity& entity)
{
	writer << "\n#" << entity.name << "\n";
	writer << "id " << entity.id << "\n";
	writer << "position "
	<< entity.position.x << " "
	<< entity.position.y << " "
	<< entity.position.z << "\n";
	writer << "rotation "
	<< entity.rotation.x << " "
	<< entity.rotation.y << " "
	<< entity.rotation.z << "\n";
	writer << "scale "
	<< entity.scale.x << " "
	<< entity.scale.y << " "
	<< entity.scale.z << "\n";
	writer << "mesh " << entity.mesh->name << "\n";
	writer << "shader " << entity.shader->name;

	// shader: If entity.s using tiled texture fragment shader, also writes number of tiles since we can change it through the editor
	if (entity.flags & EntityFlags_RenderTiledTexture)
	{
		For(6)
		{
			writer << " " << entity.uv_tile_wrap[i];
		}
	}

	writer << "\n";

	int textures = entity.textures.size();
	For(textures)
	{
		Texture texture = entity.textures[i];
		if (texture.type == "texture_diffuse")
			writer << "texture " << texture.name << "\n";
	}

	if (entity.flags & EntityFlags_RenderWireframe)
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

	if (entity.slidable)
	{
		writer << "slidable \n";
	}
}

void EntitySerializer::ClearBuffer()
{
	relations = DeferredEntityRelationBuffer{};
}
