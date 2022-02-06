
// Catalogue
struct EntityAnimation;
std::map<u32, EntityAnimation> Animation_Catalogue;

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
   std::string                description = "";
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
      
      float speed;
      // Update entity position
      if(kf->flags & EntityAnimKfFlags_ChangePosition)
      {
         //x
         speed = (kf->final_position.x - kf->starting_position.x) / kf->duration;
         entity->position.x += speed * frame_duration_ms;
         //y
         speed = (kf->final_position.y - kf->starting_position.y) / kf->duration;
         entity->position.y += speed * frame_duration_ms;
         //z
         speed = (kf->final_position.z - kf->starting_position.z) / kf->duration;
         entity->position.z += speed * frame_duration_ms;
      }
      
      // Update entity rotation
      if(kf->flags & EntityAnimKfFlags_ChangeRotation)
      {
         //x
         speed = (kf->final_rotation.x - kf->starting_rotation.x) / kf->duration;
         entity->rotation.x += speed * frame_duration_ms;
         //y
         speed = (kf->final_rotation.y - kf->starting_rotation.y) / kf->duration;
         entity->rotation.y += speed * frame_duration_ms;
         //z
         speed = (kf->final_rotation.z - kf->starting_rotation.z) / kf->duration;
         entity->rotation.z += speed * frame_duration_ms;
      }

      // Update entity scale
      if(kf->flags & EntityAnimKfFlags_ChangeScale)
      {
         //x
         speed = (kf->final_scale.x - kf->starting_scale.x) / kf->duration;
         entity->scale.x += speed * frame_duration_ms;
         //y
         speed = (kf->final_scale.y - kf->starting_scale.y) / kf->duration;
         entity->scale.y += speed * frame_duration_ms;
         //z
         speed = (kf->final_scale.z - kf->starting_scale.z) / kf->duration;
         entity->scale.z += speed * frame_duration_ms;
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

   void start_animation(Entity* entity, EntityAnimation* animation)
   {
      // makes a copy of the animation to the Entity_Animations buffer

      auto i = _find_slot();
      auto _animation = &animations[i];

      *_animation                 = *animation;
      _animation->active          = true;
      _animation->entity          = entity;
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


void AN_create_hardcoded_animations()
{

   // >> VERTICAL DOOR 
   {
      // > SLIDING UP
      {
         auto kf                 = EntityAnimationKeyframe();
         kf.duration             = 2000;
         kf.starting_scale       = vec3{0.24, 1.6, 2.350};
         kf.final_scale          = kf.starting_scale;
         kf.final_scale.z        = 0.2; 
         kf.flags                |= EntityAnimKfFlags_ChangeScale;

         auto anim               = EntityAnimation();
         anim.description        = "vertical_door_slide_up";
         anim.keyframes_count    = 1;
         anim.keyframes[0]       = kf;

         Animation_Catalogue.insert({1, anim});
      }

      // > SLIDING DOWN
      {
         auto kf                 = EntityAnimationKeyframe();
         kf.duration             = 2000;
         kf.starting_scale       = vec3{0.24, 1.6, 0.2};
         kf.final_scale          = kf.starting_scale;
         kf.final_scale.z        = 2.350;
         kf.flags                |= EntityAnimKfFlags_ChangeScale;

         auto anim               = EntityAnimation();
         anim.description        = "vertical_door_slide_down";
         anim.keyframes_count    = 1;
         anim.keyframes[0]       = kf;

         Animation_Catalogue.insert({2, anim});
      }
   }
}