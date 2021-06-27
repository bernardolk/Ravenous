
auto get_world_cells_coords_from_world_coords(float x, float y, float z);
vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k);

// how many cells we have preallocated for the world
const static int W_CELLS_NUM_X = 10;
const static int W_CELLS_NUM_Y = 10;
const static int W_CELLS_NUM_Z = 10;

// how many cells are before and after the origin in each axis
const static int W_CELLS_OFFSET_X = W_CELLS_NUM_X / 2;
const static int W_CELLS_OFFSET_Y = W_CELLS_NUM_Y / 2;
const static int W_CELLS_OFFSET_Z = W_CELLS_NUM_Z / 2;

// how many meters the cell occupies in the world
const static float W_CELL_LEN_METERS = 10.0f;
// how many entities can coexists in a cell
const static int WORLD_CELL_CAPACITY = 30;

const static vec3 W_UPPER_BOUNDS_METERS = {
   W_CELLS_OFFSET_X * W_CELL_LEN_METERS ,
   W_CELLS_OFFSET_Y * W_CELL_LEN_METERS ,
   W_CELLS_OFFSET_Z * W_CELL_LEN_METERS
};

const static vec3 W_LOWER_BOUNDS_METERS = {
   -1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS,
   -1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS,
   -1.0 * W_CELLS_OFFSET_X * W_CELL_LEN_METERS
};

enum CellUpdateStatus {
   OK,
   ENTITY_TOO_BIG,
   CELL_FULL,
   OUT_OF_BOUNDS,
   UNEXPECTED
};

struct CellUpdate {
   CellUpdateStatus status;
   string message;
   bool entity_changed_cell;
};



// -----------
// WORLD CELL
// -----------

struct WorldCell {
   Entity* entities[WORLD_CELL_CAPACITY];
   unsigned int count;
   
   // coords
   int i = -1, j = -1, k = -1;

   void init(int ii, int ji, int ki)
   {
      count = 0;

      // set coordinates
      i = ii;
      j = ji;
      k = ki;

      // initialize entities list
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

   CellUpdate add(Entity* entity)
   {
      if(count == WORLD_CELL_CAPACITY)
      {
         string message = "World cell '" + coords_str() + "' is full.";
         return CellUpdate{ CELL_FULL, message };
      }

      for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
         if(entities[i] == nullptr)
         {
            entities[i] = entity;
            count++;
            return CellUpdate{ OK };
         }

      return CellUpdate{ UNEXPECTED, "world cell add method returned weirdly." };
   }

   void defrag()
   {
      if(count == 0)
         return;

      // initialize holes array
      unsigned int hole_count = 0;
      int holes[WORLD_CELL_CAPACITY];
      for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
         holes[i] = -1;

      // find holes and store in array
      // also count how many items there are
      int new_count = 0;
      for(int i = 0; i < count; i++)
      {
         // we dont want to count the last empty spot as a hole
         if(entities[i] == nullptr)
         {
            if(i + 1 != count)
               holes[hole_count++] = i;
         }
         else 
            new_count++;
      }

      // loop through list from top to bottom and fill
      // holes as it finds candidates to swap      
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
      count = new_count;
   }

   string coords_str()
   {
      return "Cell [" + to_string(i) 
         + "," + to_string(j) + "," + to_string(k) 
         + "] (" + to_string(count) + ")";
   }

   vec3 coords()
   {
      return vec3{i, j, k};
   }

   vec3 coords_meters()
   {
      return get_world_coordinates_from_world_cell_coordinates(i, j, k);
   }

   string coords_meters_str()
   {
      vec3 mcoords = coords_meters();
      return "[x: " + format_float_tostr(mcoords[0], 1) 
         + ", y: " + format_float_tostr(mcoords[1], 1) + ", z: " + format_float_tostr(mcoords[2], 1) 
         + "]";
   }
};

// -------------------
// COORDINATE METHODS
// -------------------

vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
   float world_x = (i - W_CELLS_OFFSET_X) * W_CELL_LEN_METERS;
   float world_y = (j - W_CELLS_OFFSET_Y) * W_CELL_LEN_METERS;
   float world_z = (k - W_CELLS_OFFSET_Z) * W_CELL_LEN_METERS;

   return vec3{world_x, world_y, world_z};
}


