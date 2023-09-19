/* ==========================================
                     RAVENOUS
   ==========================================
     By Bernardo L. Knackfuss - 2020 - 2023 
   ========================================== */


// DEPENDENCY INCLUDES
#include "engine/core/platform.h"
#include "engine/core/core.h"
#include <engine/core/ui.h>
#include <engine/rvn.h>
#include "editor/tools/input_recorder.h"
#include "engine/platform/gl_window.h"

// should be conditional in the future to support multiple platforms and
// we must abstract the function calls to a common layer which can interop
// between platform layers depending on the underlying OS.
#include <engine/platform/platform.h>


// SOURCE INCLUDES

#include <editor/editor_main.h>

#include "engine/entities/traits/entity_traits.h"

#include "game/animation/an_update.h"
#include "editor/console/console.h"
#include "game/geometry_data.h"
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
	
	auto* world = T_World::Get();

	//@TODO: This here is not working because EntityManager copy constructor was deleted. This is an issue
	//    with using references it seems? A pointer would never complain about this. I should dig into this.
	//    If I have to start writing extra code to use references then I can't justify using them.
	WorldSerializer::world = world;
	
	// INITIAL GLFW AND GLAD SETUPS
	SetupGLFW(true);
	SetupGL();

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

	Player* player = Player::Get();
	player->checkpoint_pos = player->position; // set player initial checkpoint position

	// set scene attrs from global config
	CameraManager::Get()->GetCurrentCamera()->acceleration = program_config.camspeed;
	world->ambient_light = program_config.ambient_light;
	world->ambient_intensity = program_config.ambient_intensity;

	world->UpdateEntityWorldCells(player); // sets player to the world
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
	InputRecorder.Load();

	// create hardcoded animations
	AN_CreateHardcodedAnimations();

	//@TODO: better for debugging
	player->flags |= EntityFlags_RenderWireframe;

	world->Update();

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
			fatal_error("shader not set for entity '%s'.", entity->name.c_str());

		if (entity->mesh->gl_data.VAO == 0)
			fatal_error("GL DATA not set for entity '%s'.", entity->name.c_str());
	}
}

void CheckAllEntitiesHaveIds()
{
	auto entity_iterator = T_World::Get()->GetEntityIterator();
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
