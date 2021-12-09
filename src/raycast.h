#include <triangle.h>
#include <face.h>

struct RaycastTest {
   bool hit             = false;
   float distance;
   Entity* entity       = NULL;

   int obj_hit_index    = -1;
   string obj_hit_type;
   
   Triangle t;
   u16 t_index;

   Ray ray;
};

enum RayCastType {
   RayCast_TestOnlyFromOutsideIn       = 0,
   RayCast_TestBothSidesOfTriangle     = 1,
   RayCast_TestOnlyVisibleEntities     = 2
};

// Prototypes
Ray cast_pickray();
RaycastTest test_ray_against_scene                 (Ray ray, RayCastType test_type, Entity* skip, float max_distance);
RaycastTest test_ray_against_entity                (Ray ray, Entity* entity, RayCastType test_type, float max_distance);
RaycastTest test_ray_against_mesh                  (Ray ray, Mesh* mesh, glm::mat4 matModel, RayCastType test_type);
RaycastTest test_ray_against_triangle              (Ray ray, Triangle triangle, bool test_both_sides);
Triangle get_triangle_for_indexed_mesh             (Mesh* mesh, glm::mat4 matModel, int triangle_index);
Triangle get_triangle_for_indexed_mesh             (Entity* entity, int triangle_index);
RaycastTest test_ray_against_collider              (Ray ray, Mesh* collider, RayCastType test_type);
vec3 point_from_detection                          (Ray ray, RaycastTest result);
vec3 get_triangle_normal                           (Triangle t);


// -------------------------
// > TEST RAY AGASINT SCENE
// -------------------------

RaycastTest test_ray_against_scene(Ray ray, RayCastType test_type = RayCast_TestOnlyFromOutsideIn, 
   Entity* skip = nullptr, float max_distance = MAX_FLOAT)
{
   /* This will test a ray agains the scene
      @todo - This should first test ray against world cells, then get the list of entities from these world cells to
               test against 
   
   Parameters: 
      - only_test_visible_entities: refers to the option "hide entity" in editor's entity panel
      - skip_id: will skip testing entity with this id
      - test_both_sides: refers to the sides of the triangles in the mesh
   
   */

   float min_distance = MAX_FLOAT;
   RaycastTest closest_hit{false, -1};

   Entity** entity_iterator = &(G_SCENE_INFO.active_scene->entities[0]);
   int entities_vec_size =  G_SCENE_INFO.active_scene->entities.size();
	for(int it = 0; it < entities_vec_size; it++) 
   {
	   auto entity = *entity_iterator++;
      if(test_type == RayCast_TestOnlyVisibleEntities && (!entity->render_me || entity->wireframe))
         continue;
      if(skip != nullptr && entity->id == skip->id)
         continue;
         
      auto test = test_ray_against_entity(ray, entity, test_type, max_distance);

      if(test.hit && test.distance < min_distance)
      {
         closest_hit = test;
         closest_hit.entity = entity;
         min_distance = test.distance;
      }
	}

   return closest_hit;
}

RaycastTest test_ray_against_scene(Ray ray, Entity* skip)
{
   return test_ray_against_scene(ray, RayCast_TestOnlyFromOutsideIn, skip);
}

// --------------------------
// > TEST RAY AGAINST ENTITY
// --------------------------
RaycastTest test_ray_against_entity(Ray ray, Entity* entity, RayCastType test_type = RayCast_TestOnlyFromOutsideIn,
   float max_distance = MAX_FLOAT)
{
   // @TODO: when testing against player, we could:
   //      a) find the closest point between player's column and the ray
   //      b) do a sphere vs ray test 
   //      instead of testing the collider


   // first check collision with bounding box
   auto [hit, tmin, tmax] = CL_test_ray_vs_aabb(ray, entity->bounding_box);
   if(!hit)
   {
      return RaycastTest{false};
   }

   else if(hit)
   {
      if(test_type == RayCast_TestOnlyFromOutsideIn && tmin > 0)
      {
         // If tmin < 0 it means we are coming from inside the box
         if(tmin < max_distance)
            return test_ray_against_collider(ray, &entity->collider, test_type);
      }
      else
      {
         float t = tmin;
         if(tmin < 0)
            t = tmax;
         
         if(t < max_distance)
            return test_ray_against_collider(ray, &entity->collider, test_type);
      }
   }  

   return RaycastTest{false};
}

