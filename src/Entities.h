#pragma once

struct GlobalEntityInfo {
   u32 entity_counter = 0;
};

enum CollisionGeometryEnum {
   COLLISION_ALIGNED_CYLINDER,
   COLLISION_ALIGNED_BOX
};

struct Entity {
	unsigned int index;
	unsigned int id;
	Model* model;
	Shader* shader;
   void* collision_geometry_ptr;
   CollisionGeometryEnum collision_geometry_type;
	glm::vec3 position;
   glm::vec3 velocity;
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::mat4 matModel = mat4identity;
};

enum PlayerStateEnum {
   PLAYER_STATE_FALLING,
   PLAYER_STATE_STANDING,
   PLAYER_STATE_WALKING,
   PLAYER_STATE_RUNNING,
   PLAYER_STATE_SPRINTING,
   PLAYER_STATE_JUMPING,
   PLAYER_STATE_SLIDING,
   PLAYER_STATE_GRABBING
};

struct Player {
   Entity* entity_ptr,
   PlayerStateEnum player_state
};

struct SpotLight {
	unsigned int id;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
	float innercone;
	float outercone;
	float intensity_constant = 0.02f;
	float intensity_linear = 1.0f;
	float intensity_quadratic = 0.032f;
};

struct PointLight {
	unsigned int id;
	glm::vec3 position = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
	glm::vec3 diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 ambient = glm::vec3(0.01f, 0.01f, 0.01f);
	float intensity_constant = 1.0f;
	float intensity_linear = 0.5f;
	float intensity_quadratic = 0.1f;
};

struct DirectionalLight {
	unsigned int id;
	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 ambient;
};

struct Scene {
	std::vector<Entity> entities;
	std::vector<SpotLight> spotLights;
	std::vector<DirectionalLight> directionalLights;
	std::vector<PointLight> pointLights;
	//vector<LightEntity> lights;
};


vector<Vertex> quad_vertex_vec = {
	Vertex{glm::vec3(0.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(1.0f, 0.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 0.0f)},
	Vertex{glm::vec3(1.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(0.0f, 1.0f, 0.0f),glm::vec3(0.0f, 0.0f, 1.0f),glm::vec2(0.0f, 1.0f)}
};

vector<u32> quad_vertex_indices = { 0, 1, 2, 2, 3, 0 };