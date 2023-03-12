#include <engine/render/renderer.h>
#include <glad/glad.h>
#include "game/entities/player.h"
#include "shader.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/entities/entity.h"
#include "engine/io/display.h"
#include "engine/world/world.h"
#include "text/text_renderer.h"

void RenderMesh(const Mesh* mesh, RenderOptions opts)
{
	glBindVertexArray(mesh->gl_data.VAO);

	// set render modifiers
	if (opts.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (opts.always_on_top)
		glDepthFunc(GL_ALWAYS);
	if (opts.point_size != 1.0)
		glPointSize(opts.point_size);
	if (opts.line_width != 1.0)
		glLineWidth(opts.line_width);
	if (opts.dont_cull_face)
		glDisable(GL_CULL_FACE);

	// draw
	switch (mesh->render_method)
	{
		case GL_TRIANGLE_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP, 0, mesh->vertices.size());
			break;
		case GL_LINE_LOOP:
			glDrawArrays(GL_LINE_LOOP, 0, mesh->vertices.size());
			break;
		case GL_POINTS:
			glDrawArrays(GL_POINTS, 0, mesh->vertices.size());
			break;
		case GL_LINES:
			glDrawArrays(GL_LINES, 0, mesh->vertices.size());
			break;
		case GL_TRIANGLES:
			glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_INT, nullptr);
		//glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
			break;
		default:
			std::cout << "WARNING: no drawing method set for mesh '" << mesh->name << "', " <<
			"it won't be rendered!\n";
	}

	// set to defaults
	if (opts.wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (opts.always_on_top)
		glDepthFunc(GL_LESS);
	if (opts.point_size != 1.0)
		glPointSize(1.0);
	if (opts.line_width != 1.0)
		glLineWidth(1.0);
	if (opts.dont_cull_face)
		glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
}


// --------------
// RENDER ENTITY
// --------------
void RenderEntity(Entity* entity)
{
	entity->shader->Use();
	entity->shader->SetMatrix4("model", entity->mat_model);

	// bind appropriate textures
	u32 diffuse_n = 1;
	u32 specular_n = 1;
	u32 normal_n = 1;
	u32 height_n = 1;

	u32 i;
	for (i = 0; i < entity->textures.size(); i++)
	{
		// active proper texture unit before binding
		glActiveTexture(GL_TEXTURE0 + i);
		std::string number;
		// @todo: can turn this into enum for faster int comparison
		std::string type = entity->textures[i].type;
		if (type == "texture_diffuse")
			number = std::to_string(diffuse_n++);
		else if (type == "texture_specular")
			number = std::to_string(specular_n++);
		else if (type == "texture_normal")
			number = std::to_string(normal_n++);
		else if (type == "texture_height")
			number = std::to_string(height_n++);

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(entity->shader->gl_program_id, (type + number).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, entity->textures[i].id);
	}

	// SHADOW MAPS
	{
		// shadow map texture
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(entity->shader->gl_program_id, "shadowMap"), i);
		glBindTexture(GL_TEXTURE_2D, RDepthMap);
		i++;

		// shadow cubemap texture
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(entity->shader->gl_program_id, "shadowCubemap"), i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, RDepthCubemapTexture);
		i++;
	}

	// check for tiled texture
	if (entity->flags & EntityFlags_RenderTiledTexture)
	{
		entity->shader->SetInt("texture_wrap_top", entity->uv_tile_wrap[0]);
		entity->shader->SetInt("texture_wrap_bottom", entity->uv_tile_wrap[1]);
		entity->shader->SetInt("texture_wrap_front", entity->uv_tile_wrap[2]);
		entity->shader->SetInt("texture_wrap_left", entity->uv_tile_wrap[3]);
		entity->shader->SetInt("texture_wrap_right", entity->uv_tile_wrap[4]);
		entity->shader->SetInt("texture_wrap_back", entity->uv_tile_wrap[5]);
	}

	if (entity->type == EntityType_TimerMarking)
	{
		entity->shader->SetFloat3("color", entity->timer_marking_data.color);
	}

	// draw mesh
	RenderOptions render_opts;
	render_opts.wireframe = entity->flags & EntityFlags_RenderWireframe || entity->flags & EntityFlags_HiddenEntity;
	RenderMesh(entity->mesh, render_opts);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}


void RenderEditorEntity(Entity* entity, World* world, Camera* camera)
{
	entity->shader->Use();
	// important that the gizmo dont have a position set.
	entity->shader->SetMatrix4("model", entity->mat_model);
	entity->shader->SetMatrix4("view", camera->mat_view);
	entity->shader->SetMatrix4("projection", camera->mat_projection);
	entity->shader->SetMatrix4("model", entity->mat_model);
	entity->shader->SetFloat3("viewPos", camera->position);
	entity->shader->SetFloat3("entity_position", entity->position);
	entity->shader->SetFloat("shininess", world->global_shininess);

	RenderEntity(entity);
}


