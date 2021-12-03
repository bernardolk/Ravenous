
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
         cout << "FATAL: failed to allocate memory for EntityPool.\n";
         assert(false);
      }

      for(int i = 0; i < size; i++)
      {
         // initializes entity with "placement new"
         auto _ = new (pool + i) Entity();
         // marks entity as "empty"
         pool[i].flags |= EntityFlags_EmptyEntity;
      }
   }
   
   Entity* get_next()
   {
      for(int i = 0; i < size; i++)
      {
         if(pool[i].flags & EntityFlags_EmptyEntity)
         {
            pool[i].flags &= ~(EntityFlags_EmptyEntity);
            return &pool[i];
         }
      }

      cout << "EntityPool is full!\n";
      assert(false);
      return nullptr;
   }
};



