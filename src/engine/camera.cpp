
#include <engine/camera.h>

void set_camera_to_free_roam(Camera* camera)
{
	camera->type = FREE_ROAM;
}

void set_camera_to_third_person(Camera* camera)
{
	camera->type = THIRD_PERSON;
}

void camera_update_game(Camera* camera, float viewport_width, float viewport_height, vec3 position)
{
	camera->mat_view = lookAt(camera->position, camera->position + camera->front, camera->up);
	camera->mat_projection = glm::perspective(
	glm::radians(camera->fov_y),
	viewport_width / viewport_height,
	camera->near_plane, camera->far_plane
	);

	camera->position = position;
}

void camera_update_editor(Camera* camera, float viewport_width, float viewport_height, vec3 position)
{
	camera->mat_view = lookAt(camera->position, camera->position + camera->front, camera->up);
	camera->mat_projection = glm::perspective(
	glm::radians(camera->fov_y),
	viewport_width / viewport_height,
	camera->near_plane, camera->far_plane
	);

	if(camera->type == THIRD_PERSON)
	{
		// camera->Position = player->entity_ptr->position;
		camera->position = position;
		camera->position.y += 1.75;

		if(camera->orbital_angle > 360.0f)
			camera->orbital_angle -= 360.0;
		if(camera->orbital_angle < -360.0f)
			camera->orbital_angle += 360.0;

		float distance = 3;
		camera->position.x += distance * cos(camera->orbital_angle);
		camera->position.z += distance * sin(camera->orbital_angle);
		camera_look_at(camera, position, true);
	}
}

void camera_change_direction(Camera* camera, float yaw_offset, float pitch_offset)
{
	float pitch, yaw;
	compute_angles_from_direction(pitch, yaw, camera->front);

	pitch += pitch_offset;
	yaw += yaw_offset;

	// Unallows camera to perform a flip
	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;

	// Make sure we don't overflow floats when camera is spinning indefinetely
	if(yaw > 360.0f)
		yaw -= 360.0f;
	if(yaw < -360.0f)
		yaw += 360.0f;

	camera->front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->front.y = sin(glm::radians(pitch));
	camera->front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->front = normalize(camera->front);
}


void camera_look_at(Camera* camera, vec3 ref, bool is_position)
{
	// vec3 ref -> either a position or a direction vector (no need to be normalised)
	vec3 look_vec = ref;
	if(is_position)
		look_vec = ref - vec3(camera->position.x, camera->position.y, camera->position.z);
	look_vec = normalize(look_vec);

	float pitch = glm::degrees(glm::asin(look_vec.y));
	float yaw = glm::degrees(atan2(look_vec.x, -1 * look_vec.z) - PI / 2);

	camera->front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
	camera->front.y = sin(glm::radians(pitch));
	camera->front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
	camera->front = normalize(camera->front);
}

void compute_angles_from_direction(float& pitch, float& yaw, vec3 direction)
{
	pitch = glm::degrees(glm::asin(direction.y));
	yaw = glm::degrees(atan2(direction.x, -1 * direction.z) - PI / 2);
	if(pitch > 89.0f)
		pitch = 89.0f;
	if(pitch < -89.0f)
		pitch = -89.0f;
	if(yaw > 360.0f)
		yaw -= 360.0f;
	if(yaw < -360.0f)
		yaw += 360.0f;
}
