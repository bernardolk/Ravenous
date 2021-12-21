EntityState get_entity_state(Entity* entity)
{
   EntityState state; 
   state.position = entity->position;
   state.scale = entity->scale;
   state.rotation = entity->rotation;
   state.entity = entity;
   state.id = entity->id;
   return state;
}

void apply_state(EntityState state)
{
   if(state.entity == nullptr)
      return;

   state.entity->position = state.position;
   state.entity->scale = state.scale;
   state.entity->rotate_y(state.rotation.y - state.entity->rotation.y);
   state.entity->update();
}

mat4 mat_model_from_entity_state(EntityState state)
{
   glm::mat4 model = translate(mat4identity, state.position);
   model = rotate(model, glm::radians(state.rotation.x), vec3(1.0f, 0.0f, 0.0f));
   model = rotate(model, glm::radians(state.rotation.y), vec3(0.0f, 1.0f, 0.0f));
   model = rotate(model, glm::radians(state.rotation.z), vec3(0.0f, 0.0f, 1.0f));
   model = glm::scale(model, state.scale);
   return model;
}