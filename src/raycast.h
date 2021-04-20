struct RaycastTest{
   bool hit = false;
   float distance;
   Entity* entity = NULL;
};

struct Triangle{
   vec3 a;
   vec3 b;
   vec3 c;
};

struct Ray{
   vec3 origin;
   vec3 direction;
};

Ray cast_pickray();
RaycastTest test_ray_against_scene(Ray ray);
RaycastTest test_ray_against_entity(Ray ray, Entity* entity);
RaycastTest test_ray_against_triangle(Ray ray, Triangle triangle);
Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index);


RaycastTest test_ray_against_scene(Ray ray)
{
   float min_distance = MAX_FLOAT;
   RaycastTest closest_hit{false, -1};

   Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      auto test = test_ray_against_entity(ray, entity);

      if(test.hit && test.distance < min_distance)
      {
         closest_hit = {true, test.distance, entity};
         min_distance = test.distance;
      }
	}

   return closest_hit;
}


Triangle get_triangle_for_indexed_mesh(Entity* entity, int triangle_index)
{
   auto a_ind = entity->mesh.indices[3 * triangle_index + 0];
   auto b_ind = entity->mesh.indices[3 * triangle_index + 1];
   auto c_ind = entity->mesh.indices[3 * triangle_index + 2];

   auto a_mesh = entity->mesh.vertices[a_ind].position;
   auto b_mesh = entity->mesh.vertices[b_ind].position;
   auto c_mesh = entity->mesh.vertices[c_ind].position;

   auto a = entity->matModel * glm::vec4(a_mesh, 1.0);
   auto b = entity->matModel * glm::vec4(b_mesh, 1.0);
   auto c = entity->matModel * glm::vec4(c_mesh, 1.0);

   return Triangle{a, b, c};
}


RaycastTest test_ray_against_entity(Ray ray, Entity* entity)
{
   int triangles = entity->mesh.indices.size() / 3;
   float min_distance = MAX_FLOAT;
   RaycastTest min_hit_test{false, -1};
   for(int i = 0; i < triangles; i++)
   {
      Triangle t = get_triangle_for_indexed_mesh(entity, i);
      auto test = test_ray_against_triangle(ray, t);
      if(test.hit && test.distance < min_distance)
      {
         min_hit_test = test;
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
      return RaycastTest{true, t2};

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
