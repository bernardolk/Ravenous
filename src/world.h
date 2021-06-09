
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

// -----------
// WORLD CELL
// -----------

struct WorldCell {
   Entity* entities[WORLD_CELL_CAPACITY];
   unsigned int count = 0;
   
   // coords
   int i = -1, j = -1, k = -1;

   void init()
   {
      for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
         entities[i] = nullptr;
   }

   void remove(Entity* entity)
   {
      for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
         if(entities[i] == entity)
         {
            entities[i] = nullptr;
            defrag();
            return;
         }
   }

   void add(Entity* entity)
   {
      if(count == WORLD_CELL_CAPACITY)
      {
         cout << "FATAL: Too many entities inside world cell [" << i << "," << j << "," << k << "]. Aborting.\n";
         assert(false);
      }

      for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
         if(entities[i] == nullptr)
         {
            entities[i] = entity;
            count++;
         }
   }

   void defrag()
   {
      //@ATTENTION: untested code
      if(count == 0) 
         return;

      unsigned int hole_count = 0;
      int holes[WORLD_CELL_CAPACITY] = {-1};
      for(int i = 0; i < count; i++)
      {
         if(entities[i] == nullptr)
            holes[hole_count++] = i;
      }

      int idx = count - 1; 
      int hole_idx = 0;
      while(true)
      {
         int hole = holes[hole_idx];
         if(hole == -1 || idx == 0)
            break;

         auto item = entities[idx];
         if(item != nullptr)
         {
            entities[hole] = item;
            entities[idx] = nullptr;
            hole_idx++;
         }
         idx--;
      }
   }
};

// -------------------
// COORDINATE METHODS
// -------------------

vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
   float world_x = (i - WC_OFFSET_X) * WORLD_CELL_SIZE;
   float world_y = (j - WC_OFFSET_Y) * WORLD_CELL_SIZE;
   float world_z = (k - WC_OFFSET_Z) * WORLD_CELL_SIZE;

   return vec3{world_x, world_y, world_z};
}

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

// ------
// WORLD
// ------

struct World {
   WorldCell cells[WORLD_CELLS_X][WORLD_CELLS_Y][WORLD_CELLS_Z];
   WorldCell* cells_in_use[WORLD_CELLS_X * WORLD_CELLS_Y * WORLD_CELLS_Z];
   int cells_in_use_count = 0;

   void init()
   {
      for(int i = 0; i < WORLD_CELLS_X; i++)
      for(int j = 0; j < WORLD_CELLS_Y; j++)
      for(int k = 0; k < WORLD_CELLS_Z; k++)
         cells[i][j][k].init();
   }

   void update_cells_in_use_list()
   {
      cells_in_use_count = 0;
      for(int i = 0; i < WORLD_CELLS_X; i++)
      for(int j = 0; j < WORLD_CELLS_Y; j++)
      for(int k = 0; k < WORLD_CELLS_Z; k++)
      {
         auto cell = &cells[i][j][k];
         if(cell->count != 0)
         {
            cells_in_use[cells_in_use_count] = cell;
            cells_in_use_count++;
         }
      }
   }

   void assign_entity_to_world_cells(Entity* entity)
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
         auto& cell = cells[i][j][k];
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
         entity->world_cells[cell_count - 1] = &cells[i][j][k];
         entity->world_cells_count = cell_count;
         cell_count++;
      }
   }

   void update_entity_world_cells(Entity* entity)
   {
      // computes the new cells
      auto [x0, x1, z0, z1] = entity->get_rect_bounds();
      float height = entity->get_height();

      auto [i0, j0, k0] = get_world_cells_coords_from_world_coords(x0, entity->position.y, z0);
      auto [i1, j1, k1] = get_world_cells_coords_from_world_coords(x1, entity->position.y + height, z1);

      vector<WorldCell*> new_cells;
      for(int i = i0; i <= i1; i++)
      for(int j = j0; j <= j1; j++)
      for(int k = k0; k <= k1; k++)
         new_cells.push_back(&cells[i][j][k]);

      // checks the diff between new and old and stores the ones that are no longer
      // entities world cells
      vector<WorldCell*> cells_to_remove_from;
      for(int i = 0; i < entity->world_cells_count; i++)
      {
         auto entity_world_cell = entity->world_cells[i];
         bool found = false;
         for(int c = 0; c < new_cells.size(); c++)
         {
            if(new_cells[c] == entity_world_cell)
            {
               found = true;
               break;
            }
         }
         if(!found)
            cells_to_remove_from.push_back(entity_world_cell);
      }

      vector<WorldCell*> cells_to_add_to;
      for(int i = 0; i < new_cells.size(); i++)
      {
         auto cell = new_cells[i];
         bool exists = false;
         for(int j = 0; j < entity->world_cells_count; j++)
         {
            auto entity_cell = entity->world_cells[j];
            if(cell == entity_cell)
            {
               exists = true;
               break;
            }
         }      
         if(!exists)
            cells_to_add_to.push_back(cell);
      }

      // remove entity from old cells
      for(int i = 0; i < cells_to_remove_from.size(); i++)
      {
         auto cell = cells_to_remove_from[i];
         cell->remove(entity);
      }

      // add entity to new cells
      for(int i = 0; i < cells_to_add_to.size(); i++)
      {
         auto cell = cells_to_add_to[i];
         cell->add(entity);
      }

      // re-set cells to entity
      int new_cells_count = 0;
      for(int i = 0; i < new_cells.size(); i++)
      {
         auto cell = new_cells[i];
         entity->world_cells[new_cells_count] = cell;
         new_cells_count++;
      }
      entity->world_cells_count = new_cells_count;
   }
};