// -------------
// RENDER SCENE
// -------------
void RenderScene(World* world, Camera* camera)
{
	// set shader settings that are common to the scene
	// both to "normal" model shader and to tiled model shader
	auto model_shader = ShaderCatalogue.find("model")->second;
	SetShaderLightVariables(world, model_shader, camera);

	auto model_tiled_shader = ShaderCatalogue.find("tiledTextureModel")->second;
	SetShaderLightVariables(world, model_tiled_shader, camera);

	auto color_shader = ShaderCatalogue.find("color")->second;
	SetShaderLightVariables(world, color_shader, camera);

	for (int i = 0; i < world->entities.size(); i++)
	{
		Entity* entity = world->entities[i];
		if (entity->flags & EntityFlags_InvisibleEntity)
			continue;

		RenderEntity(entity);
	}
}


void SetShaderLightVariables(World* world, Shader* shader, Camera* camera)
{
	shader->Use();

	int light_count;
	// point lights
	{
		light_count = 0;
		for (auto& light : world->point_lights)
		{
			auto uniform_name = "pointLights[" + std::to_string(light_count) + "]";
			shader->SetFloat3(uniform_name + ".position", light->position);
			shader->SetFloat3(uniform_name + ".diffuse", light->diffuse);
			shader->SetFloat3(uniform_name + ".specular", light->specular);
			shader->SetFloat(uniform_name + ".constant", light->intensity_constant);
			shader->SetFloat(uniform_name + ".linear", light->intensity_linear);
			shader->SetFloat(uniform_name + ".quadratic", light->intensity_quadratic);
			light_count++;
		}
		shader->SetInt("num_point_lights", light_count);
	}

	// spot lights
	{
		light_count = 0;
		for (auto& light : world->spot_lights)
		{
			auto uniform_name = "spotLights[" + std::to_string(light_count) + "]";
			shader->SetFloat3(uniform_name + ".position", light->position);
			shader->SetFloat3(uniform_name + ".direction", light->direction);
			shader->SetFloat3(uniform_name + ".diffuse", light->diffuse);
			shader->SetFloat3(uniform_name + ".specular", light->specular);
			shader->SetFloat(uniform_name + ".constant", light->intensity_constant);
			shader->SetFloat(uniform_name + ".linear", light->intensity_linear);
			shader->SetFloat(uniform_name + ".quadratic", light->intensity_quadratic);
			shader->SetFloat(uniform_name + ".innercone", light->innercone);
			shader->SetFloat(uniform_name + ".outercone", light->outercone);
			light_count++;
		}

		shader->SetInt("num_spot_lights", light_count);
	}

	// directional lights
	{
		light_count = 0;
		for (auto& light : world->directional_lights)
		{
			auto uniform_name = "dirLights[" + std::to_string(light_count) + "]";
			shader->SetFloat3(uniform_name + ".direction", light->direction);
			shader->SetFloat3(uniform_name + ".diffuse", light->diffuse);
			shader->SetFloat3(uniform_name + ".specular", light->specular);
			light_count++;
		}
		shader->SetInt("num_directional_lights", light_count);
	}

	shader->SetMatrix4("view", camera->mat_view);
	shader->SetMatrix4("projection", camera->mat_projection);
	shader->SetFloat3("viewPos", camera->position);
	shader->SetFloat("shininess", world->global_shininess);
	shader->SetFloat3("ambient", world->ambient_light);
	shader->SetFloat("ambient_intensity", world->ambient_intensity);
	shader->SetMatrix4("lightSpaceMatrix", RDirLightSpaceMatrix);
	shader->SetFloat("cubemap_far_plane", RCubemapFarPlane);
}

// -------------------------
// RENDER GAME GUI
// -------------------------
void RenderGameGui(Player* player)
{
	auto color = player->lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1};
	RenderText("consola42", 25, 75, color, std::to_string(player->lives));

	if (player->grabbing_entity != nullptr)
	{
		PGrab = "Grabbed: ";
		std::string last_grabbed = player->grabbing_entity->name;
		PGrab += "'" + last_grabbed + "'";
	}
	RenderText(GlobalDisplayConfig::viewport_width - 400, 45, PGrab);

	std::string player_floor = "player floor: ";
	if (player->standing_entity_ptr != nullptr)
	{
		player_floor += player->standing_entity_ptr->name;
		if (PFloor != player->standing_entity_ptr->id)
		{
			PFloor = player->standing_entity_ptr->id;
			std::cout << "new floor: " << PFloor << "\n";
		}
	}
	RenderText(GlobalDisplayConfig::viewport_width - 400, 60, player_floor);
}

// ----------------
// RENDER FEATURES
// ----------------

