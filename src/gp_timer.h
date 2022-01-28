struct Timer {
   Entity*           entity = nullptr;
   bool              active = false;
   float             remaining_time = 0;

   void start(Entity* target, float duration)
   {
      entity = target;
      remaining_time = duration;
      active = true;
      
      // setup anim in Entity_Animations if there is animation
   }

   void stop()
   {
      entity = nullptr;
      active = false;
      remaining_time = 0;

      // setup anim in Entity_Animations if there is animation
   }

   bool update()
   {
      /* Returns whether the timer is still active */
      remaining_time -= G_FRAME_INFO.duration;
      if(remaining_time <= 0)
         return false;
      else
         return true;
   }
};