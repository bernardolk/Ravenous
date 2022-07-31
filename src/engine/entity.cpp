#include <rvn_macros.h>
#include <iostream>
#include <vector>
#include <engine/core/rvn_types.h>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/quaternion.hpp>
#include <engine/vertex.h>
#include <string>
#include <map>
#include <engine/collision/primitives/triangle.h>
#include <engine/collision/primitives/bounding_box.h>
#include <engine/mesh.h>
#include <engine/logging.h>
#include <engine/entity.h>
   
void Entity::update()
{
   // @todo WE DON'T NEED TO RUN THIS EVERY TICK!
   // just run for entities that change it's entity state

   // order here is very important
   this->update_model_matrix();
   this->update_collider();
   this->update_bounding_box();

   if(this->is_interactable())
      this->update_trigger();  
}


void Entity::update_collider()
{
   // // empty collider
   // this->collider.vertices.clear();

   // multiplies model matrix to collision mesh
   for (int i = 0; i < this->collision_mesh->vertices.size(); i++)
      this->collider.vertices[i] = Vertex{this->collision_mesh->vertices[i] * this->matModel};
}


void Entity::update_model_matrix()
{
   glm::mat4 model = translate(mat4identity, this->position);
   model = rotate(model, glm::radians(this->rotation.x), vec3(1.0f, 0.0f, 0.0f));
   model = rotate(model, glm::radians(this->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   model = rotate(model, glm::radians(this->rotation.z), vec3(0.0f, 0.0f, 1.0f));
   model = glm::scale(model, this->scale);
   this->matModel = model;
}


void Entity::update_bounding_box()
{
   // uses the collider to compute an AABB 
   this->bounding_box = this->collider.compute_bounding_box();
}

void Entity::update_trigger()
{
   auto centroid = this->bounding_box.get_centroid();
   glm::mat4 model = translate(mat4identity, centroid);

   // to avoid elipsoids
   this->trigger_scale.z = this->trigger_scale.x;
   model = glm::scale(model, this->trigger_scale);

   this->trigger_matModel = model;
}


void Entity::rotate_y(float angle)
{
   this->rotation.y += angle;
   this->rotation.y = (int) this->rotation.y % 360;
   if(this->rotation.y < 0)
      this->rotation.y = 360 + this->rotation.y;
}

mat4 Entity::get_rotation_matrix()
{
   mat4 rotation_matrix;		
   rotation_matrix = rotate(mat4identity,    glm::radians(this->rotation.x), vec3(1.0f, 0.0f, 0.0f));
   rotation_matrix = rotate(rotation_matrix, glm::radians(this->rotation.y), vec3(0.0f, 1.0f, 0.0f));
   rotation_matrix = rotate(rotation_matrix, glm::radians(this->rotation.z), vec3(0.0f, 0.0f, 1.0f));
   return rotation_matrix;
}

Mesh Entity::get_trigger_collider()
{
   Mesh trigger_collider;

   // empty collider
   trigger_collider.vertices.clear();

   // multiplies model matrix to collision mesh
   for (int i = 0; i < this->trigger->vertices.size(); i++)
      trigger_collider.vertices.push_back(Vertex{this->trigger->vertices[i] * this->trigger_matModel});

   return trigger_collider;
}

bool Entity::is_interactable()
{
   return this->type == EntityType_Checkpoint || this->type == EntityType_TimerTrigger;
}


/* -------------------
   TimerTriggerData
-------------------- */

void TimerTriggerData::add_marking(Entity* entity, u32 time_checkpoint)
{
   For(size)
      if(markings[i] == nullptr)
      {
         markings[i]          = entity;
         time_checkpoints[i]  = time_checkpoint;
         return;
      }

   log(LOG_WARNING, "Max number of timer markings reached when trying to add entity as one to timer trigger entity.");
}

void TimerTriggerData::delete_marking(int i)
{
   markings[i]             = nullptr;
   notification_mask[i]    = false;
   time_checkpoints[i]     = 0;
}