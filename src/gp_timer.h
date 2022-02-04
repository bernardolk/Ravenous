enum TimerType {
   TimerType_Simple           = 0,
   TimerType_TimeAttackDoor   = 1,
};

struct Timer {
   TimerType   type              = TimerType_Simple;
   Entity*     target            = nullptr;
   Entity*     trigger           = nullptr;
   bool        active            = false;
   float       remaining_time    = 0;
   float       elapsed_time      = 0;

   void start(Entity* target, Entity* trigger, float duration)
   {
      this->target            = target;
      this->trigger           = trigger;
      remaining_time    = duration;
      elapsed_time      = 0;
      active            = true;

      // @todo: could be any kind of time_attack_door, but ok.
      if(target->timer_target_type == EntityTimerTargetType_VerticalSlidingDoor)
         _start_time_attack_door_timer();
   }

   void stop()
   {
      target            = nullptr;
      trigger           = nullptr;
      active            = false;
      remaining_time    = 0;
      elapsed_time      = 0;
      type              = TimerType_Simple;
   }

   bool update()
   {
      /* Returns whether the timer is still active */
      remaining_time -= G_FRAME_INFO.duration;
      elapsed_time   += G_FRAME_INFO.duration;

      if(type == TimerType_TimeAttackDoor)
         _update_time_attack_door_timer();

      if(remaining_time <= 0)
         return false;
      else
         return true;
   }


   void _start_time_attack_door_timer()
   {
      // not sure this should live here and not in the entity.h file. But ok.
      auto data = &trigger->time_attack_trigger_data;
      For(data->size)
      {
         data->notification_mask[i] = false;
         auto entity = data->markings[i];

         // turn every marking on
         if(entity != nullptr)
            entity->color = entity->time_attack_marking_data.color_on;
      }
   }


   void _update_time_attack_door_timer()
   {
      auto data = &trigger->time_attack_trigger_data;
      For(data->size)
      {
         auto entity = data->markings[i];
         if(entity != nullptr && data->notification_mask[i] == false && data->time_checkpoints[i] <= elapsed_time)
         {
            // turn the marking off
            entity->color = entity->time_attack_marking_data.color_off;
            data->notification_mask[i] = true;
         }
      }
   }
};