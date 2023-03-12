#pragma once

// macro magic
#define ENTITY1(TypeName)                               TypeName : E_BaseEntity, T_EntityTypeHelper<TypeName>
#define ENTITY2(TypeName, Trait1)                       TypeName : E_BaseEntity, T_EntityTypeHelper<TypeName>, T_Trait<TypeName, Trait1>
#define ENTITY3(TypeName, Trait1, Trait2)               TypeName : E_BaseEntity, T_EntityTypeHelper<TypeName>, T_Trait<TypeName, Trait1>, T_Trait<TypeName, Trait2>
#define ENTITY4(TypeName, Trait1, Trait2, Trait3)       TypeName : E_BaseEntity, T_EntityTypeHelper<TypeName>, T_Trait<TypeName, Trait1>, T_Trait<TypeName, Trait2>, T_Trait<TypeName, Trait3>

#define GET_MACRO(_1,_2,_3,_4,NAME,...) NAME
#define Entity(...) GET_MACRO(__VA_ARGS__, ENTITY4, ENTITY3, ENTITY2, ENTITY1)(__VA_ARGS__)

struct E_BaseEntity
{ 
	int type_id;
	int id;
	float position[3];
};
