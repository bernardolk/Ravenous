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

EntityState get_entity_state(Entity* entity)
{
   EntityState state; 
   state.position = entity->position;
   state.scale = entity->scale;
   state.rotation = entity->rotation;
   state.entity = entity;
   return state;
}

struct UndoStack {
   u8 limit = 0;                             // index of last added item
   u8 pos = 0;                               // current index
   const static u8 capacity = 100;           // max items - 1 (pos = 0 is never assigned)
   EntityState stack[100];                   // actual stack
   bool full = false;                        // helps avoid writing out of stack mem boundaries

   void track(Entity* entity)
   {
      auto state = EntityState{
         entity, 
         entity->position, 
         entity->scale,
         entity->rotation
      };

     track(state);
   }

   void track(EntityState state)
   {
      if(full)
      {
         G_BUFFERS.rm_buffer->add("UNDO/REDO STACK FULL.", 800);
         return;
      }

      if(!_comp_state(state, check()))
      {
         stack[++pos] = state;
         limit = pos;
      }
      full = _is_buffer_full();
   }

   void undo()
   {
      auto state = _apply_undo();
      apply_state(state);
   }

   void redo()
   {
      auto state = _apply_redo();
      apply_state(state);
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

   // internal
   bool _is_buffer_full()
   {
      return limit + 1 == capacity;
   }

   // internal
   bool _comp_state(EntityState state1, EntityState state2)
   {
      return state1.entity == state2.entity
         && state1.position == state2.position
         && state1.scale == state2.scale
         && state1.rotation == state2.rotation;
   }
};