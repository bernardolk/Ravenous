#pragma once

#include <glm/glm.hpp>
#include <glm/ext/vector_float2.hpp> // vec2
#include <glm/ext/vector_float3.hpp> // vec3
#include <glm/gtx/compatibility.hpp>
#include <vector>
#include <parser.h>

enum CameraType {
   FREE_ROAM = 0,
   THIRD_PERSON = 1
};

struct Camera {
	vec3 Position;
	vec3 Front = vec3(1.0f, 0.0f, 0.0f);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);
	float Acceleration = 3.5f;
	float FOVy = 45.0f;
	float FarPlane = 300.0f;
	float NearPlane = 0.1f;
	float Sensitivity = 0.1f;
	float Yaw = 0.0f;
	float Pitch = 0.0f;
	glm::mat4 View4x4;
	glm::mat4 Projection4x4;
   CameraType type = FREE_ROAM;
   float orbital_angle = 0;
};



// Prototypes
void camera_update(Camera* camera, float viewportWidth, float viewportHeight, Player* player);
void camera_change_direction(Camera* camera, float yawOffset, float pitchOffset);
// Make camera look at a place in world coordinates to look at. If isPosition is set to true, then
// a position is expected, if else, then a direction is expected.
void camera_look_at(Camera* camera, vec3 position, bool isPosition);
Camera* camera_create(vec3 initialPosition, vec3 direction);
void save_camera_settings_to_file(string path, vec3 position, vec3 direction);
float* load_camera_settings(string path);
void set_camera_to_free_roam(Camera* camera);
void set_camera_to_third_person(Camera* camera, Player* player);


// Functions
void set_camera_to_free_roam(Camera* camera)
{
   camera->type = FREE_ROAM;
}

void set_camera_to_third_person(Camera* camera, Player* player)
{
   camera->type = THIRD_PERSON;
}

void camera_update(Camera* camera, float viewportWidth, float viewportHeight, Player* player) {
	camera->View4x4 = glm::lookAt(camera->Position, camera->Position + camera->Front, camera->Up);
	camera->Projection4x4 = glm::perspective(
      glm::radians(camera->FOVy), 
      viewportWidth / viewportHeight, 
      camera->NearPlane, camera->FarPlane
   );

   if(camera->type == THIRD_PERSON)
   {
      camera->Position = player->entity_ptr->position;
      camera->Position.y += 0.75;

      if (camera->orbital_angle > 360.0f)
         camera->orbital_angle -= 360.0;
      if (camera->orbital_angle < -360.0f)
         camera->orbital_angle += 360.0;

      camera->Position.x += 2 * cos(camera->orbital_angle);
      camera->Position.z += 2 * sin(camera->orbital_angle); 
      camera_look_at(camera, player->entity_ptr->position, true);
   }
}


void camera_change_direction(Camera* camera, float yawOffset, float pitchOffset) {
	float newPitch = camera->Pitch += pitchOffset;
	float newYaw = camera->Yaw += yawOffset;
	camera->Front.x = cos(glm::radians(newPitch)) * cos(glm::radians(newYaw));
	camera->Front.y = sin(glm::radians(newPitch));
	camera->Front.z = cos(glm::radians(newPitch)) * sin(glm::radians(newYaw));
	camera->Front = glm::normalize(camera->Front);

    // Unallows camera to perform a flip
   if (camera->Pitch > 89.0f)
     camera->Pitch = 89.0f;
   if (camera->Pitch < -89.0f)
      camera->Pitch = -89.0f;

   // Make sure we don't overflow floats when camera is spinning indefinetely
   if (camera->Yaw > 360.0f)
      camera->Yaw -= 360.0f;
   if (camera->Yaw < -360.0f)
      camera->Yaw += 360.0f;
}


void camera_look_at(Camera* camera, vec3 position, bool isPosition) {
	vec3 look_vec;
	if (isPosition)
		look_vec = glm::normalize(position - vec3(camera->Position.x, camera->Position.y, camera->Position.z));
	else
		look_vec = glm::normalize(position);

	float pitchRdns = glm::asin(look_vec.y);
	camera->Pitch = glm::degrees(pitchRdns);
	camera->Yaw = glm::degrees(atan2(look_vec.x, -1 * look_vec.z) - 3.141592 / 2);

	camera->Front.x = cos(glm::radians(camera->Pitch)) * cos(glm::radians(camera->Yaw));
	camera->Front.y = sin(glm::radians(camera->Pitch));
	camera->Front.z = cos(glm::radians(camera->Pitch)) * sin(glm::radians(camera->Yaw));
	camera->Front = glm::normalize(camera->Front);
}

Camera* camera_create(vec3 initialPosition, vec3 direction, bool isPosition = true) {
	auto camera = new Camera();
	camera->Position = initialPosition;
	camera_look_at(camera, direction, isPosition);
	return camera;
}

float* load_camera_settings(string path){
   ifstream reader(path);
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

void save_camera_settings_to_file(string path, vec3 position, vec3 direction) {
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
