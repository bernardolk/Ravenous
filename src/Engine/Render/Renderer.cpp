#include "engine/render/renderer.h"
#include "glad/glad.h"
#include "game/entities/EPlayer.h"
#include "Shader.h"
#include "engine/camera/camera.h"
#include "engine/entities/lights.h"
#include "engine/entities/Entity.h"
#include "engine/io/display.h"
#include "engine/world/World.h"
#include "text/TextRenderer.h"

void RenderMesh(const RMesh* Mesh, RenderOptions Opts)
{
	glBindVertexArray(Mesh->GLData.VAO);

	// set render modifiers
	if (Opts.Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (Opts.AlwaysOnTop)
		glDepthFunc(GL_ALWAYS);
	if (Opts.PointSize != 1.0)
		glPointSize(Opts.PointSize);
	if (Opts.LineWidth != 1.0)
		glLineWidth(Opts.LineWidth);
	if (Opts.DontCullFace)
		glDisable(GL_CULL_FACE);

	// draw
	switch (Mesh->RenderMethod)
	{
		case GL_TRIANGLE_STRIP:
			glDrawArrays(GL_TRIANGLE_STRIP, 0, Mesh->Vertices.size());
			break;
		case GL_LINE_LOOP:
			glDrawArrays(GL_LINE_LOOP, 0, Mesh->Vertices.size());
			break;
		case GL_POINTS:
			glDrawArrays(GL_POINTS, 0, Mesh->Vertices.size());
			break;
		case GL_LINES:
			glDrawArrays(GL_LINES, 0, Mesh->Vertices.size());
			break;
		case GL_TRIANGLES:
			glDrawElements(GL_TRIANGLES, Mesh->Indices.size(), GL_UNSIGNED_INT, nullptr);
		//glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
			break;
		default:
			print("WARNING: no drawing method set for mesh '%s', it won't be rendered!", Mesh->Name.c_str());
	}

	// set to defaults
	if (Opts.Wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (Opts.AlwaysOnTop)
		glDepthFunc(GL_LESS);
	if (Opts.PointSize != 1.0)
		glPointSize(1.0);
	if (Opts.LineWidth != 1.0)
		glLineWidth(1.0);
	if (Opts.DontCullFace)
		glEnable(GL_CULL_FACE);

	glBindVertexArray(0);
}


// --------------
// RENDER ENTITY
// --------------
void RenderEntity(EEntity* Entity)
{
	Entity->Shader->Use();
	Entity->Shader->SetMatrix4("model", Entity->MatModel);

	// bind appropriate textures
	uint DiffuseN = 1;
	uint SpecularN = 1;
	uint NormalN = 1;
	uint HeightN = 1;

	uint i = 0;
	for (; i < Entity->Textures.size(); i++)
	{
		// active proper texture unit before binding
		glActiveTexture(GL_TEXTURE0 + i);
		string Number;
		// @todo: can turn this into enum for faster int comparison
		string Type = Entity->Textures[i].Type;
		if (Type == "texture_diffuse")
			Number = std::to_string(DiffuseN++);
		else if (Type == "texture_specular")
			Number = std::to_string(SpecularN++);
		else if (Type == "texture_normal")
			Number = std::to_string(NormalN++);
		else if (Type == "texture_height")
			Number = std::to_string(HeightN++);

		// now set the sampler to the correct texture unit
		glUniform1i(glGetUniformLocation(Entity->Shader->GLProgramID, (Type + Number).c_str()), i);
		// and finally bind the texture
		glBindTexture(GL_TEXTURE_2D, Entity->Textures[i].ID);
	}

	// SHADOW MAPS
	{
		// shadow map texture
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(Entity->Shader->GLProgramID, "shadowMap"), i);
		glBindTexture(GL_TEXTURE_2D, RDepthMap);
		i++;

		// shadow cubemap texture
		glActiveTexture(GL_TEXTURE0 + i);
		glUniform1i(glGetUniformLocation(Entity->Shader->GLProgramID, "shadowCubemap"), i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, RDepthCubemapTexture);
		i++;
	}

	// check for tiled texture
	if (Entity->Flags & EntityFlags_RenderTiledTexture)
	{
		Entity->Shader->SetInt("texture_wrap_top", Entity->UvTileWrap[0]);
		Entity->Shader->SetInt("texture_wrap_bottom", Entity->UvTileWrap[1]);
		Entity->Shader->SetInt("texture_wrap_front", Entity->UvTileWrap[2]);
		Entity->Shader->SetInt("texture_wrap_left", Entity->UvTileWrap[3]);
		Entity->Shader->SetInt("texture_wrap_right", Entity->UvTileWrap[4]);
		Entity->Shader->SetInt("texture_wrap_back", Entity->UvTileWrap[5]);
	}

	// if (entity->type == EntityType_TimerMarking)
	// {
	// 	entity->shader->SetFloat3("color", entity->timer_marking_data.color);
	// }

	// draw mesh
	RenderOptions RenderOpts;
	RenderOpts.Wireframe = Entity->Flags & EntityFlags_RenderWireframe || Entity->Flags & EntityFlags_HiddenEntity;
	RenderMesh(Entity->Mesh, RenderOpts);

	// always good practice to set everything back to defaults once configured.
	glActiveTexture(GL_TEXTURE0);
}


void RenderEditorEntity(EEntity* Entity, RWorld* World, RCamera* Camera)
{
	Entity->Shader->Use();
	// important that the gizmo dont have a position set.
	Entity->Shader->SetMatrix4("model", Entity->MatModel);
	Entity->Shader->SetMatrix4("view", Camera->MatView);
	Entity->Shader->SetMatrix4("projection", Camera->MatProjection);
	Entity->Shader->SetMatrix4("model", Entity->MatModel);
	Entity->Shader->SetFloat3("viewPos", Camera->Position);
	Entity->Shader->SetFloat3("entity_position", Entity->Position);
	Entity->Shader->SetFloat("shininess", World->GlobalShininess);

	// RenderEntity(entity);
}


// -------------
// RENDER SCENE
// -------------
void RenderScene(RWorld* World, RCamera* Camera)
{
	EPlayer* Player = EPlayer::Get();

	// set shader settings that are common to the scene
	// both to "normal" model shader and to tiled model shader
	static auto Shaders = {ShaderCatalogue.find("model")->second, ShaderCatalogue.find("tiledTextureModel")->second, ShaderCatalogue.find("color")->second};
	for (auto* Shader : Shaders)
	{
		SetShaderLightVariables(World, Shader, Camera);
	}

	auto EntityIter = World->GetEntityIterator();
	while (auto* Entity = EntityIter())
	{
		if (Entity->Flags & EntityFlags_InvisibleEntity)
			continue;

		RenderEntity(Entity);
	}

	if (!(Player->Flags & EntityFlags_InvisibleEntity))
		RenderEntity(Player);
}


void SetShaderLightVariables(RWorld* World, RShader* Shader, RCamera* Camera)
{
	Shader->Use();

	int LightCount;
	// point lights
	{
		LightCount = 0;
		for (auto& Light : World->PointLights)
		{
			auto UniformName = "pointLights[" + std::to_string(LightCount) + "]";
			Shader->SetFloat3(UniformName + ".position", Light->Position);
			Shader->SetFloat3(UniformName + ".diffuse", Light->Diffuse);
			Shader->SetFloat3(UniformName + ".specular", Light->Specular);
			Shader->SetFloat(UniformName + ".constant", Light->IntensityConstant);
			Shader->SetFloat(UniformName + ".linear", Light->IntensityLinear);
			Shader->SetFloat(UniformName + ".quadratic", Light->IntensityQuadratic);
			LightCount++;
		}
		Shader->SetInt("num_point_lights", LightCount);
	}

	// spot lights
	{
		LightCount = 0;
		for (auto& Light : World->SpotLights)
		{
			auto UniformName = "spotLights[" + std::to_string(LightCount) + "]";
			Shader->SetFloat3(UniformName + ".position", Light->Position);
			Shader->SetFloat3(UniformName + ".direction", Light->Direction);
			Shader->SetFloat3(UniformName + ".diffuse", Light->Diffuse);
			Shader->SetFloat3(UniformName + ".specular", Light->Specular);
			Shader->SetFloat(UniformName + ".constant", Light->IntensityConstant);
			Shader->SetFloat(UniformName + ".linear", Light->IntensityLinear);
			Shader->SetFloat(UniformName + ".quadratic", Light->IntensityQuadratic);
			Shader->SetFloat(UniformName + ".innercone", Light->Innercone);
			Shader->SetFloat(UniformName + ".outercone", Light->Outercone);
			LightCount++;
		}

		Shader->SetInt("num_spot_lights", LightCount);
	}

	// directional lights
	{
		LightCount = 0;
		for (auto& Light : World->DirectionalLights)
		{
			auto UniformName = "dirLights[" + std::to_string(LightCount) + "]";
			Shader->SetFloat3(UniformName + ".direction", Light->Direction);
			Shader->SetFloat3(UniformName + ".diffuse", Light->Diffuse);
			Shader->SetFloat3(UniformName + ".specular", Light->Specular);
			LightCount++;
		}
		Shader->SetInt("num_directional_lights", LightCount);
	}

	Shader->SetMatrix4("view", Camera->MatView);
	Shader->SetMatrix4("projection", Camera->MatProjection);
	Shader->SetFloat3("viewPos", Camera->Position);
	Shader->SetFloat("shininess", World->GlobalShininess);
	Shader->SetFloat3("ambient", World->AmbientLight);
	Shader->SetFloat("ambient_intensity", World->AmbientIntensity);
	Shader->SetMatrix4("lightSpaceMatrix", RDirLightSpaceMatrix);
	Shader->SetFloat("cubemap_far_plane", RCubemapFarPlane);
}

// -------------------------
// RENDER GAME GUI
// -------------------------
void RenderGameGui(EPlayer* Player)
{
	auto Color = Player->Lives == 2 ? vec3{0.1, 0.7, 0} : vec3{0.8, 0.1, 0.1};
	RenderText("consola42", 25, 75, Color, std::to_string(Player->Lives));

	if (Player->GrabbingEntity != nullptr)
	{
		PGrab = "Grabbed: ";
		string LastGrabbed = Player->GrabbingEntity->Name;
		PGrab += "'" + LastGrabbed + "'";
	}
	RenderText(GlobalDisplayState::ViewportWidth - 400, 45, PGrab);
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
	for (unsigned int I = 0; I < 6; I++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + I, 0, GL_DEPTH_COMPONENT,
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
	float NearPlane, FarPlane;

	// directional light matrix
	NearPlane = 1.0f;
	FarPlane = 7.5f;
	mat4 LightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, NearPlane, FarPlane);
	mat4 LightView = lookAt(
		RDirectionalLightPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)
	);
	RDirLightSpaceMatrix = LightProjection * LightView;
}

