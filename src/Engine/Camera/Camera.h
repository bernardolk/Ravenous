#pragma once

#include "engine/core/core.h"

struct EPlayer;

enum NCameraType
{
	FreeRoam    = 0,
	ThirdPerson = 1
};

struct RCamera
{
	vec3 Position = vec3(0.0f);
	vec3 Front = vec3(1.0f, 0.0f, 0.0f);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);

	float Acceleration = 3.5f;
	float FovY = 45.0f;
	float FarPlane = 300.0f;
	float NearPlane = 0.1f;
	float Sensitivity = 0.1f;

	glm::mat4 MatView;
	glm::mat4 MatProjection;

	NCameraType Type = FreeRoam;
	float OrbitalAngle = 0;
};

// camera array indexes
constexpr uint8 EditorCam = 0;
constexpr uint8 GameCam = 1;

struct RCameraManager
{
	static RCameraManager* Get() 
	{ 
		static RCameraManager Instance{};
		return &Instance;
	}

	RCamera* GetCurrentCamera() { return CurrentCamera; }
	RCamera* GetGameCamera() { return &GameCamera; }
	RCamera* GetEditorCamera() { return &EditorCamera; }

	void SwitchToEditorCamera() { CurrentCamera = &EditorCamera; }
	void SwitchToGameCamera() { CurrentCamera = &GameCamera; }

	void UpdateGameCamera(float ViewportWidth, float ViewportHeight, vec3 Position);
	void UpdateEditorCamera(float ViewportWidth, float ViewportHeight, vec3 Position);
	void CameraLookAt(RCamera* Camera, vec3 Ref, bool IsPosition);

	void SetCameraToFreeRoam();
	void SetCameraToThirdPerson();

	static void ComputeAnglesFromDirection(float& Pitch, float& Yaw, vec3 Direction);
	static void ChangeCameraDirection(RCamera* Camera, float YawOffset, float PitchOffset);

private:
	RCamera GameCamera{};
	RCamera EditorCamera{};
	RCamera* CurrentCamera = &EditorCamera;
};
