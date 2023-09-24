#pragma once

#include "engine/core/core.h"

struct EPlayer;

enum NCameraType
{
	FREE_ROAM    = 0,
	THIRD_PERSON = 1
};

struct RCamera
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

	NCameraType type = FREE_ROAM;
	float orbital_angle = 0;
};

// camera array indexes
constexpr uint8 EditorCam = 0;
constexpr uint8 GameCam = 1;

struct RCameraManager
{
	DeclSingleton(RCameraManager);
	
	RCamera* GetCurrentCamera() { return current_camera; }
	RCamera* GetGameCamera() { return &game_camera; }
	RCamera* GetEditorCamera() { return &editor_camera; }
	
	void SwitchToEditorCamera() { current_camera = &editor_camera; }
	void SwitchToGameCamera() { current_camera = &game_camera; }
	
	void UpdateGameCamera(float viewport_width, float viewport_height, vec3 position);
	void UpdateEditorCamera(float viewport_width, float viewport_height, vec3 position);
	void CameraLookAt(RCamera* camera, vec3 ref, bool is_position);
	
	void SetCameraToFreeRoam();
	void SetCameraToThirdPerson();
	
	static void ComputeAnglesFromDirection(float& pitch, float& yaw, vec3 direction);
	static void ChangeCameraDirection(RCamera* camera, float yaw_offset, float pitch_offset);
	
private:
	RCamera game_camera;
	RCamera editor_camera;
	RCamera* current_camera = &editor_camera;
};