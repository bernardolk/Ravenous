


void editor_check_clicking()
{
   if(G_INPUT_INFO.mouse_state & MOUSE_LEFT_BTN && !(G_INPUT_INFO.mouse_state & MOUSE_DRAGGING))
   {
      auto pickray = cast_pickray();
      auto test = test_ray_against_scene(pickray);
      if(test.hit)
      {
         cout << "HIT ENTITY '" + test.entity->name + "' :D \n";
      }
   }
}