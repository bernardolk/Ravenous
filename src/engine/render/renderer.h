#pragma once

#include "engine/core/core.h"

constexpr unsigned int RShadowBufferWidth = 1920, RShadowBufferHeight = 1080;
inline unsigned int RDepthMapFbo;
inline unsigned int RDepthMap;

inline mat4 RDirLightSpaceMatrix;
inline vec3 RDirectionalLightPos = vec3{-2.0f, 4.0f, -1.0f};

// depth cubemap (point light shadow)
inline unsigned int RDepthCubemapFbo;
inline mat4 RPointLightSpaceMatrices[6];
inline unsigned int RDepthCubemapTexture;
inline float RCubemapNearPlane = 1.0f;
inline float RCubemapFarPlane = 25.0f;

struct RenderOptions
{
	bool wireframe = false;
	bool always_on_top = false;
	float point_size = 1.0;
	float line_width = 1.0;
	// for immediate point shader
	vec3 color = vec3{-1.0};
	float opacity = 1.0;
	bool dont_cull_face = false;
};

void render_mesh(const Mesh* mesh, RenderOptions opts = RenderOptions{});
void RenderEntity(Entity* entity);
void render_scene(World* world, Camera* camera);
void render_editor_entity(Entity* entity, World* world, Camera* camera);
void set_shader_light_variables(World* world, Shader* shader, Camera* camera);


// leave for debugging
inline std::string PGrab = "Grabbed: ";
inline int PFloor = -1;

// --------------
// RENDER ENTITY
// --------------
void RenderEntity(Entity* entity);
void render_editor_entity(Entity* entity, World* world, Camera* camera);


// -------------
// RENDER SCENE
// -------------
void render_scene(World* world, Camera* camera);
void set_shader_light_variables(World* world, Shader* shader, Camera* camera);

// -------------------------
// RENDER GAME GUI
// -------------------------
void render_game_gui(Player* player);

// ----------------
// RENDER FEATURES
// ----------------
void create_depth_buffer();
void create_light_space_transform_matrices();

// -----------------
// RENDER DEPTH MAP
// -----------------
void render_depth_map(World* world);
void render_depth_cubemap(World* world);
void render_depth_map_debug();
