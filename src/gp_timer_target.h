// enum TimerTargetFlags {
//    TimerTargetFlag_Inactive   = 0,
//    TimerTargetFlag_Starting   = 1,
//    TimerTargetFlag_Active     = 2,
//    TimerTargetFlag_Stopping   = 3,
// }


struct TimerTarget {
   Entity*           entity = nullptr;
   // TimerTargetFlags  flags = 0;
   bool              active = false;
   float             remaining_time = 0;

   void start(Entity* target, float duration)
   {
      entity = target;
      remaining_time = duration;
      active = true;
   }

   void stop()
   {
      entity = nullptr;
      active = false;
      remaining_time = 0;
   }

   bool update()
   {
      /* Returns whether the timer is still active */
      remaining_time -= G_FRAME_INFO.duration;
      if(remaining_time <= 0)
      {
         stop();
         return false;
      }
      else
         return true;
   }
};