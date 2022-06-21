#pragma once

struct Player;

enum CameraType {
   FREE_ROAM      = 0,
   THIRD_PERSON   = 1
};

struct Camera {
	vec3 Position     = vec3(0.0f);
	vec3 Front        = vec3(1.0f, 0.0f, 0.0f);
	vec3 Up           = vec3(0.0f, 1.0f, 0.0f);

	float Acceleration   = 3.5f;
	float FOVy           = 45.0f;
	float FarPlane       = 300.0f;
	float NearPlane      = 0.1f;
	float Sensitivity    = 0.1f;

	glm::mat4 View4x4;
	glm::mat4 Projection4x4;
   
   CameraType type      = FREE_ROAM;
   float orbital_angle  = 0;
};

// camera array indexes
extern const u8 EDITOR_CAM;
extern const u8 FPS_CAM; 

void camera_update_game                (Camera* camera, float viewportWidth, float viewportHeight, vec3 position);
void camera_update_editor              (Camera* camera, float viewportWidth, float viewportHeight, vec3 position);
void camera_change_direction           (Camera* camera, float yawOffset, float pitchOffset);
void camera_look_at                    (Camera* camera, vec3 ref, bool isPosition);
void set_camera_to_free_roam           (Camera* camera);
void set_camera_to_third_person        (Camera* camera);
void compute_angles_from_direction     (float& pitch, float& yaw, vec3 direction);