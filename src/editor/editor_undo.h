struct EntityState {
   Entity* entity = nullptr;
   unsigned int id;
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
   state.id = entity->id;
   return state;
}

bool compare_state(EntityState s1, EntityState s2)
{
   return s1.position == s2.position &&
      s1.scale == s2.scale &&
      s1.rotation == s2.rotation;
}

struct DeletedEntityLog {
   u8 size = 0;
   const static u8 capacity = 100;
   int entity_ids[capacity];

   void add(Entity* entity) 
   {
      if(size + 1 == capacity)
      {
         G_BUFFERS.rm_buffer->add("DeletedEntityLog is FULL!", 3000);
         return;
      }

      entity_ids[size++] = entity->id;
   }
};

struct UndoStack {
   u8 limit = 0;                             // index of last added item
   u8 pos = 0;                               // current index
   const static u8 capacity = 100;           // max items - 1 (pos = 0 is never assigned)
   EntityState stack[100];                   // actual stack
   DeletedEntityLog deletion_log;            // stores ids of entities that have been deleted
   bool full = false;                        // helps avoid writing out of stack mem boundaries

   void track(Entity* entity)
   {
      auto state = EntityState{
         entity,
         entity->id, 
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
      if(pos == 0)
         return;

      // gets a valid state to undo
      EntityState state;
      do {
         state = _get_state_and_move_back();
         if(pos == 1 && !_is_state_valid(state))
            return;
      } while(!_is_state_valid(state));

      apply_state(state);
   }

   void redo()
   {
      if(pos == 0)
         return;
         
      // gets a valid state to redo
      EntityState state;
      do {
         state = _get_state_and_move_up();
         if(pos == limit && !_is_state_valid(state))
            return;
      } while(!_is_state_valid(state));

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
   EntityState _get_state_and_move_back()
   {
      if(pos > 1)
         return stack[--pos];
      if(pos == 1)
         return stack[pos];
      else
         return EntityState{};
   }

   // internal
   EntityState _get_state_and_move_up()
   {
      if(pos < limit)
         return stack[++pos];
      if(pos == limit)
         return stack[pos];
      else
         return EntityState{};
   }

   // internal
   bool _is_buffer_full()
   {
      return limit + 1 == capacity;
   }

   // internal
   bool _is_state_valid(EntityState state)
   {
      // if entity was deleted, it isnt valid
      for(int i = 0; i < deletion_log.size; i++)
         if(deletion_log.entity_ids[i] == state.id)
            return false;
      
      // if entity current state is equal to state in stack
      // then is not valid for undo also
      return !_comp_state(get_entity_state(state.entity), state);
   }

   // internal
   bool _comp_state(EntityState state1, EntityState state2)
   {
      return state1.id == state2.id
         && state1.position == state2.position
         && state1.scale == state2.scale
         && state1.rotation == state2.rotation;
   }
};