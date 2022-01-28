bool save_configs_to_file()
{
   ofstream writer(CONFIG_FILE_PATH);
   if(!writer.is_open())
   {
      cout << "Saving config file failed.\n";
      return false;
   }

   writer << "scene = " << G_CONFIG.initial_scene << "\n";
   writer << "camspeed = " << G_CONFIG.camspeed << "\n";
   writer << "ambient_light = " 
      << G_CONFIG.ambient_light.x << " "
      << G_CONFIG.ambient_light.y << " "
      << G_CONFIG.ambient_light.z << "\n";
      
   writer << "ambient_intensity = " << G_CONFIG.ambient_intensity << "\n";

   writer.close();
   cout << "Config file saved succesfully.\n";

   return true;
}