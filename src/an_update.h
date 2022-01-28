
const static u32 AN_MAX_ENTITY_ANIMATION_KEYFRAMES = 16;

enum EntityAnimationKeyframeFlags {
   EntityAnimKfFlags_ChangePosition   = 1 << 0,
   EntityAnimKfFlags_ChangeRotation   = 1 << 1,
   EntityAnimKfFlags_ChangeScale      = 1 << 2,
};

struct EntityAnimationKeyframe {
   u32      duration;                  // expressed in milliseconds
   vec3     final_position;
   vec3     final_rotation;
   vec3     final_scale;
   vec3     starting_position;
   vec3     starting_rotation;
   vec3     starting_scale;

   u32 flags;
};

struct EntityAnimation {
   bool                       active = false;
   Entity*                    entity = nullptr;
   u32                        keyframes_count = 0;
   EntityAnimationKeyframe    keyframes[AN_MAX_ENTITY_ANIMATION_KEYFRAMES];

   float                      runtime = 0;               // expressed in milliseconds
   u32                        current_keyframe = 0;
   float                      keyframe_runtime = 0;

   void update()
   {
      /* executes current keyframe in entity and updates runtimes, turns animation inactive once it ends. */

      float frame_duration_ms = G_FRAME_INFO.duration * 1000;

      auto kf = &keyframes[current_keyframe];

      keyframe_runtime     += frame_duration_ms;
      runtime              += frame_duration_ms;

      // --------------------
      // > Perform animation
      // --------------------

      

      // Update entity position
      if(kf->flags & EntityAnimKfFlags_ChangePosition)
      {

      }
      
      // Update entity rotation
      if(kf->flags & EntityAnimKfFlags_ChangeRotation)
      {

      }

      // Update entity scale
      if(kf->flags & EntityAnimKfFlags_ChangeScale)
      {
         if(kf->final_scale.x > 0)
         {
            float speed = (kf->final_scale.x - kf->starting_scale.x) / kf->duration;
            entity->scale.x += speed * frame_duration_ms;
         }
         if(kf->final_scale.y > 0)
         {
            float speed = (kf->final_scale.y - kf->starting_scale.y) / kf->duration;
            entity->scale.y += speed * frame_duration_ms;
         }
         if(kf->final_scale.z > 0)
         {
            float speed = (kf->final_scale.z - kf->starting_scale.z) / kf->duration;
            entity->scale.z += speed * frame_duration_ms;
         }
      }

      entity->update();

      // updates keyframe if necessary
      if(keyframe_runtime >= kf->duration)
      {
         current_keyframe++;
         if(current_keyframe > keyframes_count)
         {
            active = false;
            return;
         }
      }
   }
};

struct EntityAnimationBuffer {
   const static size_t     animation_buffer_array_size = 16;
   EntityAnimation         animations[animation_buffer_array_size];

   size_t _find_slot()
   {
      For(animation_buffer_array_size)
      {
         if(!animations[i].active)
         {
            // reset slot
            animations[i] = EntityAnimation();
            return i;
         }
      }

      Quit_fatal("EntityAnimationBuffer overflow. Too many animations.");
      return 0;
   }

   void start_animation(Entity* entity, EntityAnimationKeyframe* keyframes, u32 keyframes_count)
   {
      //@todo: later in the future we might want to have an Animation_Catalogue and then reference animations
      //       by id. Then we can search the catalogue and place a pointer to the keyframe array into the anim
      //       buffer or something like that.

      auto i = _find_slot();
      auto animation = &animations[i];

      animation->active          = true;
      animation->entity          = entity;
      animation->keyframes_count = keyframes_count;
      
      // copy keyframes to buffer
      For(keyframes_count)
         animation->keyframes[i] = keyframes[i];
   }

   void update_animations()
   {
      For(Entity_Animations.animation_buffer_array_size)
      {  
         auto anim = &Entity_Animations.animations[i];
         
         if(anim->active)
            anim->update();
      }
   }

} Entity_Animations;



void AN_door_sliding_animation(Entity* entity)
{
  

   return;
}