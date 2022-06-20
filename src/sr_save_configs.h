bool save_configs_to_file()
{
   std::ofstream writer(CONFIG_FILE_PATH);
   if(!writer.is_open())
   {
      std::cout << "Saving config file failed.\n";
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
   std::cout << "Config file saved succesfully.\n";

   return true;
}

void save_camera_settings_to_file(std::string path, vec3 position, vec3 direction)
{
   std::ofstream ofs;
   ofs.open(path);
   ofs << position.x << " ";
   ofs << position.y << " ";
   ofs << position.z << "\n";
   ofs << direction.x << " ";
   ofs << direction.y << " ";
   ofs << direction.z;
   ofs.close();
}