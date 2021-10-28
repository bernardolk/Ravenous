Mesh CL_get_collider(Entity* entity)
{
   // A collider is a CollisionMesh with Model matrix applied to it
   Mesh collider;
   for (int i = 0; i < entity->collision_mesh->vertices.size(); i++)
      collider.vertices.push_back(Vertex{entity->collision_mesh->vertices[i] * entity->matModel});

   return collider;
}