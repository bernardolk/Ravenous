#pragma once

#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp> 
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/compatibility.hpp>
#include <vector>
#include <parser.h>

enum CameraType {
   FREE_ROAM = 0,
   THIRD_PERSON = 1
};

struct Camera {
	vec3 Position = vec3(0.0f);
	vec3 Front = vec3(1.0f, 0.0f, 0.0f);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);

	float Acceleration = 3.5f;
	float FOVy = 45.0f;
	float FarPlane = 300.0f;
	float NearPlane = 0.1f;
	float Sensitivity = 0.1f;

	glm::mat4 View4x4;
	glm::mat4 Projection4x4;
   
   CameraType type = FREE_ROAM;
   float orbital_angle = 0;
};

// camera array indexes
u8 EDITOR_CAM = 0;
u8 FPS_CAM = 1; 

// Prototypes
void camera_update(Camera* camera, float viewportWidth, float viewportHeight, Player* player);
void camera_change_direction(Camera* camera, float yawOffset, float pitchOffset);
void camera_look_at(Camera* camera, vec3 ref, bool isPosition);
void save_camera_settings_to_file(std::string path, vec3 position, vec3 direction);
float* load_camera_settings(std::string path);
void set_camera_to_free_roam(Camera* camera);
void set_camera_to_third_person(Camera* camera, Player* player);
void compute_angles_from_direction(float& pitch, float& yaw, vec3 direction);

// Functions
void set_camera_to_free_roam(Camera* camera)
{
   camera->type = FREE_ROAM;
}

void set_camera_to_third_person(Camera* camera, Player* player)
{
   camera->type = THIRD_PERSON;
}

void camera_update(Camera* camera, float viewportWidth, float viewportHeight, Player* player)
{
   camera->View4x4 = glm::lookAt(camera->Position, camera->Position + camera->Front, camera->Up);
	camera->Projection4x4 = glm::perspective(
      glm::radians(camera->FOVy), 
      viewportWidth / viewportHeight, 
      camera->NearPlane, camera->FarPlane
   );
   
   switch(PROGRAM_MODE.current)
   {
      case GAME_MODE:
         camera->Position    = player->eye();
         player->orientation = camera->Front;
         break;
      case EDITOR_MODE:
         if(camera->type == THIRD_PERSON)
         {
            camera->Position = player->entity_ptr->position;
            camera->Position.y += 1.75;

            if (camera->orbital_angle > 360.0f)
               camera->orbital_angle -= 360.0;
            if (camera->orbital_angle < -360.0f)
               camera->orbital_angle += 360.0;

            float distance = 3;
            camera->Position.x += distance * cos(camera->orbital_angle);
            camera->Position.z += distance * sin(camera->orbital_angle); 
            camera_look_at(camera, player->entity_ptr->position, true);
         }
         break;
   }
}

void camera_change_direction(Camera* camera, float yawOffset, float pitchOffset)
{
   float pitch, yaw;
   compute_angles_from_direction(pitch, yaw, camera->Front);

	pitch     += pitchOffset;
	yaw       += yawOffset;

   // Unallows camera to perform a flip
   if (pitch > 89.0f)  pitch = 89.0f;
   if (pitch < -89.0f) pitch = -89.0f;

   // Make sure we don't overflow floats when camera is spinning indefinetely
   if (yaw > 360.0f)  yaw -= 360.0f;
   if (yaw < -360.0f) yaw += 360.0f;

	camera->Front.x   = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->Front.y   = sin(glm::radians(pitch));
	camera->Front.z   = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->Front     = glm::normalize(camera->Front);
}


void camera_look_at(Camera* camera, vec3 ref, bool isPosition)
{
   // vec3 ref -> either a position or a direction vector (no need to be normalised)
	vec3 look_vec = ref;
	if (isPosition) 
      look_vec = ref - vec3(camera->Position.x, camera->Position.y, camera->Position.z);
	look_vec = glm::normalize(look_vec);

	float pitch = glm::degrees(glm::asin(look_vec.y));
	float yaw   = glm::degrees(atan2(look_vec.x, -1 * look_vec.z) - PI / 2);

	camera->Front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->Front.y = sin(glm::radians(pitch));
	camera->Front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->Front = glm::normalize(camera->Front);
}

float* load_camera_settings(std::string path)
{
   std::ifstream reader(path);
	std::string line;

	static float camera_settings[6];

   // get camera position
   {
	   getline(reader, line);
      const char* cline = line.c_str();
      size_t size = line.size();

      Parser::Parse p { 
         cline, 
         size 
      };

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[0] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[1] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[2] = p.fToken;
   }
    // get camera direction
   {
	   getline(reader, line);
      const char* cline = line.c_str();
      size_t size = line.size();

      Parser::Parse p { 
         cline, 
         size 
      };

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[3] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[4] = p.fToken;

      p = parse_all_whitespace(p);
      p = parse_float(p);
      camera_settings[5] = p.fToken;
   }

   return &camera_settings[0];
}

void save_camera_settings_to_file(std::string path, vec3 position, vec3 direction)
{
   std::ofstream ofs;
   ofs.open(path);
   ofs << position.x << " ";
   ofs << position.y << " ";
   ofs << position.z << "\n";
   ofs << direction.x << " ";
   ofs << direction.y << " ";
   ofs << direction.z;
   ofs.close();
}

void compute_angles_from_direction(float& pitch, float& yaw, vec3 direction)
{
   pitch = glm::degrees(glm::asin(direction.y));
   yaw = glm::degrees(atan2(direction.x, -1 * direction.z) - PI / 2);
   if (pitch > 89.0f)  pitch = 89.0f;
   if (pitch < -89.0f) pitch = -89.0f;
   if (yaw > 360.0f)   yaw -= 360.0f;
   if (yaw < -360.0f)  yaw += 360.0f;
   return;
}