void CreateDepthBuffer()
{
	// create framebuffer objects
	glGenFramebuffers(1, &RDepthMapFbo);
	glGenFramebuffers(1, &RDepthCubemapFbo);

	// for directional lights:
	// create framebuffer depth buffer texture
	glGenTextures(1, &RDepthMap);
	glBindTexture(GL_TEXTURE_2D, RDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, RShadowBufferWidth,
		RShadowBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	// bind texture to framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, RDepthMapFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, RDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// for point lights:
	glGenTextures(1, &RDepthCubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, RDepthCubemapTexture);
	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			RShadowBufferWidth, RShadowBufferHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
		);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, RDepthCubemapFbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, RDepthCubemapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CreateLightSpaceTransformMatrices()
{
	float near_plane, far_plane;

	// directional light matrix
	near_plane = 1.0f;
	far_plane = 7.5f;
	mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	mat4 light_view = lookAt(
		RDirectionalLightPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)
	);
	RDirLightSpaceMatrix = light_projection * light_view;
}

// -----------------
// RENDER DEPTH MAP
// -----------------
void RenderDepthMap(World* world)
{
	// setup
	glViewport(0, 0, RShadowBufferWidth, RShadowBufferHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, RDepthMapFbo);

	glClear(GL_DEPTH_BUFFER_BIT);
	auto depth_shader = ShaderCatalogue.find("depth")->second;
	depth_shader->Use();
	depth_shader->SetMatrix4("lightSpaceMatrix", RDirLightSpaceMatrix);

	for (int it = 0; it < world->entities.size(); it++)
	{
		Entity* entity = world->entities[0];
		if (entity->flags & EntityFlags_InvisibleEntity)
			continue;

		depth_shader->SetMatrix4("model", entity->mat_model);
		RenderMesh(entity->mesh, RenderOptions{});
	}

	// de-setup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height);
}

void RenderDepthCubemap(World* world)
{
	// for now, testing, we are doing this just for the first point light source
	if (world->point_lights.size() == 0)
		return;

	auto light = world->point_lights[0];

	float aspect = static_cast<float>(RShadowBufferWidth) / static_cast<float>(RShadowBufferHeight);
	mat4 cubemap_proj = glm::perspective(glm::radians(90.0f), aspect, RCubemapNearPlane, RCubemapFarPlane);
	RPointLightSpaceMatrices[0] =
	cubemap_proj * lookAt(light->position, light->position + vec3(1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[1] =
	cubemap_proj * lookAt(light->position, light->position + vec3(-1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[2] =
	cubemap_proj * lookAt(light->position, light->position + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
	RPointLightSpaceMatrices[3] =
	cubemap_proj * lookAt(light->position, light->position + vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, -1.0));
	RPointLightSpaceMatrices[4] =
	cubemap_proj * lookAt(light->position, light->position + vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[5] =
	cubemap_proj * lookAt(light->position, light->position + vec3(0.0, 0.0, -1.0), vec3(0.0, -1.0, 0.0));

	// setup
	glViewport(0, 0, RShadowBufferWidth, RShadowBufferHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, RDepthCubemapFbo);

	glClear(GL_DEPTH_BUFFER_BIT);
	auto depth_shader = ShaderCatalogue.find("depth_cubemap")->second;
	depth_shader->Use();

	for (unsigned int i = 0; i < 6; ++i)
		depth_shader->SetMatrix4("shadowMatrices[" + std::to_string(i) + "]", RPointLightSpaceMatrices[i]);

	depth_shader->SetFloat("cubemap_far_plane", RCubemapFarPlane);
	depth_shader->SetFloat3("lightPos", light->position);

	for (int i = 0; i < world->entities.size(); i++)
	{
		auto entity = world->entities[i];
		if (entity->flags & EntityFlags_InvisibleEntity)
			continue;

		depth_shader->SetMatrix4("model", entity->mat_model);
		RenderMesh(entity->mesh, RenderOptions{});
	}

	// de-setup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height);
}

void RenderDepthMapDebug()
{
	glViewport(0, 0, GlobalDisplayConfig::viewport_width, GlobalDisplayConfig::viewport_height);
	//glClearColor(0.196, 0.298, 0.3607, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// shadow map texture


	auto depth_debug_shader = ShaderCatalogue.find("depth_debug")->second;
	depth_debug_shader->Use();
	/*
	depth_debug_shader->setInt("depthMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, R_DEPTH_MAP);
	auto plane = Geometry_Catalogue.find("plane")->second;
	render_mesh(plane);
	*/

	auto aabb = GeometryCatalogue.find("aabb")->second;
	RenderMesh(aabb);
}

// void use_depth_map()
// {
// 	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    ConfigureShaderAndMatrices();
//    glBindTexture(GL_TEXTURE_2D, depthMap);
// }
