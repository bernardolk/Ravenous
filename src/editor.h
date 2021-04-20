


void editor_check_clicking()
{
   if(G_INPUT_INFO.is_mouse_left_btn_press && !G_INPUT_INFO.is_mouse_drag)
   {
      auto pickray = cast_pickray();
      auto test = test_ray_against_scene(pickray);
      if(test.hit)
      {
         cout << "HIT ENTITY '" + test.entity->name + "' :D \n";
      }
   }
}