// ---------------------------
// > TEST RAY AGAINT COLLIDER
// ---------------------------
// This doesn't take a matModel

int Frame_Ray_Collider_Count;

RaycastTest test_ray_against_collider(Ray ray, Mesh* collider, RayCastType test_type)
{
   Frame_Ray_Collider_Count++;

   int triangles = collider->indices.size() / 3;
   float min_distance = MAX_FLOAT;
   RaycastTest min_hit_test{false, -1};
   for(int i = 0; i < triangles; i++)
   {
      Triangle t = get_triangle_for_collider_indexed_mesh(collider, i);
      bool test_both_sides =  test_type == RayCast_TestBothSidesOfTriangle;
      auto test = test_ray_against_triangle(ray, t, test_both_sides);
      if(test.hit && test.distance < min_distance)
      {
         min_hit_test = test;
         min_hit_test.t_index = i;
         min_distance = test.distance;
      }
   }

   return min_hit_test;
}

// ------------------------
// > TEST RAY AGAINST MESH
// ------------------------
// This does take a matModel

RaycastTest test_ray_against_mesh(Ray ray, Mesh* mesh, glm::mat4 matModel, RayCastType test_type)
{
   int triangles = mesh->indices.size() / 3;
   float min_distance = MAX_FLOAT;
   RaycastTest min_hit_test{false, -1};
   for(int i = 0; i < triangles; i++)
   {
      Triangle t = get_triangle_for_indexed_mesh(mesh, matModel, i);
      bool test_both_sides =  test_type == RayCast_TestBothSidesOfTriangle;
      auto test = test_ray_against_triangle(ray, t, test_both_sides);
      if(test.hit && test.distance < min_distance)
      {
         min_hit_test = test;
         min_hit_test.t_index = i;
         min_distance = test.distance;
      }
   }

   return min_hit_test;
}

// ----------------------------
// > TEST RAY AGAINST TRIANGLE
// ----------------------------
RaycastTest test_ray_against_triangle(Ray ray, Triangle triangle, bool test_both_sides = true)
{
   auto &A        = triangle.a;
   auto &B        = triangle.b;
   auto &C        = triangle.c;
	vec3 E1        = B - A;
	vec3 E2        = C - A;
	vec3 AO        = ray.origin - A;

   // check hit with one side of triangle
   vec3 N         = cross(E1, E2);
	float det      = -dot(ray.direction, N);
	float invdet   = 1.0 / det;
	vec3 DAO       = cross(AO, ray.direction);
	float u        = dot(E2, DAO) * invdet;
	float v        = -dot(E1, DAO) * invdet;
	float t        = dot(AO, N) * invdet;
	bool test      = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);

   if(!test && test_both_sides)
   {
      // check other side
      N        = cross(E2, E1);
      det      = -dot(ray.direction, N);
      invdet   = 1.0 / det;
      DAO      = cross(ray.direction, AO);
      u        = dot(E2, DAO) * invdet;
      v        = -dot(E1, DAO) * invdet;
      t        = dot(AO, N) * invdet;
      test     = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
   }

   if (test)
   {
      RaycastTest result;
      result.hit = true;
      result.distance = t;
      result.t = triangle;
      result.ray = ray;
      return result;
   }

	return RaycastTest{false, -1};
}


// --------------------------
// > TEST RAY AGAINST LIGHTS
// --------------------------
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

      auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
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

      auto test = test_ray_against_mesh(ray, aabb_mesh, aabb_model, RayCast_TestBothSidesOfTriangle);
      if(test.hit && test.distance < min_distance)
      {
         closest_hit = {true, test.distance, NULL, spot_c, "spot"};
         min_distance = test.distance;
      }
      spot_c++;
   }

   return closest_hit;
}

// ---------------
// > CAST PICKRAY
// ---------------
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