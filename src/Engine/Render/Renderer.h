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

// leave for debugging
inline std::string PGrab = "Grabbed: ";
inline int PFloor = -1;

// --------------
// RENDER MESH
// --------------
void RenderMesh(const RMesh* mesh, RenderOptions opts = RenderOptions{});

// --------------
// RENDER ENTITY
// --------------
void RenderEntity(EEntity* entity);
void RenderEditorEntity(EEntity* entity, RWorld* world, RCamera* camera);

// -------------
// RENDER SCENE
// -------------
void RenderScene(RWorld* world, RCamera* camera);
void SetShaderLightVariables(RWorld* world, RShader* shader, RCamera* camera);

// -------------------------
// RENDER GAME GUI
// -------------------------
void RenderGameGui(EPlayer* player);

// ----------------
// RENDER FEATURES
// ----------------
void CreateDepthBuffer();
void CreateLightSpaceTransformMatrices();

// -----------------
// RENDER DEPTH MAP
// -----------------
void RenderDepthMap();
void RenderDepthCubemap();
void RenderDepthMapDebug();