auto get_world_cells_coords_from_world_coords(float x, float y, float z)
{
   struct {
      int i, j, k;
   } world_cell_coords;

   // if out of bounds return -1
   if(x < W_LOWER_BOUNDS_METERS.x || x > W_UPPER_BOUNDS_METERS.x ||
      y < W_LOWER_BOUNDS_METERS.y || y > W_UPPER_BOUNDS_METERS.y ||
      z < W_LOWER_BOUNDS_METERS.z || z > W_UPPER_BOUNDS_METERS.z)
   {
      world_cell_coords.i = -1;
      world_cell_coords.j = -1;
      world_cell_coords.k = -1;
      return world_cell_coords;
   }

   // int division to truncate float result to correct cell position
   world_cell_coords.i = (x + W_CELLS_OFFSET_X * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;
   world_cell_coords.j = (y + W_CELLS_OFFSET_Y * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;
   world_cell_coords.k = (z + W_CELLS_OFFSET_Z * W_CELL_LEN_METERS) / W_CELL_LEN_METERS;

   return world_cell_coords;
}

auto get_world_cells_coords_from_world_coords(vec3 position)
{
   return get_world_cells_coords_from_world_coords(position.x, position.y, position.z);
}

// ------
// WORLD
// ------

struct WorldStruct {
   WorldCell cells[W_CELLS_NUM_X][W_CELLS_NUM_Y][W_CELLS_NUM_Z];
   WorldCell* cells_in_use[W_CELLS_NUM_X * W_CELLS_NUM_Y * W_CELLS_NUM_Z];
   int cells_in_use_count = 0;

   WorldStruct()
   {
      init();
   }

   void init()
   {
      for(int i = 0; i < W_CELLS_NUM_X; i++)
      for(int j = 0; j < W_CELLS_NUM_Y; j++)
      for(int k = 0; k < W_CELLS_NUM_Z; k++)
         cells[i][j][k].init(i, j, k);
   }

   void update_cells_in_use_list()
   {
      cells_in_use_count = 0;
      for(int i = 0; i < W_CELLS_NUM_X; i++)
      for(int j = 0; j < W_CELLS_NUM_Y; j++)
      for(int k = 0; k < W_CELLS_NUM_Z; k++)
      {
         auto cell = &cells[i][j][k];
         if(cell->count != 0)
         {
            cells_in_use[cells_in_use_count] = cell;
            cells_in_use_count++;
         }
      }
   }

   CellUpdate update_entity_world_cells(Entity* entity, vec3 pos_offset1 = vec3{0}, vec3 pos_offset2 = vec3{0})
   {
      // If entity is not an AABB, use pos_offset vectors to adjust x-z offsets from entity reference to
      // boundaries
      
      string message;

      // computes the new cells
      auto [x0, x1, z0, z1] = entity->get_rect_bounds();
      float height = entity->get_height();

      vec3 pos1 = vec3{x0, entity->position.y, z0} + pos_offset1;
      vec3 pos2 = vec3{x1, entity->position.y + height, z1} + pos_offset2;
      auto [i0, j0, k0] = get_world_cells_coords_from_world_coords(pos1);
      auto [i1, j1, k1] = get_world_cells_coords_from_world_coords(pos2);

      // out of bounds catch
      if (i0 == -1 || i1 == -1)
      {
        message = "Entity '" + entity->name + "' is located out of current world bounds.";
         return CellUpdate{OUT_OF_BOUNDS, message};
      }

      vector<WorldCell*> new_cells;
      for(int i = i0; i <= i1; i++)
      for(int j = j0; j <= j1; j++)
      for(int k = k0; k <= k1; k++)
         new_cells.push_back(&cells[i][j][k]);

      // entity too large catch
      if(new_cells.size() > ENTITY_WOLRD_CELL_OCCUPATION_LIMIT)
      {
         message = "Entity '" + entity->name + "' is too large and it occupies more than " +
            "the limit of " + to_string(ENTITY_WOLRD_CELL_OCCUPATION_LIMIT) + " world cells at a time.";
         return CellUpdate{ENTITY_TOO_BIG, message};
      }

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
      // PS: if this fails, entity will have no cells. Easier to debug.
      for(int i = 0; i < cells_to_add_to.size(); i++)
      {
         auto cell = cells_to_add_to[i];
         auto cell_update = cell->add(entity);
         if(cell_update.status != OK)
         {
            return cell_update;
         }
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

      bool changed_cells = cells_to_add_to.size() > 0 || cells_to_remove_from.size() > 0;
      return CellUpdate{OK, "", changed_cells};
   }
};







