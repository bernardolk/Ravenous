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
#include "Game/GeometryData.h"
#include "Game/Entities/EPlayer.h"
#include "Engine/Camera/camera.h"
#include "Engine/Io/loaders.h"
#include "Engine/MainLoop.h"
#include "Engine/RavenousEngine.h"
#include "Engine/Collision/ClController.h"
#include "Engine/Render/ImRender.h"
#include "Engine/Serialization/sr_config.h"
#include "Engine/Serialization/sr_world.h"
#include "Engine/World/World.h"

// FUNCTION PROTOTYPES
void LoadShaders();
void StartFrame();
void CheckAllEntitiesHaveShaders();
void CheckAllEntitiesHaveIds();
void CheckAllGeometryHasGlData();

int main()
{
	RavenousEngine::Initialize();

	auto* World = RWorld::Get();

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::World = World;

	// load shaders, textures and geometry
	stbi_set_flip_vertically_on_load(true);
	LoadTexturesFromAssetsFolder();
	LoadShaders();
	LoadModels();

	// Allocate buffers and logs
	Rvn::Init();
	// COLLISION_LOG           = CL_allocate_collision_log();
	InitializeConsoleBuffers();

	// Initialises immediate draw
	RImDraw::Init();

	// loads initial scene
	ConfigSerializer::LoadGlobalConfigs();
	auto& ProgramConfig = *ProgramConfig::Get();

	WorldSerializer::LoadFromFile(ProgramConfig.InitialScene);

	EPlayer* Player = EPlayer::Get();
	Player->CheckpointPos = Player->Position; // set player initial checkpoint position

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
	CheckAllEntitiesHaveIds();
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

//    ----------------------------------------------------------------

void CheckAllEntitiesHaveShaders()
{
	auto EntityIterator = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		if (Entity->Shader == nullptr)
			fatal_error("shader not set for entity '%s'.", Entity->Name.c_str());

		if (Entity->Mesh->GLData.VAO == 0)
			fatal_error("GL DATA not set for entity '%s'.", Entity->Name.c_str());
	}
}

void CheckAllEntitiesHaveIds()
{
	auto EntityIterator = RWorld::Get()->GetEntityIterator();
	while (auto* Entity = EntityIterator())
	{
		if (Entity->ID == -1)
			fatal_error("There are entities without IDs. Check scene loading code for a flaw.");
	}
}

void CheckAllGeometryHasGlData()
{
	ForIt(GeometryCatalogue)
	{
		auto Item = it->second;
		if (Item->GLData.VAO == 0 || Item->GLData.VBO == 0)
		{
			assert(false);
		}
	}
}
