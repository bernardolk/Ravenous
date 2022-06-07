
struct EntityPool {

   Entity* pool;
   const int size;
   int count = 0;

   EntityPool(const int size) : size(size)
   {

   };

   void init()
   {
      // allocate pool memory
      pool = (Entity*) malloc(size * sizeof(Entity));
      if(pool == NULL)
      {
         std::cout << "FATAL: failed to allocate memory for EntityPool.\n";
         assert(false);
      }

      For(size)
      {
         // initializes entity with "placement new"
         auto _ = new (pool + i) Entity();
         // marks entity as "empty"
         pool[i].flags |= EntityFlags_EmptyEntity;
      }
   }
   
   Entity* get_next()
   {
      For(size)
      {
         if(pool[i].flags & EntityFlags_EmptyEntity)
         {
            pool[i].flags &= ~(EntityFlags_EmptyEntity);
            return &pool[i];
         }
      }

      std::cout << "EntityPool is full!\n";
      assert(false);
      return nullptr;
   }

   void free_slot(Entity* entity)
   {
      Entity* cursor = pool;
      bool deleted = false;
      For(size)
      {
         if(cursor->id == entity->id)
         {
            // inits new entity with "placement new"
            auto _ = new (cursor) Entity();
            cursor->flags |= EntityFlags_EmptyEntity;
            deleted = true;
            break;
         }
         cursor++;
      }

      if(!deleted)
         log(LOG_WARNING, "Entity '" + entity->name + "' requested to be deleted couldn't be found in entity pool.");
   }
};



