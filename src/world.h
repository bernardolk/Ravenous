
auto get_world_cells_coords_from_world_coords(float x, float y, float z);

// how many cells we have preallocated for the world
const static int WORLD_CELLS_X = 10;
const static int WORLD_CELLS_Y = 10;
const static int WORLD_CELLS_Z = 10;

const static int WC_OFFSET_X = WORLD_CELLS_X / 2;
const static int WC_OFFSET_Y = WORLD_CELLS_Y / 2;
const static int WC_OFFSET_Z = WORLD_CELLS_Z / 2;


// how many meters the cell occupies in the world
const static float WORLD_CELL_SIZE = 10.0f;
// how many entities can coexists in a cell
const static int WORLD_CELL_CAPACITY = 30;

struct WorldCell {
   Entity* entities[WORLD_CELL_CAPACITY];
   unsigned int count = 0;
   
   // coords
   int i = -1, j = -1, k = -1;
};

struct World {
   WorldCell cells[WORLD_CELLS_X][WORLD_CELLS_Y][WORLD_CELLS_Z];
} WORLD;

auto get_world_cells_coords_from_world_coords(float x, float y, float z)
{
   struct {
      int i, j, k;
   } world_cell_coords;

   float world_coords_x_offset = WC_OFFSET_X * WORLD_CELL_SIZE;
   float world_coords_y_offset = WC_OFFSET_Y * WORLD_CELL_SIZE;
   float world_coords_z_offset = WC_OFFSET_Z * WORLD_CELL_SIZE;

   // int division to truncate float result to correct cell position
   world_cell_coords.i = (x + world_coords_x_offset) / WORLD_CELL_SIZE;
   world_cell_coords.j = (y + world_coords_y_offset) / WORLD_CELL_SIZE;
   world_cell_coords.k = (z + world_coords_z_offset) / WORLD_CELL_SIZE;

   return world_cell_coords;
}

vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
   float world_x = (i - WC_OFFSET_X) * WORLD_CELL_SIZE;
   float world_y = (j - WC_OFFSET_Y) * WORLD_CELL_SIZE;
   float world_z = (k - WC_OFFSET_Z) * WORLD_CELL_SIZE;

   return vec3{world_x, world_y, world_z};
}


void assign_entity_to_world_cell(Entity* entity)
{
   auto [x0, x1, z0, z1] = entity->get_rect_bounds();
   float height = entity->get_height();

   auto [i0, j0, k0] = get_world_cells_coords_from_world_coords(x0, entity->position.y, z0);
   auto [i1, j1, k1] = get_world_cells_coords_from_world_coords(x1, entity->position.y + height, z1);

   int cell_count = 1;
   for(int i = i0; i <= i1; i++)
   for(int j = j0; j <= j1; j++)
   for(int k = k0; k <= k1; k++)
   {
      // check if entity is too large
      if(cell_count > ENTITY_WOLRD_CELL_OCCUPATION_LIMIT)
      {
         cout << "FATAL: Entity '" << entity->name << "' is too large and it occupies more than "
            << "the limit of " << ENTITY_WOLRD_CELL_OCCUPATION_LIMIT << " world cells at a time.\n";
         assert(false);
      }


      // update cell situation
      auto& cell = WORLD.cells[i][j][k];
      if(cell.count + 1 > WORLD_CELL_CAPACITY)
      {
         cout << "FATAL: Too many entities inside world cell [" << i << "," << j << "," << k << "]. Aborting.\n";
         assert(false);
      }
      cell.i = i;
      cell.j = j;
      cell.k = k;
      cell.entities[cell.count] = entity;
      cell.count++;

      // assign cell to entity
      entity->world_cells[cell_count - 1] = &WORLD.cells[i][j][k];
      entity->world_cells_count = cell_count;
      cell_count++;
   }
}






