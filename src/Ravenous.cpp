/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2023 
   ========================================== */

#define PLATFORM OS_WINDOWS
#define DEBUG_BUILD

#include "Engine/Platform/Platform.h"
#include "Engine/core/ui.h"
#include "Engine/rvn.h"
#include "Editor/tools/InputRecorder.h"

#include "Editor/EditorMain.h"

#include "Game/Animation/AnUpdate.h"
#include "Editor/Console/Console.h"
#include "Editor/Reflection/Serialization.h"
#include "Engine/IO/loaders.h"
#include "Game\Entities\Player.h"
#include "Engine/Camera/camera.h"
#include "Engine/MainLoop.h"
#include "Engine/RavenousEngine.h"
#include "Engine/Collision/ClController.h"
#include "Engine/Render/ImRender.h"
#include "engine/render/Shader.h"
#include "Engine/Serialization/sr_config.h"
#include "Engine/World/World.h"

// FUNCTION PROTOTYPES
void StartFrame();
void CheckAllEntitiesHaveShaders();
void CheckAllEntitiesHaveIds();
void CheckAllGeometryHasGlData();

#include "Test/TestSerialization.h"

int main()
{
	RavenousEngine::Initialize();
	
	auto* World = RWorld::Get();

	// load shaders, textures and geometry
	stbi_set_flip_vertically_on_load(true);
	LoadTexturesFromAssetsFolder();
	LoadShaders();
	LoadModels();

	// Allocate buffers and logs
	Rvn::Init();
	InitializeConsoleBuffers();

	// Initialises immediate draw
	RImDraw::Init();

	// loads initial scene
	ConfigSerializer::LoadGlobalConfigs();
	auto& ProgramConfig = *ProgramConfig::Get();

	Serialization::LoadWorldFromDisk();

	EPlayer* Player = EPlayer::Get();

	// set scene attrs from global config
	RCameraManager::Get()->GetCurrentCamera()->Acceleration = ProgramConfig.Camspeed;
	World->AmbientLight = ProgramConfig.AmbientLight;
	World->AmbientIntensity = ProgramConfig.AmbientIntensity;

	World->UpdateEntityWorldChunk(Player); // sets player to the world
	ClRecomputeCollisionBufferEntities(); // populates collision buffer and others

	Editor::Initialize();

	// render features initialization
	CreateDepthBuffer();
	CreateLightSpaceTransformMatrices();

	// Pre-loop checks
	CheckAllEntitiesHaveShaders();
	CheckAllGeometryHasGlData();

	// load pre recorded input recordings
	RInputRecorder::Get()->Load();

	// create hardcoded animations
	AnCreateHardcodedAnimations();

	//@TODO: better for debugging
	Player->Flags |= EntityFlags_RenderWireframe;

	World->Update();

	RavenousMainLoop();
}

// =================================
// Utility / Safety Checks
// =================================

void CheckAllEntitiesHaveShaders()
{
	auto** DefaultShaderPtrPtr = Find(ShaderCatalogue, DefaultEntityShader);
	if (!DefaultShaderPtrPtr || !*DefaultShaderPtrPtr) {
		FatalError("Initialization: Default shader \"%s\" not found.", DefaultEntityShader.c_str())
	}
	
	REntityIterator It;
	while (auto* Entity = It())
	{
		if (Entity->Shader == nullptr) {
			Log("Initialization: shader is not set for entity '%s'.", Entity->Name.c_str())
			Entity->Shader = *DefaultShaderPtrPtr;
		}

		if (Entity->Mesh->GLData.VAO == 0) {
			FatalError("Initialization: GL DATA not set for entity '%s'.", Entity->Name.c_str())
		}
	}
}

void CheckAllGeometryHasGlData()
{
	for (auto& [k, Item] : GeometryCatalogue) {
		if (Item->GLData.VAO == 0 || Item->GLData.VBO == 0) {
			FatalError("GLData not set for entity")
		}
	}
}