/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2023 
   ========================================== */


// DEPENDENCY INCLUDES
#include "engine/core/platform.h"
#include "engine/core/core.h"
#include <engine/core/ui.h>
#include "engine/core/logging.h"
#include <engine/rvn.h>
#include "editor/tools/input_recorder.h"
#include "engine/world/scene_manager.h"
#include "engine/engine_state.h"
#include "engine/platform/gl_window.h"


/**
 *	Refactor plan:
 *	
 *	1. Create a engine/core.h and put all essentials there (including fwd declarations of common types)
 *	2. Start moving methods from outer scope to static types that implement a "static T* Get()" and refactor global instances to use that method
 *	3. Refactor ravenous.cpp: Delete unused code, make core loop very high level (refactor input/update/render loops into its own Engine methods)
 *	4. Implement cpp files for header-only code (excluding editor code) and move them into proper place in file structure
 *	5. Make inputs work in this build
 *
 *	Next steps plan:
 *
 *	1. Finish reflection system to work with multiple inheritance
 *	2. Implement the MemoryArena and EntityManager code from the loose files into here
 *	3. Refactor current entity types (doors, markings, etc) into actual entity types
 *	4. Make se/deserialization work for these types
 *	5. Profit $$$
 *	
 */


// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <engine/platform/platform.h>


// SOURCE INCLUDES

#include <editor/editor_main.h>

#include "engine/entities/traits/entity_traits.h"

#include "game/animation/an_update.h"
#include "editor/console/console.h"
#include "game/geometry.h"
#include "game/entities/player.h"
#include "editor/editor.h"
#include "engine/camera/camera.h"
#include "engine/io/loaders.h"
#include "engine/main_loop.h"
#include "engine/collision/cl_controller.h"
#include "engine/render/im_render.h"
#include "engine/serialization/sr_config.h"
#include "engine/serialization/sr_world.h"
#include "engine/world/world.h"

// FUNCTION PROTOTYPES
void LoadShaders();

void StartFrame();
void CheckAllEntitiesHaveShaders();
void CheckAllEntitiesHaveIds();
void CheckAllGeometryHasGlData();

int main()
{
	auto* ES = EngineState::Get();
	auto* world = T_World::Get();

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::world = world;
	ConfigSerializer::scene_info = GlobalSceneInfo::Get();

	// INITIAL GLFW AND GLAD SETUPS
	SetupGLFW(true);
	SetupGL();

	auto* GSI = GlobalSceneInfo::Get();

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

	Player* player = GSI->player;
	world->player = player;
	player->checkpoint_pos = player->position; // set player initial checkpoint position

	// set scene attrs from global config
	GSI->camera->acceleration = program_config.camspeed;
	world->ambient_light = program_config.ambient_light;
	world->ambient_intensity = program_config.ambient_intensity;

	world->UpdateEntityWorldCells(player); // sets player to the world
	CL_RecomputeCollisionBufferEntities(player);       // populates collision buffer and others

	Editor::Initialize();

	// render features initialization
	CreateDepthBuffer();
	CreateLightSpaceTransformMatrices();

	// Pre-loop checks
	CheckAllEntitiesHaveShaders();
	CheckAllEntitiesHaveIds();
	CheckAllGeometryHasGlData();

	// load pre recorded input recordings
	InputRecorder.Load();

	// create hardcoded animations
	AN_CreateHardcodedAnimations();

	//@TODO: better for debugging
	player->flags |= EntityFlags_RenderWireframe;

	world->Update();

	/**		NEW STUFF	*/
	//auto* new_world = T_World::Get();
	
	// set first one to active
	// auto* active_chunk = new_world->chunks.GetAt(0, 0, 0);
	// new_world->active_chunks.push_back(active_chunk);

	// create entities
	// auto* door = active_chunk->RequestEntityStorage<E_Door>();
	
	// update all entities, trait by trait, world chunk by chunk
	// new_world->Update();

	RavenousMainLoop();
}

//    ----------------------------------------------------------------

void StartFrame()
{
	float current_frame_time = glfwGetTime();
	Rvn::frame.real_duration = current_frame_time - Rvn::frame.last_frame_time;
	Rvn::frame.duration = Rvn::frame.real_duration * Rvn::frame.time_step;
	Rvn::frame.last_frame_time = current_frame_time;

	// @TODO: Can't remember why this is important...
	// forces framerate for simulation to be small
	if (Rvn::frame.duration > 0.02)
	{
		Rvn::frame.duration = 0.02;
	}

	Rvn::frame.sub_second_counter += Rvn::frame.real_duration;
	Rvn::frame.fps_counter += 1;
	if (Rvn::frame.sub_second_counter > 1)
	{
		Rvn::frame.fps = Rvn::frame.fps_counter;
		Rvn::frame.fps_counter = 0;
		Rvn::frame.sub_second_counter -= 1;
	}
}

void CheckAllEntitiesHaveShaders()
{
	auto entity_iterator = T_World::Get()->GetEntityIterator();
	while(auto* entity = entity_iterator())
	{
		if (entity->shader == nullptr)
			Quit_fatal("shader not set for entity '" + entity->name + "'.");

		if (entity->mesh->gl_data.VAO == 0)
			Quit_fatal("GL DATA not set for entity '" + entity->name + "'.");
	}
}

void CheckAllEntitiesHaveIds()
{
	auto entity_iterator = T_World::Get()->GetEntityIterator();
	while(auto* entity = entity_iterator())
	{
		if (entity->name != PlayerName && entity->id == -1)
			Quit_fatal("There are entities without IDs. Check scene loading code for a flaw.");
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
