
#include "engine/entities/Entity.h"
#include "engine/geometry/mesh.h"

/** Main update function, shouldn't run every frame. */
void EEntity::Update()
{
	/* Order here is very important */
	UpdateModelMatrix();
	UpdateCollider();
	UpdateBoundingBox();

	// if (IsInteractable())
	// 	UpdateTrigger();
}

void EEntity::UpdateCollider()
{
	// // empty collider
	// collider.vertices.clear();

	// multiplies model matrix to collision mesh
	for (int i = 0; i < collision_mesh->vertices.size(); i++)
	{
		collider.vertices[i] = vec3(mat_model * vec4(collision_mesh->vertices[i], 1.0));
	}
}

void EEntity::UpdateModelMatrix()
{
	glm::mat4 model = translate(Mat4Identity, position);
	model = rotate(model, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
	model = rotate(model, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);
	mat_model = model;
}

void EEntity::UpdateBoundingBox()
{
	// uses the collider to compute an AABB 
	bounding_box = collider.ComputeBoundingBox();
}

void EEntity::UpdateTrigger()
{
	auto centroid = bounding_box.GetCentroid();
	glm::mat4 model = translate(Mat4Identity, centroid);

	// to avoid elipsoids
	trigger_scale.z = trigger_scale.x;
	model = glm::scale(model, trigger_scale);

	trigger_mat_model = model;
}

void EEntity::RotateY(float angle)
{
	rotation.y += angle;
	rotation.y = static_cast<int>(rotation.y) % 360;
	if (rotation.y < 0)
		rotation.y = 360 + rotation.y;
}

mat4 EEntity::GetRotationMatrix()
{
	mat4 rotation_matrix;
	rotation_matrix = rotate(Mat4Identity, glm::radians(rotation.x), vec3(1.0f, 0.0f, 0.0f));
	rotation_matrix = rotate(rotation_matrix, glm::radians(rotation.y), vec3(0.0f, 1.0f, 0.0f));
	rotation_matrix = rotate(rotation_matrix, glm::radians(rotation.z), vec3(0.0f, 0.0f, 1.0f));
	
	return rotation_matrix;
}

RCollisionMesh EEntity::GetTriggerCollider()
{
	RCollisionMesh trigger_collider;

	// multiplies model matrix to collision mesh
	for (int i = 0; i < trigger->vertices.size(); i++)
		trigger_collider.vertices.push_back(vec3(vec4(trigger->vertices[i].position, 1) * trigger_mat_model));
	
	for (int i = 0; i < trigger->indices.size(); i++)
		trigger_collider.indices.push_back(trigger->indices[i]);


	return trigger_collider;
}

void EEntity::MakeInvisible()
{
	flags |= EntityFlags_InvisibleEntity;
}

void EEntity::MakeVisible()
{
	flags &= ~EntityFlags_InvisibleEntity;
}