// -----------------
// RENDER DEPTH MAP
// -----------------
void RenderDepthMap()
{
	// setup
	glViewport(0, 0, RShadowBufferWidth, RShadowBufferHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, RDepthMapFbo);

	glClear(GL_DEPTH_BUFFER_BIT);
	auto DepthShader = ShaderCatalogue.find("depth")->second;
	DepthShader->Use();
	DepthShader->SetMatrix4("lightSpaceMatrix", RDirLightSpaceMatrix);

	auto EntityIterator = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		if (Entity->Flags & EntityFlags_InvisibleEntity)
			continue;

		DepthShader->SetMatrix4("model", Entity->MatModel);
		RenderMesh(Entity->Mesh, RenderOptions{});
	}

	// de-setup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight);
}

void RenderDepthCubemap()
{
	auto* World = RWorld::Get();
	// for now, testing, we are doing this just for the first point light source
	if (World->PointLights.size() == 0)
		return;

	auto Light = World->PointLights[0];

	float AspectRatio = static_cast<float>(RShadowBufferWidth) / static_cast<float>(RShadowBufferHeight);
	mat4 CubemapProj = glm::perspective(glm::radians(90.0f), AspectRatio, RCubemapNearPlane, RCubemapFarPlane);
	RPointLightSpaceMatrices[0] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[1] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(-1.0, 0.0, 0.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[2] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));
	RPointLightSpaceMatrices[3] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, -1.0));
	RPointLightSpaceMatrices[4] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(0.0, 0.0, 1.0), vec3(0.0, -1.0, 0.0));
	RPointLightSpaceMatrices[5] =
	CubemapProj * lookAt(Light->Position, Light->Position + vec3(0.0, 0.0, -1.0), vec3(0.0, -1.0, 0.0));

	// setup
	glViewport(0, 0, RShadowBufferWidth, RShadowBufferHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, RDepthCubemapFbo);

	glClear(GL_DEPTH_BUFFER_BIT);
	auto DepthShader = ShaderCatalogue.find("depth_cubemap")->second;
	DepthShader->Use();

	for (unsigned int i = 0; i < 6; ++i)
		DepthShader->SetMatrix4("shadowMatrices[" + std::to_string(i) + "]", RPointLightSpaceMatrices[i]);

	DepthShader->SetFloat("cubemap_far_plane", RCubemapFarPlane);
	DepthShader->SetFloat3("lightPos", Light->Position);

	auto EntityIterator = World->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		if (Entity->Flags & EntityFlags_InvisibleEntity)
			continue;

		DepthShader->SetMatrix4("model", Entity->MatModel);
		RenderMesh(Entity->Mesh, RenderOptions{});
	}

	// de-setup
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight);
}

void RenderDepthMapDebug()
{
	glViewport(0, 0, GlobalDisplayState::ViewportWidth, GlobalDisplayState::ViewportHeight);
	//glClearColor(0.196, 0.298, 0.3607, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// shadow map texture


	auto DepthDebugShader = ShaderCatalogue.find("depth_debug")->second;
	DepthDebugShader->Use();
	/*
	DepthDebugShader->setInt("depthMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, R_DEPTH_MAP);
	auto plane = Geometry_Catalogue.find("plane")->second;
	render_mesh(plane);
	*/

	auto Aabb = GeometryCatalogue.find("aabb")->second;
	RenderMesh(Aabb);
}

// void use_depth_map()
// {
// 	glViewport(0, 0, GlobalDisplayConfig::VIEWPORT_WIDTH, GlobalDisplayConfig::VIEWPORT_HEIGHT);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    ConfigureShaderAndMatrices();
//    glBindTexture(GL_TEXTURE_2D, depthMap);
// }
