#include <engine/core/rvn_types.h>
#include <iostream> // utils
#include <string>
#include <engine/vertex.h> // utils
#include <algorithm> // utils
#include <utils.h>
#include <rvn_macros.h>
#include <engine/collision/primitives/bounding_box.h>
#include <vector>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <engine/mesh.h>
#include <glm/gtx/normal.hpp>
#include <engine/collision/primitives/triangle.h>
#include <engine/entity.h>
#include <engine/world/world.h>


void WorldCell::init(int ii, int ji, int ki)
{
   this->count = 0;

   // set logical coordinates
   this->i = ii;
   this->j = ji;
   this->k = ki;

   // set physical world coordinates to bounding box
   vec3 origin = get_world_coordinates_from_world_cell_coordinates(ii, ji, ki);
   this->bounding_box.minx = origin.x;
   this->bounding_box.miny = origin.y;
   this->bounding_box.minz = origin.z;
   this->bounding_box.maxx = origin.x + W_CELL_LEN_METERS;
   this->bounding_box.maxy = origin.y + W_CELL_LEN_METERS;
   this->bounding_box.maxz = origin.z + W_CELL_LEN_METERS;

   // initialize entities list
   for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
      this->entities[i] = nullptr;
}

void WorldCell::remove(Entity* entity)
{
   for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
      if(this->entities[i] == entity)
      {
         this->entities[i] = nullptr;
         this->defrag();
         return;
      }
}

CellUpdate WorldCell::add(Entity* entity)
{
   if(count == WORLD_CELL_CAPACITY)
   {
   std::string message = "World cell '" + this->coords_str() + "' is full.";
      return CellUpdate{ CellUpdate_CELL_FULL, message };
   }

   for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
      if(this->entities[i] == nullptr)
      {
         this->entities[i] = entity;
         this->count++;
         return CellUpdate{ CellUpdate_OK };
      }

   return CellUpdate{ CellUpdate_UNEXPECTED, "world cell add method returned weirdly." };
}

//@TODO: Refactor this whole thing to use mempool
void WorldCell::defrag()
{
   if(this->count == 0)
      return;

   // initialize holes array
   unsigned int hole_count = 0;
   int holes[WORLD_CELL_CAPACITY];
   for(int i = 0; i < WORLD_CELL_CAPACITY; i++)
      holes[i] = -1;

   // find holes and store in array
   // also count how many items there are
   int new_count = 0;
   for(int i = 0; i < this->count; i++)
   {
      // we dont want to count the last empty spot as a hole
      if(this->entities[i] == nullptr)
      {
         if(i + 1 != this->count)
            holes[hole_count++] = i;
      }
      else 
         new_count++;
   }

   // loop through list from top to bottom and fill
   // holes as it finds candidates to swap      
   int idx = this->count - 1; 
   int hole_idx = 0;
   while(true)
   {
      int hole = holes[hole_idx];
      if(hole == -1 || idx == 0)
         break;

      auto item = this->entities[idx];
      if(item != nullptr)
      {
         this->entities[hole] = item;
         this->entities[idx] = nullptr;
         hole_idx++;
      }
      idx--;
   }
   this->count = new_count;
}

std::string WorldCell::coords_str()
{
   return "Cell [" + std::to_string(this->i) 
      + "," + std::to_string(this->j) + "," + std::to_string(this->k) 
      + "] (" + std::to_string(this->count) + ")";
}

vec3 WorldCell::coords()
{
   return vec3{this->i, this->j, this->k};
}

vec3 WorldCell::coords_meters()
{
   return get_world_coordinates_from_world_cell_coordinates(this->i, this->j, this->k);
}

std::string WorldCell::coords_meters_str()
{
   vec3 mcoords = this->coords_meters();
   return "[x: " + format_float_tostr(mcoords[0], 1) 
      + ", y: " + format_float_tostr(mcoords[1], 1) + ", z: " + format_float_tostr(mcoords[2], 1) 
      + "]";
}