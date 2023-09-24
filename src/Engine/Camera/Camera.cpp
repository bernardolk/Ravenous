#include "engine/camera/camera.h"

RCameraManager::RCameraManager()
{
	game_camera = RCamera{};
	editor_camera = RCamera{};
}

void RCameraManager::SetCameraToFreeRoam()
{
	current_camera->type = FREE_ROAM;
}

void RCameraManager::SetCameraToThirdPerson()
{
	current_camera->type = THIRD_PERSON;
}

void RCameraManager::UpdateGameCamera(float viewport_width, float viewport_height, vec3 position)
{
	RCamera* camera = GetGameCamera();
	camera->mat_view = lookAt(camera->position, camera->position + camera->front, camera->up);
	camera->mat_projection = glm::perspective(
		glm::radians(camera->fov_y),
		viewport_width / viewport_height,
		camera->near_plane, camera->far_plane
	);

	camera->position = position;
}

void RCameraManager::UpdateEditorCamera(float viewport_width, float viewport_height, vec3 position)
{
	RCamera* camera = GetEditorCamera();

	camera->mat_view = lookAt(camera->position, camera->position + camera->front, camera->up);
	camera->mat_projection = glm::perspective(
		glm::radians(camera->fov_y),
		viewport_width / viewport_height,
		camera->near_plane, camera->far_plane
	);

	if (camera->type == THIRD_PERSON)
	{
		// camera->Position = player->entity_ptr->position;
		camera->position = position;
		camera->position.y += 1.75;

		if (camera->orbital_angle > 360.0f)
			camera->orbital_angle -= 360.0;
		if (camera->orbital_angle < -360.0f)
			camera->orbital_angle += 360.0;

		float distance = 3;
		camera->position.x += distance * cos(camera->orbital_angle);
		camera->position.z += distance * sin(camera->orbital_angle);
		CameraLookAt(camera, position, true);
	}
}

void RCameraManager::ChangeCameraDirection(RCamera* camera, float yaw_offset, float pitch_offset)
{
	float pitch, yaw;
	ComputeAnglesFromDirection(pitch, yaw, camera->front);

	pitch += pitch_offset;
	yaw += yaw_offset;

	// Unallows camera to perform a flip
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	// Make sure we don't overflow floats when camera is spinning indefinetely
	if (yaw > 360.0f)
		yaw -= 360.0f;
	if (yaw < -360.0f)
		yaw += 360.0f;

	camera->front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->front.y = sin(glm::radians(pitch));
	camera->front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->front = normalize(camera->front);
}


void RCameraManager::CameraLookAt(RCamera* camera, vec3 ref, bool is_position)
{
	// vec3 ref -> either a position or a direction vector (no need to be normalised)
	vec3 look_vec = ref;
	if (is_position)
		look_vec = ref - vec3(camera->position.x, camera->position.y, camera->position.z);
	look_vec = normalize(look_vec);

	float pitch = glm::degrees(glm::asin(look_vec.y));
	float yaw = glm::degrees(atan2(look_vec.x, -1 * look_vec.z) - PI / 2);

	camera->front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->front.y = sin(glm::radians(pitch));
	camera->front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->front = normalize(camera->front);
}

void RCameraManager::ComputeAnglesFromDirection(float& pitch, float& yaw, vec3 direction)
{
	pitch = glm::degrees(glm::asin(direction.y));
	yaw = glm::degrees(atan2(direction.x, -1 * direction.z) - PI / 2);
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	if (yaw > 360.0f)
		yaw -= 360.0f;
	if (yaw < -360.0f)
		yaw += 360.0f;
}
