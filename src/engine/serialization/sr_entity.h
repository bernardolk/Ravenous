#pragma once

struct EntityManager;
struct Entity;

namespace Parser
{
    struct ParseUnit;
}

enum SrEntityRelation {
    SrEntityRelation_TimerTarget    = 0,
    SrEntityRelation_TimerMarking   = 1
 };

// Allows storing relationships between parsed entities to be set after parsing is done
struct DeferredEntityRelationBuffer {
    static constexpr int    size = 64;
    int                     count = 0;
    Entity*                 entities[size]{};
    u64                     deferred_entity_ids[size]{};
    SrEntityRelation        relations[size]{};
    u32                     aux_uint_buffer[size]{};
};

struct EntitySerializer
{
    EntityManager&                  manager;
    DeferredEntityRelationBuffer    relations;
    
    void parse(Parser::ParseUnit p) const;
    void save(Entity& entity) const;
};
