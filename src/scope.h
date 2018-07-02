#ifndef SCOPE_H_
#define SCOPE_H_

#include "common.h"
#include "entity.h"

typedef enum ScopeKind {
	Scope_None,
	Scope_Global,
	Scope_File,
	Scope_Module,
	Scope_Struct,
	Scope_Function,
	Scope_Param,
	Scope_Block
} ScopeKind;

typedef struct Scope {
	ScopeKind kind;			//< the type of scope
	Map elements; 			//< char* -> Entity*
	struct Scope* parent; 			//< null if global
	struct Scope** children;
	u32 num_children;
} Scope;

Scope* new_scope(ScopeKind kind, Scope* parent);

bool has_name(Scope* scope, const char* name);

Entity* add_entity(Scope* scope, const char* name, Entity* entity);

Entity* get_entity(Scope* scope, const char* name);

void add_child_scope(Scope* parent, Scope* child);



#endif