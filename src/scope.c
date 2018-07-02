#include "scope.h"


Scope* new_scope(ScopeKind kind, Scope* parent) {
	Scope* scope = (Scope*) malloc(sizeof(Scope));
	scope->kind = kind;
	scope->parent = parent;
	scope->children = NULL;
	scope->num_children = 0;
	return scope;
}

bool has_name(Scope* scope, const char* name) {
	return map_get(&scope->elements, name) != NULL;
}

Entity* add_entity(Scope* scope, const char* name, Entity* entity) {
	if(has_name(scope, name)) {
		Entity* en = get_entity(scope, name);
		if(en == entity)
			return en;
		else {
			return NULL;
		}
	}
	else {
		map_put(&scope->elements, name, entity);
		return entity;
	}
}

Entity* get_entity(Scope* scope, const char* name) {
	Scope* curr = scope;

	while(curr) {
		Entity* e = (Entity*) map_get(&curr->elements, name);
		if(e)
			return e;

		curr = curr->parent;
	}

	return NULL;
}

void add_child_scope(Scope* parent, Scope* child) {
	buf_push(parent->children, child);
}
