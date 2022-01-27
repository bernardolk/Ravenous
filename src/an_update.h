
u32 AN_MAX_ENTITY_ANIMATION_KEYFRAMES = 16;

struct EntityAnimationKeyframe {
   float    duration;
   float    speed;
   vec3     direction;
   vec3     final_position;
   vec3     final_scale;
};

struct EntityAnimation {
   bool                       active = false;
   Entity*                    entity = nullptr;
   u32                        keyframes_count = 0;
   EntityAnimationKeyframe    keyframes[AN_MAX_ENTITY_ANIMATION_KEYFRAMES];
};

struct EntityAnimationBuffer {
   const static size_t     animation_buffer_array_size = 16;
   EntityAnimation         animations[animation_buffer_array_size];

   size_t find_slot()
   {
      For(animation_buffer_array_size)
      {
         if(!animations[i].active)
         {
            // reset slot
            animation[i] = EntityAnimation();
            return i;
         }
      }

      Quit_fatal("EntityAnimationBuffer overflow. Too many animations.");
   }

} Entity_Animations;


void AN_update_animations(Player* player)
{
   AN_animate_player(Player* player);

   AN_update_animation_buffer();

}

void 


void AN_start_animation(Entity* entity, EntityAnimationKeyframes* keyframes, u32 keyframes_count)
{
   //@todo: later in the future we might want to have an Animation_Catalogue and then reference animations
   //       by id. Then we can search the catalogue and place a pointer to the keyframe array into the anim
   //       buffer or something like that.

   auto i = Entity_Animations.find_slot();
   auto animation = &Entity_Animation.animations[i]

   animation->active = true;
   animation->entity = entity;
   animation->keyframes_count = keyframes_count;
   
   // copy keyframes to buffer
   For(keyframes_count)
      animation->keyframes[i] = keyframes[i];
}