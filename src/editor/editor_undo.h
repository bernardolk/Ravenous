struct EntityState {
   Entity* entity = nullptr;
   vec3 position;
   vec3 scale;
   vec3 rotation;
};

void apply_state(EntityState state)
{
   if(state.entity == nullptr)
      return;

   state.entity->position = state.position;
   state.entity->scale = state.scale;
   state.entity->rotate_y(state.rotation.y - state.entity->rotation.y);
}

struct UndoStack {
   u8 limit = 0;
   u8 pos = 0;
   EntityState stack[100];

   void track(Entity* entity)
   {
      auto state = EntityState{
         entity, 
         entity->position, 
         entity->scale,
         entity->rotation
      };
      if(!_comp_state(state, check()))
      {
         stack[++pos] = state;
         limit = pos;
      }
      G_BUFFERS.rm_buffer->add("pos = " + to_string(pos), 500);
   }

   void undo()
   {
      auto state = _apply_undo();
      apply_state(state);
      G_BUFFERS.rm_buffer->add("pos = " + to_string(pos), 500);
   }

   void redo()
   {
      auto state = _apply_redo();
      apply_state(state);
      G_BUFFERS.rm_buffer->add("pos = " + to_string(pos), 500);
   }

   EntityState check()
   {
      if(pos > 0)
         return stack[pos];
      else
         return EntityState{};
   }

   // internal
   EntityState _apply_undo()
   {
      if(pos > 1)
         return stack[--pos];
      if(pos == 1)
         return stack[pos];
      else
         return EntityState{};
   }

   // internal
   EntityState _apply_redo()
   {
      if(pos < limit)
         return stack[++pos];
      else
         return EntityState{};
   }

   bool _comp_state(EntityState state1, EntityState state2)
   {
      return state1.entity == state2.entity
         && state1.position == state2.position
         && state1.scale == state2.scale
         && state1.rotation == state2.rotation;
   }
};

EntityState get_entity_state(Entity* entity)
{
   EntityState state; 
   state.position = entity->position;
   state.scale = entity->scale;
   state.rotation = entity->rotation;
   state.entity = entity;
   return state;
}