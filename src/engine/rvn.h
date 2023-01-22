#pragma once

struct RenderMessageBuffer;
struct Scene;
struct Camera;
struct Entity;
struct Player;
struct EntityBuffer;
struct EntityBufferElement;
struct GLFWwindow;

namespace Paths
{
	const static std::string Project = "c:/dev/ravenous";
	const static std::string Textures = Project + "/assets/textures/";
	const static std::string Models = Project + "/assets/models/";
	const static std::string Fonts = Project + "/assets/fonts/";
	const static std::string Shaders = Project + "/shaders/";
	const static std::string Camera = Project + "/camera.txt";
	const static std::string Scenes = Project + "/scenes/";
	const static std::string ShaderFileExtension = ".shd";
	const static std::string Config = Project + "/config.txt";
	const static std::string SceneTemplate = "template_scene";
	const static std::string InputRecordings = Project + "/recordings/";
}
struct ProgramConfig
{
	std::string initial_scene;
	float camspeed = 1;
	vec3 ambient_light{};
	float ambient_intensity = 0;
};

struct FrameData
{
	float duration = 0;
	float real_duration = 0;
	float last_frame_time = 0;
	int fps = 0;
	int fps_counter = 0;
	float sub_second_counter = 0;
	float time_step = 1;
};

struct Rvn
{
	static constexpr size_t collision_log_buffer_capacity = 150;
	static constexpr size_t collision_log_capacity = 20;
	static constexpr size_t collision_buffer_capacity = 1000;
	static constexpr size_t message_buffer_capacity = 10;
	static constexpr int max_messages_to_render = 8;

	inline static FrameData frame;
	inline static std::string scene_name;

	inline static EntityBuffer* entity_buffer;
	inline static RenderMessageBuffer* rm_buffer;

	static void Init();
	static void PrintDynamic(const std::string& msg, float duration = 0, vec3 color = vec3(-1));
	static void Print(const std::string& msg, float duration = 0, vec3 color = vec3(-1));
};

struct GlobalSceneInfo
{
	Scene* active_scene = nullptr;
	Camera* camera = nullptr;
	Camera* views[2];
	Player* player = nullptr;
	bool input_mode = false;
	std::string scene_name;
	bool tmp_unstuck_things = false;
};

// @TODO
extern GlobalSceneInfo GSceneInfo;

struct GlobalDisplayConfig
{
	GLFWwindow* window;
	constexpr inline static float viewport_width = 1980;
	constexpr inline static float viewport_height = 1080;
};


/* ------------------
   > Entity Buffer
------------------ */
// stores all relevant entity ptrs for collision detection with player during the frame
struct EntityBufferElement
{
	Entity* entity = nullptr;
	bool collision_check = false;
};

struct EntityBuffer
{
	size_t size = 0;
	EntityBufferElement buffer[Rvn::collision_buffer_capacity];
};


/* --------------------------
   > Render Message Buffer
-------------------------- */
// stores messages to be displayed on screen during a certain duration

struct RenderMessageBufferElement
{
	std::string message;
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
	bool Add(std::string msg, float duration, vec3 color = vec3(-1));
	bool AddUnique(std::string msg, float duration, vec3 color = vec3(-1));
	void Cleanup();
	void Render();
};
