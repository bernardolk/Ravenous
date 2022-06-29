#pragma once

struct RenderMessageBuffer;
struct Scene;
struct Camera;
struct Entity;
struct Player;
struct FrameData;
struct EntityBuffer;
struct EntityBufferElement;
struct GLFWwindow;

const static std::string   PROJECT_PATH                        = "c:/repositories/ravenous";
const static std::string   TEXTURES_PATH                       = PROJECT_PATH + "/assets/textures/";
const static std::string   MODELS_PATH                         = PROJECT_PATH + "/assets/models/";
const static std::string   FONTS_PATH                          = PROJECT_PATH + "/assets/fonts/";
const static std::string   SHADERS_FOLDER_PATH                 = PROJECT_PATH + "/shaders/";
const static std::string   CAMERA_FILE_PATH                    = PROJECT_PATH + "/camera.txt";
const static std::string   SCENES_FOLDER_PATH                  = PROJECT_PATH + "/scenes/";
const static std::string   SHADERS_FILE_EXTENSION              = ".shd";
const static std::string   CONFIG_FILE_PATH                    = PROJECT_PATH + "/config.txt";
const static std::string   SCENE_TEMPLATE_FILENAME             = "template_scene";
const static std::string   INPUT_RECORDINGS_FOLDER_PATH        = PROJECT_PATH + "/recordings/";

struct RVN {
   static const size_t   COLLISION_LOG_BUFFER_CAPACITY         = 150;
   static const size_t   COLLISION_LOG_CAPACITY                = 20;
   static const size_t   COLLISION_BUFFER_CAPACITY             = 1000;
   static const size_t   MESSAGE_BUFFER_CAPACITY               = 10;
   static const int      MAX_MESSAGES_TO_RENDER                = 8;

   static FrameData              frame;
   static std::string            scene_name;

   static EntityBuffer*          entity_buffer;
   static RenderMessageBuffer*   rm_buffer;

   static void                   init                             ();
   static void                   print_dynamic                    (const std::string msg, float duration = 0, vec3 color = vec3(-1));
   static void                   print                            (const std::string msg, float duration = 0, vec3 color = vec3(-1));
};

struct FrameData {
   float    duration;
   float    real_duration;
   float    last_frame_time;
   int      fps;
   int      fps_counter;
   float    sub_second_counter;
   float    time_step            = 1;
};

struct GlobalSceneInfo {
   Scene*      active_scene = NULL;
   Camera*     camera;
   Camera*     views[2];
   Player*     player;
   bool        input_mode = false;
   std::string scene_name;
   bool        tmp_unstuck_things = false;
};

// @TODO
extern GlobalSceneInfo G_SCENE_INFO;

struct GlobalDisplayConfig {
   GLFWwindow* window;
   const inline static float VIEWPORT_WIDTH   = 1980;
   const inline static float VIEWPORT_HEIGHT  = 1080;
};


/* ------------------
   > Entity Buffer
------------------ */
// stores all relevant entity ptrs for collision detection with player during the frame
struct EntityBufferElement {
   Entity*  entity;
   bool     collision_check = false;
};

struct EntityBuffer {
   size_t               size           = 0;
   EntityBufferElement  buffer[RVN::COLLISION_BUFFER_CAPACITY];
};


/* --------------------------
   > Render Message Buffer
-------------------------- */
// stores messages to be displayed on screen during a certain duration

struct RenderMessageBufferElement {
   const std::string    message;
   float                elapsed           = 0;
   float                duration          = 0;
   const vec3           color;
};

struct RenderMessageBuffer {
   u16                         count = 0;
   RenderMessageBufferElement  buffer[RVN::MESSAGE_BUFFER_CAPACITY];

   bool add          (const std::string& msg, float duration, vec3 color = vec3(-1));
   bool add_unique   (const std::string& msg, float duration, vec3 color = vec3(-1));
   void cleanup      ();
   void render       ();
};