#pragma once

const static std::string PROJECT_PATH                    = "c:/repositories/ravenous";
const static std::string TEXTURES_PATH                   = PROJECT_PATH + "/assets/textures/";
const static std::string MODELS_PATH                     = PROJECT_PATH + "/assets/models/";
const static std::string FONTS_PATH                      = PROJECT_PATH + "/assets/fonts/";
const static std::string SHADERS_FOLDER_PATH             = PROJECT_PATH + "/shaders/";
const static std::string CAMERA_FILE_PATH                = PROJECT_PATH + "/camera.txt";
const static std::string SCENES_FOLDER_PATH              = PROJECT_PATH + "/scenes/";
const static std::string SHADERS_FILE_EXTENSION          = ".shd";
const static std::string CONFIG_FILE_PATH                = PROJECT_PATH + "/config.txt";
const static std::string SCENE_TEMPLATE_FILENAME         = "template_scene";
const static std::string INPUT_RECORDINGS_FOLDER_PATH    = PROJECT_PATH + "/recordings/";

struct GLFWwindow;

struct GlobalDisplayInfo {
   GLFWwindow* window;
   const static float VIEWPORT_WIDTH   = 1980;
   const static float VIEWPORT_HEIGHT  = 1080;
};