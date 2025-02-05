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

struct RRenderOptions
{
	bool Wireframe = false;
	bool AlwaysOnTop = false;
	float PointSize = 1.0;
	float LineWidth = 1.0;
	// for immediate point shader
	vec3 Color = vec3{-1.0};
	float Opacity = 1.0;
	bool DontCullFace = false;
};

// leave for debugging
inline std::string PGrab = "Grabbed: ";
inline int PFloor = -1;

// --------------
// RENDER MESH
// --------------
void RenderMesh(const RMesh* Mesh, RRenderOptions Opts = RRenderOptions{});

// --------------
// RENDER ENTITY
// --------------
void RenderEntity(EEntity* Entity);
void RenderEditorEntity(EEntity* Entity, RWorld* World, RCamera* Camera);

// -------------
// RENDER SCENE
// -------------
void RenderScene(RWorld* World, RCamera* Camera);
void SetShaderLightVariables(RWorld* World, RShader* Shader, RCamera* Camera);

// -------------------------
// RENDER GAME GUI
// -------------------------
void RenderGameGui(EPlayer* Player);

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
