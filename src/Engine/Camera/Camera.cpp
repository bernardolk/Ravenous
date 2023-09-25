#include "engine/Camera/Camera.h"

RCameraManager::RCameraManager()
{
	GameCamera = RCamera{};
	EditorCamera = RCamera{};
}

void RCameraManager::SetCameraToFreeRoam()
{
	CurrentCamera->Type = FreeRoam;
}

void RCameraManager::SetCameraToThirdPerson()
{
	CurrentCamera->Type = ThirdPerson;
}

void RCameraManager::UpdateGameCamera(float ViewportWidth, float ViewportHeight, vec3 Position)
{
	RCamera* Camera = GetGameCamera();
	Camera->MatView = lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
	Camera->MatProjection = glm::perspective(
		glm::radians(Camera->FovY),
		ViewportWidth / ViewportHeight,
		Camera->NearPlane, Camera->FarPlane
	);

	Camera->Position = Position;
}

void RCameraManager::UpdateEditorCamera(float ViewportWidth, float ViewportHeight, vec3 Position)
{
	RCamera* Camera = GetEditorCamera();

	Camera->MatView = lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
	Camera->MatProjection = glm::perspective(
		glm::radians(Camera->FovY),
		ViewportWidth / ViewportHeight,
		Camera->NearPlane, Camera->FarPlane
	);

	if (Camera->Type == ThirdPerson)
	{
		// Camera->Position = player->entity_ptr->position;
		Camera->Position = Position;
		Camera->Position.y += 1.75;

		if (Camera->OrbitalAngle > 360.0f)
			Camera->OrbitalAngle -= 360.0;
		if (Camera->OrbitalAngle < -360.0f)
			Camera->OrbitalAngle += 360.0;

		float Distance = 3;
		Camera->Position.x += Distance * cos(Camera->OrbitalAngle);
		Camera->Position.z += Distance * sin(Camera->OrbitalAngle);
		CameraLookAt(Camera, Position, true);
	}
}

void RCameraManager::ChangeCameraDirection(RCamera* Camera, float YawOffset, float PitchOffset)
{
	float Pitch, Yaw;
	ComputeAnglesFromDirection(Pitch, Yaw, Camera->Front);

	Pitch += PitchOffset;
	Yaw += YawOffset;

	// Unallows Camera to perform a flip
	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	// Make sure we don't overflow floats when Camera is spinning indefinetely
	if (Yaw > 360.0f)
		Yaw -= 360.0f;
	if (Yaw < -360.0f)
		Yaw += 360.0f;

	Camera->Front.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
	Camera->Front.y = sin(glm::radians(Pitch));
	Camera->Front.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
	Camera->Front = normalize(Camera->Front);
}


void RCameraManager::CameraLookAt(RCamera* Camera, vec3 Reference, bool IsPosition)
{
	// vec3 ref -> either a position or a direction vector (no need to be normalised)
	vec3 LookVector = Reference;
	if (IsPosition)
		LookVector = Reference - vec3(Camera->Position.x, Camera->Position.y, Camera->Position.z);
	LookVector = glm::normalize(LookVector);

	float Pitch = glm::degrees(glm::asin(LookVector.y));
	float Yaw = glm::degrees(atan2(LookVector.x, -1 * LookVector.z) - PI / 2);

	Camera->Front.x = cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
	Camera->Front.y = sin(glm::radians(Pitch));
	Camera->Front.z = cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));
	Camera->Front = normalize(Camera->Front);
}

void RCameraManager::ComputeAnglesFromDirection(float& Pitch, float& Yaw, vec3 Direction)
{
	Pitch = glm::degrees(glm::asin(Direction.y));
	Yaw = glm::degrees(atan2(Direction.x, -1 * Direction.z) - PI / 2);
	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;
	if (Yaw > 360.0f)
		Yaw -= 360.0f;
	if (Yaw < -360.0f)
		Yaw += 360.0f;
}
