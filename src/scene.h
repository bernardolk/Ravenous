// CREATE SCENE 
   Scene demo_scene;

// ENTITY SETUP
unsigned int brick_texture = load_texture_from_file("brickwall.jpg", "w:/assets/textures");
unsigned int brick_normal_texture = load_texture_from_file("brickwall_normal.jpg", "w:/assets/textures");
Texture quad_wall_texture{
   brick_texture,
   "texture_diffuse",
   "whatever"
};
Texture quad_wall_normal_texture{
   brick_normal_texture,
   "texture_normal",
   "whatever"
};
vector<Texture> texture_vec;
texture_vec.push_back(quad_wall_texture);
texture_vec.push_back(quad_wall_normal_texture);

Mesh quad_mesh;
quad_mesh.vertices = quad_vertex_vec;
quad_mesh.indices = quad_vertex_indices;
quad_mesh.render_method = GL_TRIANGLES;

Model quad_model;
quad_model.mesh = quad_mesh;
quad_model.textures = texture_vec;
quad_model.gl_data = setup_gl_data_for_mesh(&quad_mesh);

// platform 1
Entity platform{
   G_ENTITY_INFO.entity_counter,
   ++G_ENTITY_INFO.entity_counter,
   &quad_model,
   &model_shader,
   vec3(0.5, 0, 0.5),
   vec3(90, 0, 90),
   vec3(1.0f, 1.0f, 1.0f)
};

// platform 2
Entity platform2{
   G_ENTITY_INFO.entity_counter,
   ++G_ENTITY_INFO.entity_counter,
   &quad_model,
   &model_shader,
   vec3(1, -0.3, 1   ),
   vec3(90, 0, 90),
   vec3(1.0f, 1.0f, 1.0f)
};

// collision geometry for platform
CollisionGeometryAlignedBox cgab { 1, 0, 1 };

platform.collision_geometry_type = COLLISION_ALIGNED_BOX;
platform.collision_geometry_ptr = &cgab;


platform2.collision_geometry_type = COLLISION_ALIGNED_BOX;
platform2.collision_geometry_ptr = &cgab;

// add to scene
demo_scene.entities.push_back(&platform);
demo_scene.entities.push_back(&platform2);



// CYLINDER

float cylinder_half_length = 0.35f;
float cylinder_radius = 0.15f;
unsigned int pink_texture = load_texture_from_file("pink.jpg", "w:/assets/textures");
Texture cylinder_texture{
   pink_texture,
   "texture_diffuse",
   "whatever"
};

Mesh cylinder_mesh;
cylinder_mesh.vertices = construct_cylinder(cylinder_radius, cylinder_half_length, 24);
cylinder_mesh.render_method = GL_TRIANGLE_STRIP;

Model cylinder_model;
cylinder_model.mesh = cylinder_mesh;
cylinder_model.textures = std::vector<Texture>{cylinder_texture};
cylinder_model.gl_data = setup_gl_data_for_mesh(&cylinder_mesh);

Entity cylinder{
   G_ENTITY_INFO.entity_counter,
   ++G_ENTITY_INFO.entity_counter,
   &cylinder_model,
   &model_shader,
   vec3(0,1,1)
};

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
l1.ambient = vec3(0.6,0.6,0.6);
l1.intensity_linear = 0.4f;
l1.intensity_quadratic = 0.04f;
demo_scene.pointLights.push_back(l1);

G_SCENE_INFO.active_scene = &demo_scene;

// GAMEPLAY CODE

// create player
Player player;
player.entity_ptr = &cylinder;
player.player_state = PLAYER_STATE_FALLING;
player.entity_ptr->velocity.y = -0.5f;