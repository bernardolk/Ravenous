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
	string initial_scene;
	float camspeed = 1;
	vec3 ambient_light{};
	float ambient_intensity = 0;

public:
	static ProgramConfig* Get()
	{
		static ProgramConfig instance;
		return &instance;
	}

private:
	ProgramConfig() = default;
	ProgramConfig(const ProgramConfig& other) = delete;
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


// stores all relevant entity ptrs for collision detection with player during the frame
struct EntityBufferElement
{
	E_Entity* entity = nullptr;
	bool collision_checked = false;
};


struct Rvn
{
	static constexpr size_t collision_log_buffer_capacity = 150;
	static constexpr size_t collision_log_capacity = 20;
	static constexpr size_t collision_buffer_capacity = 1000;
	static constexpr size_t message_buffer_capacity = 300;
	static constexpr int max_messages_to_render = 8;

	inline static FrameData frame;
	inline static std::string scene_name;

	inline static vector<EntityBufferElement> entity_buffer{};
	inline static RenderMessageBuffer* rm_buffer;

	static void Init();
	static void PrintDynamic(const std::string& msg, float duration = 0, vec3 color = vec3(-1));
	static void Print(const std::string& msg, float duration = 0, vec3 color = vec3(-1));

	static float GetFrameDuration() { return frame.duration; }
};


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
