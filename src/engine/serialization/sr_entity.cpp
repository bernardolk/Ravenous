#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <engine/core/types.h>
#include "engine/rvn.h"
#include "engine/core/logging.h"
#include <engine/serialization/sr_entity.h>
#include <engine/collision/collision_mesh.h>
#include <glm/gtx/quaternion.hpp>
#include "engine/collision/primitives/bounding_box.h"
#include "engine/geometry/mesh.h"
#include "engine/entities/entity.h"
#include "engine/entities/allocator/entity_pool.h"
#include "engine/entities/manager/entity_manager.h"
#include <engine/serialization/sr_common.h>
#include "engine/serialization/parsing/parser.h"
#include "engine/io/loaders.h"
#include "engine/geometry/mesh.h"
#include "engine/render/shader.h"
#include "engine/serialization/sr_entity.h"

const std::string SrLoadEntity_TypeNotSetErrorMsg = "Need to load entity type before loading type-specific data.";

void EntitySerializer::parse(Parser& parser)
{
	auto new_entity = manager->CreateEntity({});
	bool is_type_set = false;

	auto& p = parser;
	p.ParseName();
	new_entity->name = get_parsed<std::string>(parser);

	while (parser.NextLine())
	{
		p.ParseToken();
		const auto property = get_parsed<std::string>(parser);

		if (property == "id")
		{
			p.ParseAllWhitespace();
			p.ParseU64();
			u64 id = get_parsed<u64>(parser);
			new_entity->id = id;

			if (manager->next_entity_id < id)
				manager->next_entity_id = id;
		}

		else if (property == "position")
		{
			p.ParseVec3();
			new_entity->position = get_parsed<glm::vec3>(parser);
		}

		else if (property == "rotation")
		{
			p.ParseVec3();
			new_entity->rotation = get_parsed<glm::vec3>(parser);
		}

		else if (property == "scale")
		{
			p.ParseVec3();
			const auto s = get_parsed<glm::vec3>(parser);

			if (s.x < 0 || s.y < 0 || s.z < 0)
			{
				std::cout << "FATAL: ENTITY SCALE PROPERTY CANNOT BE NEGATIVE. AT '" << parser.filepath
				<< "' LINE NUMBER " << parser.line_count << "\n";
			}
			new_entity->scale = s;
		}

		else if (property == "shader")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto shader_name = get_parsed<std::string>(parser);

			auto find = ShaderCatalogue.find(shader_name);
			if (find != ShaderCatalogue.end())
			{
				if (shader_name == "tiledTextureModel")
				{
					new_entity->flags |= EntityFlags_RenderTiledTexture;
					For(6)
					{
						p.ParseAllWhitespace();
						p.ParseInt();
						if (!p.HasToken())
							Quit_fatal("Scene description contain an entity with box tiled shader without full tile quantity description.");

						new_entity->uv_tile_wrap[i] = get_parsed<int>(parser);
					}
				}
				new_entity->shader = find->second;
			}
			else
			{
				std::cout << "SHADER '" << shader_name << "' NOT FOUND WHILE LOADING SCENE DESCRIPTION FILE \n";
				assert(false);
			}
		}

		else if (property == "mesh")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto model_name = get_parsed<std::string>(parser);

			auto find_mesh = GeometryCatalogue.find(model_name);
			if (find_mesh != GeometryCatalogue.end())
				new_entity->mesh = find_mesh->second;
			else
				new_entity->mesh = LoadWavefrontObjAsMesh(Paths::Models, model_name);

			// @TODO: For now collision mesh is loaded from the same model as regular mesh.
			auto find_c_mesh = CollisionGeometryCatalogue.find(model_name);
			if (find_c_mesh != CollisionGeometryCatalogue.end())
				new_entity->collision_mesh = find_c_mesh->second;
			else
				new_entity->collision_mesh = LoadWavefrontObjAsCollisionMesh(Paths::Models, model_name);

			new_entity->collider = *new_entity->collision_mesh;
		}

		else if (property == "texture")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto texture_name = get_parsed<std::string>(parser);

			// > texture definition error handling
			// >> check for missing info
			if (texture_name.empty())
			{
				std::cout << "Fatal: Texture for entity '" << new_entity->name << "' is missing name. \n";
				assert(false);
			}

			// fetches texture in catalogue
			auto texture = TextureCatalogue.find(texture_name);
			if (texture == TextureCatalogue.end())
			{
				std::cout << "Fatal: '" << texture_name << "' was not found (not pre-loaded) inside Texture Catalogue \n";
				assert(false);
			}

			new_entity->textures.clear();
			new_entity->textures.push_back(texture->second);

			// fetches texture normal in catalogue, if any
			auto normal = TextureCatalogue.find(texture_name + "_normal");
			if (normal != TextureCatalogue.end())
			{
				new_entity->textures.push_back(normal->second);
			}
		}

		else if (property == "hidden")
		{
			new_entity->flags |= EntityFlags_HiddenEntity;
		}

		else if (property == "type")
		{
			p.ParseAllWhitespace();
			p.ParseToken();
			const auto entity_type = get_parsed<std::string>(parser);

			if (entity_type == SrEntityType::Static)
				manager->SetType(new_entity, EntityType_Static);

			else if (entity_type == SrEntityType::Checkpoint)
				manager->SetType(new_entity, EntityType_Checkpoint);

			else if (entity_type == SrEntityType::TimerTrigger)
				manager->SetType(new_entity, EntityType_TimerTrigger);

			else if (entity_type == SrEntityType::TimerTarget)
				manager->SetType(new_entity, EntityType_TimerTarget);

			else if (entity_type == SrEntityType::TimerMarking)
				manager->SetType(new_entity, EntityType_TimerMarking);

			else
				Quit_fatal("Entity type '" + entity_type + "' not identified.");

			is_type_set = true;
		}

		// ---------------------------------
		// > entity type related properties
		// ---------------------------------

		else if (property == "timer_target")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseU64();
			auto timer_target_id = get_parsed<u64>(parser);

			int i = relations.count;
			relations.deferred_entity_ids[i] = timer_target_id;
			relations.entities[i] = new_entity;
			relations.relations[i] = SrEntityRelation_TimerTarget;
			relations.count++;
		}

		else if (property == "timer_duration")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseFloat();
			new_entity->timer_trigger_data.timer_duration = get_parsed<int>(parser);
		}

		else if (property == "timer_target_type")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseUint();
			new_entity->timer_target_data.timer_target_type = static_cast<EntityTimerTargetType>(get_parsed<u32>(parser));
		}

		else if (property == "timer_start_animation")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseUint();
			new_entity->timer_target_data.timer_start_animation = get_parsed<u32>(parser);
		}

		else if (property == "timer_stop_animation")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseUint();
			new_entity->timer_target_data.timer_stop_animation = get_parsed<u32>(parser);
		}

		else if (property == "timer_marking")
		{
			if (!is_type_set) Quit_fatal(SrLoadEntity_TypeNotSetErrorMsg);

			p.ParseAllWhitespace();
			p.ParseUint();
			const auto marking_id = get_parsed<u32>(parser);

			p.ParseAllWhitespace();
			p.ParseUint();
			const auto marking_time_checkpoint = get_parsed<u32>(parser);

			int i = relations.count;
			relations.deferred_entity_ids[i] = marking_id;
			relations.entities[i] = new_entity;
			relations.relations[i] = SrEntityRelation_TimerMarking;
			relations.aux_uint_buffer[i] = marking_time_checkpoint;
			relations.count++;
		}

		else if (property == "timer_marking_color_on")
		{
			p.ParseVec3();
			new_entity->timer_marking_data.color_on = get_parsed<glm::vec3>(parser);
		}

		else if (property == "timer_marking_color_off")
		{
			p.ParseVec3();
			new_entity->timer_marking_data.color_off = get_parsed<glm::vec3>(parser);
		}

		// ---------------------------------

		else if (property == "trigger")
		{
			p.ParseVec3();
			new_entity->trigger_scale = get_parsed<glm::vec3>(parser);
		}

		else if (property == "slidable")
		{
			new_entity->slidable = true;
		}

		else
		{
			break;
		}
	}
}

void EntitySerializer::save(std::ofstream& writer, Entity& entity)
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

	if (entity.slidable)
	{
		writer << "slidable \n";
	}
}

void EntitySerializer::_clear_buffer()
{
	relations = DeferredEntityRelationBuffer{};
}
