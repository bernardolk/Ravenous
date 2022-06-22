#pragma once

auto world_coords_to_cells(float x, float y, float z);
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
const static float W_CELL_LEN_METERS = 50.0f;
// how many entities can coexists in a cell
const static int WORLD_CELL_CAPACITY = 150;

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
   CellUpdate_OK,
   CellUpdate_ENTITY_TOO_BIG,
   CellUpdate_CELL_FULL,
   CellUpdate_OUT_OF_BOUNDS,
   CellUpdate_UNEXPECTED
};

struct CellUpdate {
   CellUpdateStatus status;
  std::string message;
   bool entity_changed_cell;
};

struct Entity;
struct BoundingBox;


// -----------
// WORLD CELL
// -----------
// World cells are the way we spatially partition the game world. These are AABB's that contain a list of entities that touch them.
// Entities also have a list of the world cells they are currently living in.
// The reason is that in this game, things are more static than dynamic, so performing work on the lists on position changes is not
// that expensive (because it doesn't happen as often) and is useful to have a entity->world_cells link like this for various checks.
// WORLD CELLS ORIGINS LIE TO THE NEGATIVE AND GROW TOWARDS THE POSITIVE DIRECTION OF THEIR AXIS.
// So index 0 is at -50 (e.g.) and the cell extends until -40 (when we have length = 10).

struct WorldCell {
   Entity* entities[WORLD_CELL_CAPACITY];
   unsigned int count;
   
   // logical coords
   int i = -1, j = -1, k = -1;

   // world coords / bounding box
   BoundingBox bounding_box;

   void init(int ii, int ji, int ki);
   void remove(Entity* entity);
   CellUpdate add(Entity* entity);
   void defrag();
   std::string coords_str();
   vec3 coords();
   vec3 coords_meters();
   std::string coords_meters_str();
};

// -------------------
// COORDINATE METHODS
// -------------------

inline vec3 get_world_coordinates_from_world_cell_coordinates(int i, int j, int k)
{
   float world_x = (i - W_CELLS_OFFSET_X) * W_CELL_LEN_METERS;
   float world_y = (j - W_CELLS_OFFSET_Y) * W_CELL_LEN_METERS;
   float world_z = (k - W_CELLS_OFFSET_Z) * W_CELL_LEN_METERS;

   return vec3{world_x, world_y, world_z};
}


inline auto world_coords_to_cells(float x, float y, float z)
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

inline auto world_coords_to_cells(vec3 position)
{
   return world_coords_to_cells(position.x, position.y, position.z);
}


// ----------------
// > WORLD
// ----------------


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

   CellUpdate update_entity_world_cells(Entity* entity)
   {
     std::string message;

      // computes which cells the entity is occupying based on it's axis aligned bounding box
      auto [bb_min, bb_max]   = entity->bounding_box.bounds();
      auto [i0, j0, k0]       = world_coords_to_cells(bb_min);
      auto [i1, j1, k1]       = world_coords_to_cells(bb_max);

      // out of bounds catch
      if (i0 == -1 || i1 == -1)
      {
        message = "Entity '" + entity->name + "' is located out of current world bounds.";
         return CellUpdate{CellUpdate_OUT_OF_BOUNDS, message};
      }

      std::vector<WorldCell*> new_cells;
      for(int i = i0; i <= i1; i++)
      for(int j = j0; j <= j1; j++)
      for(int k = k0; k <= k1; k++)
         new_cells.push_back(&cells[i][j][k]);

      // entity too large catch
      if(new_cells.size() > ENTITY_WOLRD_CELL_OCCUPATION_LIMIT)
      {
         message = "Entity '" + entity->name + "' is too large and it occupies more than " +
            "the limit of " + std::to_string(ENTITY_WOLRD_CELL_OCCUPATION_LIMIT) + " world cells at a time.";
         return CellUpdate{CellUpdate_ENTITY_TOO_BIG, message};
      }

      // computes outdated world cells to remove the entity from
      std::vector<WorldCell*> cells_to_remove_from;
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

      // computes the cells to add entity to
      std::vector<WorldCell*> cells_to_add_to;
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
         if(cell_update.status != CellUpdate_OK)
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
      return CellUpdate{CellUpdate_OK, "", changed_cells};
   }
};







