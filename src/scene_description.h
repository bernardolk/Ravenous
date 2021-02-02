// CREATE SCENE 
 Scene demo_scene;

// ENTITY SETUP


auto find1 = Shader_Catalogue.find("model");
auto model_shader = find1->second;

auto find2 = Geometry_Catalogue.find("quad");
auto quad_mesh = find2->second;

/* 
struct Entity {
	unsigned int index;
	unsigned int id;
	Shader* shader;
	glm::vec3 position;
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);
	glm::mat4 matModel = mat4identity;
   glm::vec3 velocity;
   void* collision_geometry_ptr;
   CollisionGeometryEnum collision_geometry_type;
   std::string name = "NONAME";
   std::vector<Texture> textures;
   GLData gl_data;
};
*/


// collision geometry for platform
CollisionGeometryAlignedBox cgab { 1, 0, 1 };

/*
   // platform 1
   Entity platform;
   platform.name = "Platform 1";
   platform.index = G_ENTITY_INFO.entity_counter;
   platform.id = ++G_ENTITY_INFO.entity_counter;
   platform.shader = &model_shader;
   platform.position = vec3(0.5, 0, 1.0);
   platform.rotation = vec3(90, 0, 90);
   platform.scale = vec3(1.0f, 1.0f, 1.0f);
   platform.mesh = quad_mesh;
   platform.collision_geometry_type = COLLISION_ALIGNED_BOX;
   platform.collision_geometry_ptr = &cgab;

   // add to scene
   demo_scene.entities.push_back(&platform);
*/


// // COLLISION DEBUG BOUNDARY LINES

// Mesh line_mesh;
// line_mesh.vertices = line_vertex_vec;
// line_mesh.render_method = GL_LINE_LOOP;

// Model line_model;
// line_model.mesh = line_mesh;
// line_model.gl_data = setup_gl_data_for_mesh(&line_mesh);

// Entity collision_lines {
//      G_ENTITY_INFO.entity_counter,
//    ++G_ENTITY_INFO.entity_counter,
//    &line_model,
//    &line_shader
// };

// collision_lines.name = "LINE1";

// demo_scene.entities.push_back(&collision_lines);

// // COLLISION DEBUG LINES 2
// Mesh line_mesh2;
// line_mesh2.vertices = line_vertex_vec;
// line_mesh2.render_method = GL_LINE_LOOP;

// Model line_model2;
// line_model2.mesh = line_mesh2;
// line_model2.gl_data = setup_gl_data_for_mesh(&line_mesh2);

// Entity collision_lines2 {
//      G_ENTITY_INFO.entity_counter,
//    ++G_ENTITY_INFO.entity_counter,
//    &line_model2,
//    &line_shader
// };

// collision_lines2.name = "LINE2";

// demo_scene.entities.push_back(&collision_lines2);

// // COLLISION DEBUG LINES 2
// Mesh line_mesh3;
// line_mesh3.vertices = line_vertex_vec;
// line_mesh3.render_method = GL_LINE_LOOP;

// Model line_model3;
// line_model3.mesh = line_mesh3;
// line_model3.gl_data = setup_gl_data_for_mesh(&line_mesh3);

// Entity collision_lines3 {
//      G_ENTITY_INFO.entity_counter,
//    ++G_ENTITY_INFO.entity_counter,
//    &line_model3,
//    &line_shader
// };

// collision_lines3.name = "LINE3";

// demo_scene.entities.push_back(&collision_lines3);



// CYLINDER
float cylinder_half_length = 0.35f;
float cylinder_radius = 0.15f;
unsigned int pink_texture = load_texture_from_file("pink.jpg", "w:/assets/textures");
Texture cylinder_texture{
   pink_texture,
   "texture_diffuse",
   "whatever"
};

Mesh* cylinder_mesh = new Mesh();
cylinder_mesh->vertices = construct_cylinder(cylinder_radius, cylinder_half_length, 24);
cylinder_mesh->render_method = GL_TRIANGLE_STRIP;
cylinder_mesh->gl_data = setup_gl_data_for_mesh(cylinder_mesh);

Entity cylinder;
cylinder.index = G_ENTITY_INFO.entity_counter;
cylinder.id = ++G_ENTITY_INFO.entity_counter;
cylinder.shader = &model_shader;
cylinder.position = vec3(0,1,1);
cylinder.textures = std::vector<Texture>{cylinder_texture};
cylinder.mesh = *cylinder_mesh;

// player collision geometry
cylinder.collision_geometry_type = COLLISION_ALIGNED_CYLINDER;
CollisionGeometryAlignedCylinder cgac { cylinder_half_length, cylinder_radius};
cylinder.collision_geometry_ptr = &cgac;

demo_scene.entities.push_back(&cylinder);


// lightsource
PointLight l1;
l1.id = 1;
l1.position = vec3(0.5, 2.5, 0.5);
l1.diffuse = vec3(1.0, 1.0, 1.0);
l1.ambient = vec3(1.0,1.0,1.0);
l1.intensity_linear = 0.4f;
l1.intensity_quadratic = 0.04f;
demo_scene.pointLights.push_back(l1);

G_SCENE_INFO.active_scene = &demo_scene;

// GAMEPLAY CODE

// create player
Player player;
player.entity_ptr = &cylinder;
player.player_state = PLAYER_STATE_FALLING;
player.entity_ptr->velocity.y = -1 * player.fall_speed;
player.entity_ptr->name = "player";