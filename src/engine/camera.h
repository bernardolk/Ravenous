#pragma once

#include "engine/core/core.h"

struct Player;

enum CameraType
{
	FREE_ROAM    = 0,
	THIRD_PERSON = 1
};

struct Camera
{
	vec3 position = vec3(0.0f);
	vec3 front = vec3(1.0f, 0.0f, 0.0f);
	vec3 up = vec3(0.0f, 1.0f, 0.0f);

	float acceleration = 3.5f;
	float fov_y = 45.0f;
	float far_plane = 300.0f;
	float near_plane = 0.1f;
	float sensitivity = 0.1f;

	glm::mat4 mat_view;
	glm::mat4 mat_projection;

	CameraType type = FREE_ROAM;
	float orbital_angle = 0;
};

// camera array indexes
constexpr u8 EditorCam = 0;
constexpr u8 GameCam = 1;

void UpdateGameCamera(Camera* camera, float viewport_width, float viewport_height, vec3 position);
void UpdateEditorCamera(Camera* camera, float viewport_width, float viewport_height, vec3 position);
void ChangeCameraDirection(Camera* camera, float yaw_offset, float pitch_offset);
void CameraLookAt(Camera* camera, vec3 ref, bool is_position);
void SetCameraToFreeRoam(Camera* camera);
void SetCameraToThirdPerson(Camera* camera);
void ComputeAnglesFromDirection(float& pitch, float& yaw, vec3 direction);
