#include <string>
#include <map>
#include <engine/core/rvn_types.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/compatibility.hpp>
#include <engine/camera.h>

extern const u8 EDITOR_CAM     = 0;
extern const u8 FPS_CAM        = 1; 

void set_camera_to_free_roam(Camera* camera)
{
   camera->type = FREE_ROAM;
}

void set_camera_to_third_person(Camera* camera)
{
   camera->type = THIRD_PERSON;
}

void camera_update_game(Camera* camera, float viewportWidth, float viewportHeight, vec3 position)
{
   camera->View4x4 = glm::lookAt(camera->Position, camera->Position + camera->Front, camera->Up);
	camera->Projection4x4 = glm::perspective(
      glm::radians(camera->FOVy), 
      viewportWidth / viewportHeight, 
      camera->NearPlane, camera->FarPlane
   );
   
   camera->Position = position;
}

void camera_update_editor(Camera* camera, float viewportWidth, float viewportHeight, vec3 position)
{
   camera->View4x4 = glm::lookAt(camera->Position, camera->Position + camera->Front, camera->Up);
	camera->Projection4x4 = glm::perspective(
      glm::radians(camera->FOVy), 
      viewportWidth / viewportHeight, 
      camera->NearPlane, camera->FarPlane
   );
   
   if(camera->type == THIRD_PERSON)
   {
      // camera->Position = player->entity_ptr->position;
      camera->Position    = position;
      camera->Position.y += 1.75;

      if (camera->orbital_angle > 360.0f)
         camera->orbital_angle -= 360.0;
      if (camera->orbital_angle < -360.0f)
         camera->orbital_angle += 360.0;

      float distance = 3;
      camera->Position.x += distance * cos(camera->orbital_angle);
      camera->Position.z += distance * sin(camera->orbital_angle); 
      camera_look_at(camera, position, true);
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