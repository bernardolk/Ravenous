#include "engine/entities/Entity.h"
#include "engine/geometry/mesh.h"

/* ==================================================================
 * Update:
 *  Main update function, shouldn't need to run every frame.
/* ================================================================== */
void EEntity::Update()
{
	/* Order here is very important */
	UpdateModelMatrix();
	UpdateCollider();
	UpdateBoundingBox();
}

void EEntity::UpdateCollider()
{
	// // empty collider
	// collider.vertices.clear();

	if (Collider.Vertices.size() == 0)
	{
		Collider = *CollisionMesh;
	}

	// multiplies model matrix to collision mesh
	for (int i = 0; i < CollisionMesh->Vertices.size(); i++)
	{
		Collider.Vertices[i] = vec3(MatModel * vec4(CollisionMesh->Vertices[i], 1.0));
	}
}

void EEntity::UpdateModelMatrix()
{
	glm::mat4 Model = translate(Mat4Identity, Position);
	Model = rotate(Model, glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	Model = rotate(Model, glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f));
	Model = glm::scale(Model, Scale);
	MatModel = Model;
}

void EEntity::UpdateBoundingBox()
{
	// uses the collider to compute an AABB 
	BoundingBox = Collider.ComputeBoundingBox();
}

void EEntity::UpdateTrigger()
{
	auto Centroid = BoundingBox.GetCentroid();
	glm::mat4 Model = translate(Mat4Identity, Centroid);

	// to avoid elipsoids
	TriggerScale.z = TriggerScale.x;
	Model = glm::scale(Model, TriggerScale);

	TriggerMatModel = Model;
}

void EEntity::RotateY(float Angle)
{
	Rotation.y += Angle;
	Rotation.y = static_cast<int>(Rotation.y) % 360;
	if (Rotation.y < 0)
		Rotation.y = 360 + Rotation.y;
}

mat4 EEntity::GetRotationMatrix()
{
	mat4 RotationMatrix;
	RotationMatrix = rotate(Mat4Identity, glm::radians(Rotation.x), vec3(1.0f, 0.0f, 0.0f));
	RotationMatrix = rotate(RotationMatrix, glm::radians(Rotation.y), vec3(0.0f, 1.0f, 0.0f));
	RotationMatrix = rotate(RotationMatrix, glm::radians(Rotation.z), vec3(0.0f, 0.0f, 1.0f));

	return RotationMatrix;
}

RCollisionMesh EEntity::GetTriggerCollider()
{
	RCollisionMesh TriggerCollider;

	// multiplies model matrix to collision mesh
	for (int i = 0; i < Trigger->Vertices.size(); i++)
		TriggerCollider.Vertices.push_back(vec3(vec4(Trigger->Vertices[i].Position, 1) * TriggerMatModel));

	for (int I = 0; I < Trigger->Indices.size(); I++)
		TriggerCollider.Indices.push_back(Trigger->Indices[I]);


	return TriggerCollider;
}

void EEntity::MakeInvisible()
{
	Flags |= EntityFlags_InvisibleEntity;
}

void EEntity::MakeVisible()
{
	Flags &= ~EntityFlags_InvisibleEntity;
}

vec3 EEntity::GetForwardVector() const
{
	return normalize(Rotation);
}
