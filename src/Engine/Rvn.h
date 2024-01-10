#pragma once

#include "engine/core/core.h"

namespace Paths
{
	const static string Project = "c:/dev/ravenous";
	const static string Textures = Project + "/assets/textures/";
	const static string Models = Project + "/assets/models/";
	const static string CollisionMeshes = Project + "/assets/collision_meshes/";
	const static string Fonts = Project + "/assets/fonts/";
	const static string Shaders = Project + "/shaders/";
	const static string Camera = Project + "/camera.txt";
	const static string Scenes = Project + "/scenes/";
	const static string World = Project + "/world/";
	const static string ShaderFileExtension = ".shd";
	const static string Config = Project + "/config.txt";
	const static string SceneTemplate = "template_scene";
	const static string InputRecordings = Project + "/recordings/";
	const static string MeshExports = Project + "/bin/models/";
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

	inline static string SceneName;

	inline static vector<EntityBufferElement> EntityBuffer;
	inline static REditorMsgManager* EditorMsgManager;

	static void Init();
};

// ======================
//	Editor Messages
// ======================


#define PrintEditorMsg(msg) \
{ \
	static uint MsgId = 0; \
	Rvn::EditorMsgManager->AddMessage(MsgId, msg); \
}

// stores messages to be displayed on screen during a certain duration
struct REditorMsg
{
	inline static uint constexpr RenderMessageMaxSize = 80;

	uint Id = 0;
	string Message = "";
	float Elapsed = 0.f;
	float Duration = 0.f;
	vec3 Color;
};

struct REditorMsgManager
{
	
public:
	static constexpr inline uint MaxMessagesToRender = 4;

	void AddMessage(uint& MsgId, const string MsgString, float Duration = 2.f, vec3 Color = vec3{-1.f});
	void Update();
	void Render();
	
private:
	vector<REditorMsg> Messages;
	static inline constexpr std::hash<std::string> Hasher;
};
