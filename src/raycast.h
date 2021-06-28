struct Triangle {
   vec3 a;
   vec3 b;
   vec3 c;
};

struct Face {
   Triangle a;
   Triangle b;
};

struct RaycastTest {
   bool hit = false;
   float distance;
   Entity* entity = NULL;
   int obj_hit_index = -1;
   string obj_hit_type;
   Triangle t;
   u16 t_index;
};

struct Ray {
   vec3 origin;
   vec3 direction;
};

Ray cast_pickray();
RaycastTest test_ray_against_scene(Ray ray,  bool only_test_visible_entities);
RaycastTest test_ray_against_entity(Ray ray, Entity* entity);
RaycastTest test_ray_against_mesh(Ray ray, Mesh* mesh, glm::mat4 matModel);
RaycastTest test_ray_against_triangle(Ray ray, Triangle triangle);
Triangle get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 matModel, int triangle_index);
Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index);
vec3 point_from_detection(Ray ray, RaycastTest result);


RaycastTest test_ray_against_lights(Ray ray)
{
   float min_distance = MAX_FLOAT;
   RaycastTest closest_hit{false, -1};

   auto scene = G_SCENE_INFO.active_scene;
   auto aabb_mesh = Geometry_Catalogue.find("aabb")->second;

   int point_c = 0;
	for (auto point_light_ptr = scene->pointLights.begin(); 
      point_light_ptr != scene->pointLights.end(); 
      point_light_ptr++)
   {
      // subtract lightbulb model size from position
      auto position = point_light_ptr->position - vec3{0.1575, 0, 0.1575};
      auto aabb_model = translate(mat4identity, position);
      aabb_model = glm::scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

      auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model);
      if(test.hit && test.distance < min_distance)
      {
         closest_hit = {true, test.distance, NULL, point_c, "point"};
         min_distance = test.distance;
      }
      point_c++;
   }

   int spot_c = 0;
   for (auto spotlight_ptr = scene->spotLights.begin(); 
      spotlight_ptr != scene->spotLights.end(); 
      spotlight_ptr++)
   {
       // subtract lightbulb model size from position
      auto position = spotlight_ptr->position - vec3{0.1575, 0, 0.1575};
      auto aabb_model = translate(mat4identity, position);
      aabb_model = glm::scale(aabb_model, vec3{0.3f, 0.6f, 0.3f});

      auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model);
      if(test.hit && test.distance < min_distance)
      {
         closest_hit = {true, test.distance, NULL, spot_c, "spot"};
         min_distance = test.distance;
      }
      spot_c++;
   }

   return closest_hit;
}


RaycastTest test_ray_against_scene(Ray ray, bool only_test_visible_entities = false)
{
   float min_distance = MAX_FLOAT;
   RaycastTest closest_hit{false, -1};

   Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      if(only_test_visible_entities && (!entity->render_me || entity->wireframe))
         continue;
         
      auto test = test_ray_against_entity(ray, entity);

      if(test.hit && test.distance < min_distance)
      {
         closest_hit = test;
         closest_hit.entity = entity;
         min_distance = test.distance;
      }
	}

   return closest_hit;
}

Triangle get_triangle_for_indexed_mesh(Mesh* mesh, glm::mat4 matModel, int triangle_index)
{
   auto a_ind = mesh->indices[3 * triangle_index + 0];
   auto b_ind = mesh->indices[3 * triangle_index + 1];
   auto c_ind = mesh->indices[3 * triangle_index + 2];

   auto a_vertice = mesh->vertices[a_ind].position;
   auto b_vertice = mesh->vertices[b_ind].position;
   auto c_vertice = mesh->vertices[c_ind].position;

   auto a = matModel * glm::vec4(a_vertice, 1.0);
   auto b = matModel * glm::vec4(b_vertice, 1.0);
   auto c = matModel * glm::vec4(c_vertice, 1.0);

   return Triangle{a, b, c};
}

Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index)
{
   Mesh* mesh = entity->mesh;
   mat4 model = entity->matModel;

   return get_triangle_for_indexed_mesh(mesh, model, triangle_index);
}


RaycastTest test_ray_against_entity(Ray ray, Entity* entity)
{
   Mesh* mesh = entity->mesh;
   mat4 model = entity->matModel;

   return test_ray_against_mesh(ray, mesh, model);
}


