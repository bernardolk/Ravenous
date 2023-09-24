#pragma once

#include "engine/core/core.h"

namespace Paths
{
	const static string Project = "c:/dev/ravenous";
	const static string Textures = Project + "/assets/textures/";
	const static string Models = Project + "/assets/models/";
	const static string Fonts = Project + "/assets/fonts/";
	const static string Shaders = Project + "/shaders/";
	const static string Camera = Project + "/camera.txt";
	const static string Scenes = Project + "/scenes/";
	const static string ShaderFileExtension = ".shd";
	const static string Config = Project + "/config.txt";
	const static string SceneTemplate = "template_scene";
	const static string InputRecordings = Project + "/recordings/";
}

struct ProgramConfig
{
	DeclSingleton(ProgramConfig);

	string initial_scene;
	float camspeed = 1;
	vec3 ambient_light{};
	float ambient_intensity = 0;
};


// stores all relevant entity ptrs for collision detection with player during the frame
struct EntityBufferElement
{
	EEntity* entity = nullptr;
	bool collision_checked = false;
};


// TODO: get rid of this Rvn thing and CollisionBuffer concept too.
//	. For iterative collision checks, just maintain a local "checked" small array of entity ptrs or IDs and in the algorithm, if collision happens and CollidedEntities.Num() > 0, scan the array for each new test to see if it was already checked and skip if so. Use an array instead of a map because we will likely have at most 3 entities there at any test.
//	. Move all log stuff into where they belong
//	. Create Engine namespace for placing FrameData and such in it.
//	. Move program config to Editor code
//	. Create a Project Config sigleton structure to hold all the paths to things and all relevant configs. Editor changes it, Engine reads it.
//	. Create a Editor Configs singleton structure for all editor related configs such as cam speed etc, flags for debug rendering and what not
//	. Move RenderMessageBuffer into editor and refactor it.


struct Rvn
{
	static constexpr u32 collision_log_buffer_capacity = 150;
	static constexpr u32 collision_log_capacity = 20;
	static constexpr u32 collision_buffer_capacity = 1000;
	static constexpr u32 message_buffer_capacity = 300;
	static constexpr int max_messages_to_render = 8;

	inline static string scene_name;

	inline static vector<EntityBufferElement> entity_buffer{};
	inline static RenderMessageBuffer* rm_buffer;

	static void Init();
	static void PrintDynamic(const string& msg, float duration = 0, vec3 color = vec3(-1));
	static void Print(const string& msg, float duration = 0, vec3 color = vec3(-1));
};


// stores messages to be displayed on screen during a certain duration
struct RenderMessageBufferElement
{
	string message;
	float elapsed = 0;
	float duration = 0;
	vec3 color;
};

struct RenderMessageBuffer
{
	constexpr static u32 capacity = Rvn::message_buffer_capacity;
	u16 count = 0;

private:
	RenderMessageBufferElement buffer[capacity]{};

public:
	bool Add(string msg, float duration, vec3 color = vec3(-1));
	bool AddUnique(string msg, float duration, vec3 color = vec3(-1));
	void Cleanup();
	void Render();
};
