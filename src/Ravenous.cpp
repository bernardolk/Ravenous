/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2023 
   ========================================== */

#define PLATFORM OS_WINDOWS
#define DEBUG_BUILD

#include "Engine/Platform/Platform.h"
#include "engine/core/ui.h"
#include "engine/rvn.h"
#include "editor/tools/InputRecorder.h"

#include "editor/EditorMain.h"

#include "game/animation/AnUpdate.h"
#include "editor/console/console.h"
#include "game/GeometryData.h"
#include "game/entities/EPlayer.h"
#include "editor/editor.h"
#include "engine/camera/camera.h"
#include "engine/io/loaders.h"
#include "engine/MainLoop.h"
#include "Engine/RavenousEngine.h"
#include "engine/collision/ClController.h"
#include "engine/render/ImRender.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/world/World.h"

// FUNCTION PROTOTYPES
void LoadShaders();
void StartFrame();
void CheckAllEntitiesHaveShaders();
void CheckAllEntitiesHaveIds();
void CheckAllGeometryHasGlData();

int main()
{
	RavenousEngine::Initialize();
	
	auto* world = RWorld::Get();

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::world = world;

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
	ImDraw::Init();

	// loads initial scene
	ConfigSerializer::LoadGlobalConfigs();
	auto& program_config = *ProgramConfig::Get();
	
	WorldSerializer::LoadFromFile(program_config.initial_scene);

	EPlayer* player = EPlayer::Get();
	player->checkpoint_pos = player->position; // set player initial checkpoint position

	// set scene attrs from global config
	RCameraManager::Get()->GetCurrentCamera()->acceleration = program_config.camspeed;
	world->ambient_light = program_config.ambient_light;
	world->ambient_intensity = program_config.ambient_intensity;

	world->UpdateEntityWorldChunk(player); // sets player to the world
	CL_RecomputeCollisionBufferEntities();       // populates collision buffer and others

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
	AN_CreateHardcodedAnimations();

	//@TODO: better for debugging
	player->flags |= EntityFlags_RenderWireframe;

	world->Update();

	RavenousMainLoop();
	
}

//    ----------------------------------------------------------------

void CheckAllEntitiesHaveShaders()
{
	auto entity_iterator = RWorld::Get()->GetEntityIterator();
	while(auto* entity = entity_iterator())
	{
		if (entity->shader == nullptr)
			fatal_error("shader not set for entity '%s'.", entity->name.c_str());

		if (entity->mesh->gl_data.VAO == 0)
			fatal_error("GL DATA not set for entity '%s'.", entity->name.c_str());
	}
}

void CheckAllEntitiesHaveIds()
{
	auto entity_iterator = RWorld::Get()->GetEntityIterator();
	while(auto* entity = entity_iterator())
	{
		if (entity->id == -1)
			fatal_error("There are entities without IDs. Check scene loading code for a flaw.");
	}
}

void CheckAllGeometryHasGlData()
{
	ForIt(GeometryCatalogue)
	{
		auto item = it->second;
		if (item->gl_data.VAO == 0 || item->gl_data.VBO == 0)
		{
			assert(false);
		}
	}
}
