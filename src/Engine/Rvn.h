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
	const static string World = Project + "/world/";
	const static string ShaderFileExtension = ".shd";
	const static string Config = Project + "/config.txt";
	const static string SceneTemplate = "template_scene";
	const static string InputRecordings = Project + "/recordings/";
}

// TODO: Get Rid of this
struct ProgramConfig
{
	static ProgramConfig* Get()
	{
		static ProgramConfig Instance{};
		return &Instance;
	}

	string InitialScene;
	float Camspeed = 1;
	vec3 AmbientLight;
	float AmbientIntensity = 0;
};


// stores all relevant entity ptrs for collision detection with player during the frame
struct EntityBufferElement
{
	EEntity* Entity = nullptr;
	bool CollisionChecked = false;
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
	static constexpr uint CollisionLogBufferCapacity = 150;
	static constexpr uint CollisionLogCapacity = 20;
	static constexpr uint CollisionBufferCapacity = 1000;
	static constexpr uint MessageBufferCapacity = 300;
	static constexpr int MaxMessagesToRender = 8;

	inline static string SceneName;

	inline static vector<EntityBufferElement> EntityBuffer;
	inline static RenderMessageBuffer* RmBuffer;

	static void Init();
	static void PrintDynamic(const string& Msg, float Duration = 0, vec3 Color = vec3(-1));
	static void Print(const string& Msg, float Duration = 0, vec3 Color = vec3(-1));
};


// stores messages to be displayed on screen during a certain duration
struct RenderMessageBufferElement
{
	string Message;
	float Elapsed = 0;
	float Duration = 0;
	vec3 Color;
};

struct RenderMessageBuffer
{
	constexpr static uint Capacity = Rvn::MessageBufferCapacity;
	uint16 Count = 0;

private:
	RenderMessageBufferElement Buffer[Capacity]{};

public:
	bool Add(string Msg, float Duration, vec3 Color = vec3(-1));
	bool AddUnique(string Msg, float Duration, vec3 Color = vec3(-1));
	void Cleanup();
	void Render();
};