RaycastTest test_ray_against_mesh(Ray ray, Mesh* mesh, glm::mat4 matModel)
{
   int triangles = mesh->indices.size() / 3;
   float min_distance = MAX_FLOAT;
   RaycastTest min_hit_test{false, -1};
   for(int i = 0; i < triangles; i++)
   {
      Triangle t = get_triangle_for_indexed_mesh(mesh, matModel, i);
      auto test = test_ray_against_triangle(ray, t);
      if(test.hit && test.distance < min_distance)
      {
         min_hit_test = test;
         min_hit_test.t_index = i;
         min_distance = test.distance;
      }
   }

   return min_hit_test;
}


RaycastTest test_ray_against_triangle(Ray ray, Triangle triangle)
{
   auto &A = triangle.a;
   auto &B = triangle.b;
   auto &C = triangle.c;

	vec3 E1 = B - A;
	vec3 E2 = C - A;
	vec3 N = glm::cross(E1, E2);
	float det = -glm::dot(ray.direction, N);
	float invdet = 1.0 / det;
	vec3 AO = ray.origin - A;
	vec3 DAO = glm::cross(AO, ray.direction);
	float u = glm::dot(E2, DAO) * invdet;
	float v = -glm::dot(E1, DAO) * invdet;
	float t = glm::dot(AO, N) * invdet;
	bool test1 = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
	if (test1)
		return RaycastTest{true, t};

   // check "other-side" (why so much duplication though?)
   E1 = B - A;
   E2 = C - A;
   N = glm::cross(E2, E1);
   det = -glm::dot(ray.direction, N);
   invdet = 1.0 / det;
   AO = ray.origin - A;
   DAO = glm::cross(ray.direction, AO);
   u = glm::dot(E2, DAO) * invdet;
   v = -glm::dot(E1, DAO) * invdet;
   float t2 = glm::dot(AO, N) * invdet;
   bool test2 = (det >= 1e-6 && t2 >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
   if (test2)
   {
      RaycastTest test;
      test.hit = true;
      test.distance = t2;
      test.t = triangle;
      return test;
   }

	return RaycastTest{false, -1};
}


Ray cast_pickray() 
{
	float screenX_normalized = (G_INPUT_INFO.mouse_coords.x - G_DISPLAY_INFO.VIEWPORT_WIDTH / 2) /
                              (G_DISPLAY_INFO.VIEWPORT_WIDTH / 2);
	float screenY_normalized = -1 * (G_INPUT_INFO.mouse_coords.y - G_DISPLAY_INFO.VIEWPORT_HEIGHT / 2) / 
                                   (G_DISPLAY_INFO.VIEWPORT_HEIGHT / 2);

	glm::vec4 ray_clip(screenX_normalized, screenY_normalized, -1.0, 1.0);
	glm::mat4 inv_view = glm::inverse(G_SCENE_INFO.camera->View4x4);
	glm::mat4 inv_proj = glm::inverse(G_SCENE_INFO.camera->Projection4x4);
	vec3 ray_eye_3 = (inv_proj * ray_clip);
	glm::vec4 ray_eye(ray_eye_3.x, ray_eye_3.y, -1.0, 0.0);
   auto direction = glm::normalize(inv_view * ray_eye);
   auto origin = G_SCENE_INFO.camera->Position;

	return Ray{origin, direction};
}

vec3 point_from_detection(Ray ray, RaycastTest result)
{
   assert(result.hit);

   return ray.origin + ray.direction * result.distance;
}

Face face_from_axis_aligned_triangle(Triangle t)
{
   vec3 normal = glm::triangleNormal(t.a, t.b, t.c);
   vec3 a2 = glm::rotate(t.a, glm::radians(180.0f), normal);
   vec3 b2 = glm::rotate(t.b, glm::radians(180.0f), normal);
   vec3 c2 = glm::rotate(t.c, glm::radians(180.0f), normal);
   return Face{t, Triangle{a2, b2, c2}};


   // float x0 = min({t.a.x, t.b.x, t.c.x});
   // float x1 = max({t.a.x, t.b.x, t.c.x});
   // float y0 = min({t.a.y, t.b.y, t.c.y});
   // float y1 = max({t.a.y, t.b.y, t.c.y});
   // float z0 = min({t.a.z, t.b.z, t.c.z});
   // float z1 = max({t.a.z, t.b.z, t.c.z});

   // auto t1 = Triangle {vec3{x0,z0,y0}, vec3{x1,z0,y0}, vec3{x0,z0,y1}};
}