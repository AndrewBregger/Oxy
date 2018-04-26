#include "ast.h"

std::vector<std::string> item_strings = {
#define ITEMKIND(n) #n,
	ITEMKINDS
#undef ITEMKIND
};

std::vector<std::string> expr_strings = {
#define EXPRKIND(n) #n,
	EXPRKINDS
#undef EXPRKIND
};

std::vector<std::string> type_strings = {
#define TYPESPECKIND(n) #n,
	TYPESPECKINDS
#undef TYPESPECKIND
};

std::vector<std::string> stmt_strings = {
#define STMTKIND(n) #n,
	STMTKINDS
#undef STMTKIND
};

Ident* new_ident(const std::string& name, SourceLoc loc) {
  Ident* ident = new Ident;
  ident->value = name;
  ident->loc = loc;
  return ident;
}

SourceLoc new_sourceloc(File* file, u64 line, u64 column, u64 span) {
	SourceLoc loc;
	loc.file = file;
	loc.line = line;
	loc.column = column;
	return loc;
}

TypeSpec* new_typespec(TypeSpecKind kind, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new TypeSpec;
	spec->kind = kind;
	spec->mut = mut;
	spec->loc = loc;
}

TypeSpec* new_name_typespec(Ident* name, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecName, mut, loc);
  // memory is allocated for the type spec
  spec->name = name;
  return spec;
}

TypeSpec* new_func_typespec(const std::vector<TypeSpec*>& args, TypeSpec* ret, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecFunc, None, loc);
  spec->funct.args = args;
  spec->funct.ret = ret;
  return spec;
}

TypeSpec* new_array_typespec(TypeSpec* elem, Expr* size, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecArray, mut, loc);
  spec->array.elem = elem;
  spec->array.size = size;
  return spec;
}

TypeSpec* new_ptr_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecPtr, mut, loc);
  spec->ptr.elem = elem;
  return spec;
}

TypeSpec* new_ref_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecRef, mut, loc);
  spec->ref.elem = elem;
  return spec;
}

TypeSpec* new_map_typespec(TypeSpec* key, TypeSpec* val, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecMap, mut, loc);
  spec->map.key = key;
  spec->map.value = val;
  return spec;
}

TypeSpec* new_tuple_typespec(std::vector<TypeSpec*> types, Mutablity mut, SourceLoc loc) {
	TypeSpec* spec = new_typespec(TypeSpecTuple, mut, loc);
  spec->tuple.types = types;
  return spec;
}

Stmt* new_stmt(StmtKind kind, SourceLoc loc) {
  Stmt* stmt = new Stmt;
  stmt->kind = kind;
  stmt->loc = loc;
  return stmt;
}

Stmt* new_expr_stmt(Expr* expr, SourceLoc loc) {
  Stmt* stmt = new_stmt(ExprStmt, loc);
  stmt->expr = expr;
  return stmt;
}

Stmt* new_item_stmt(Item* item, SourceLoc loc) {
  Stmt* stmt = new_stmt(ItemStmt, loc);
  stmt->item = item;
  return stmt;
}

Expr* new_expr(ExprKind kind, SourceLoc loc) {
  Expr* expr = new Expr;
  expr->kind = kind;
  expr->loc = loc;
  return expr;
}

Expr* new_name(Ident* name, SourceLoc loc) {
  Expr* expr = new_expr(Name, loc);
  expr->name = name;
  return expr;

}

Expr* new_literal(Token token, SourceLoc loc) {
  Expr* expr = new_expr(Literal, loc);
  expr->literal = token;
  return expr;
}

Expr* new_structliteral(TypeSpec* name, std::vector<Expr*> members, SourceLoc loc) {
  Expr* expr = new_expr(StructLiteral, loc);
  expr->struct_lit.name = name;
  expr->struct_lit.members = new Expr*[members.size()];
  expr->struct_lit.num_members = members.size();
  for(u32 i = 0;i < members.size(); ++i)
    expr->struct_lit.members[i] = members[i];
  return expr;
}

Expr* new_compoundliteral(std::vector<Expr*> members, SourceLoc loc) {
  Expr* expr = new_expr(CompoundLiteral, loc);
  expr->compound_lit.members = new Expr*[members.size()];
  expr->compound_lit.num_members = members.size();
  for(u32 i = 0;i < members.size(); ++i)
    expr->compound_lit.members[i] = members[i];
  return expr;
}

Expr* new_unary(Token op, Expr* e, SourceLoc loc) {
  Expr* expr = new_expr(Unary, loc);
  expr->unary.op = op;
  expr->unary.expr = e;
  return expr;
}

Expr* new_binary(Token op, Expr* lhs, Expr* rhs, SourceLoc loc) {
  Expr* expr = new_expr(Binary, loc);
  expr->binary.op = op;
  expr->binary.lhs = lhs;
  expr->binary.rhs = rhs;
  return expr;
}

Expr* new_fncall(Expr* name, std::vector<Expr*> actuals, SourceLoc loc) {
  Expr* expr = new_expr(FnCall, loc);
  expr->fncall.name = name;
  expr->fncall.actuals = new Expr*[actuals.size()];
  expr->fncall.num_actuals = actuals.size();
  for(u32 i = 0;i < actuals.size(); ++i)
    expr->fncall.actuals[i] = actuals[i];
  return expr;
}

Expr* new_field(Expr* operand, Ident* name, SourceLoc loc) {
  Expr* expr = new_expr(Field, loc);
  expr->field.operand = operand;
  expr->field.name = name;
  return expr;
}

Expr* new_dotfncall(Expr* operand, Ident* name, std::vector<Expr*> actuals, SourceLoc loc) {
  Expr* expr = new_expr(DotFnCall, loc);
  expr->dotcall.operand = operand;
  expr->dotcall.name = name;
  expr->dotcall.actuals = new Expr*[actuals.size()];
  expr->dotcall.num_actuals = actuals.size();
  for(u32 i = 0;i < actuals.size(); ++i)
    expr->dotcall.actuals[i] = actuals[i];
  return expr;
}

Expr* new_if(Expr* cond, Expr* body, Expr* else_if, SourceLoc loc) {
  Expr* expr = new_expr(If, loc);
  expr->if_expr.cond = cond;
  expr->if_expr.body = body;
  expr->if_expr.else_if = else_if;
  return expr;
}

//Expr* new_iflet() {
//  Expr* expr = new_expr(FnCall, loc);
//  return expr;
//}

Expr* new_while(Expr* cond, Expr* body, SourceLoc loc) {
  Expr* expr = new_expr(While, loc);
  expr->while_expr.cond = cond;
  expr->while_expr.body = body;
  return expr;
}

Expr* new_for(Expr* cond, Expr* body, SourceLoc loc) {
  Expr* expr = new_expr(For, loc);
  expr->for_expr.cond = cond;
  expr->for_expr.body = body;
  return expr;
}

Expr* new_return(std::vector<Expr*> exprs, SourceLoc loc) {
  Expr* expr = new_expr(Return, loc);
  expr->return_expr.exprs = new Expr*[exprs.size()];
  expr->return_expr.num_exprs = exprs.size();
  for(u32 i = 0;i < exprs.size(); ++i)
    expr->return_expr.exprs[i] = exprs[i];
  return expr;
}

Expr* new_break(Token token, SourceLoc loc) {
  Expr* expr = new_expr(Break, loc);
  expr->break_expr = token;
  return expr;
}

Expr* new_continue(Token token, SourceLoc loc) {
  Expr* expr = new_expr(Continue, loc);
  expr->continue_expr = token;
  return expr;
}

Expr* new_block(std::vector<Stmt*> stmts, SourceLoc loc) {
  Expr* expr = new_expr(Block, loc);
  expr->block.stmts= new Stmt*[stmts.size()];
  expr->block.num_stmts = stmts.size();
  for(u32 i = 0;i < stmts.size(); ++i)
    expr->block.stmts[i] = stmts[i];
  return expr;
}

Expr* new_binding(Expr* name, Expr* e, SourceLoc loc) {
  Expr* expr = new_expr(Binding, loc);
  expr->binding.name = name;
  expr->binding.binding = e;
  return expr;
}

Expr* new_in(Expr* in, Expr* e, SourceLoc loc) {
  Expr* expr = new_expr(In, loc);
  expr->in.in = in;
  expr->in.expr = e;
  return expr;
}

Expr* new_tuple(std::vector<Expr*> elems, SourceLoc loc) {
  Expr* expr = new_expr(Tuple, loc);
  expr->tuple.elems = new Expr*[elems.size()];
  expr->tuple.num_elems = elems.size();
  for(u32 i = 0;i < elems.size(); ++i)
    expr->tuple.elems[i] = elems[i];
  return expr;
}

Item* new_item(ItemKind kind, SourceLoc loc) {
  Item* item = new Item;
  item->kind = kind;
  item->loc = loc;
  return item;
}

Item* new_itemlocal(std::vector<Ident*> names, TypeSpec* type, Expr* init, Mutablity mut, SourceLoc loc) {
  Item* item = new_item(ItemLocal, loc);
  item->local.mut = mut;
  item->local.names = names;
  item->local.type = type;
  item->local.init = init;
  return item;
}

Item* new_itemfunction(Ident* name, std::vector<Item*> arguments, TypeSpec* ret, SourceLoc loc) {
  Item* item = new_item(ItemFunction, loc);
  item->function.name = name;
  item->function.arguments = arguments;
  item->function.ret = ret;
  return item;
}

Item* new_itemstruct(Ident* name, std::vector<Item*> fields, SourceLoc loc) {
  Item* item = new_item(ItemStruct, loc);
  item->structure.name = name;
  item->structure.fields = fields;
  return item;
}

Item* new_itemtuplestruct(Ident* name, std::vector<TypeSpec*> fields, SourceLoc loc) {
  Item* item = new_item(ItemTupleStruct, loc);
  item->tuplestruct.name = name;
  item->tuplestruct.fields = fields;
  return item;
}

Item* new_itemenum(std::vector<Item*> elems, SourceLoc loc) {
  Item* item = new_item(ItemEnum, loc);
  item->enumeration.elems = elems;
  return item;
}

Item* new_itemuse(Ident* name, std::vector<Expr*> members, SourceLoc loc) {
  Item* item = new_item(ItemUse, loc);
  item->use.name = name;
  item->use.members = members;
  return item;
}

Item* new_itemmodule(Ident* name, std::vector<Item*> memebers, SourceLoc loc) {
  Item* item = new_item(ItemModule, loc);
  item->module.name = name;
  item->module.members = memebers;
  return item;
}

Item* new_itemfield(TypeSpec* type, Ident* name, Expr* init, SourceLoc loc) {
  Item* item = new_item(ItemField, loc);
  item->field.type = type;
  item->field.name = name;
  item->field.init = init;
  return item;
}

const char* item_string(ItemKind kind) {
	return item_strings[kind].c_str();
}

const char* expr_string(ExprKind kind) {
	return expr_strings[kind].c_str();
}

const char* stmt_string(StmtKind kind) {
	return stmt_strings[kind].c_str();
}

const char* typespec_string(TypeSpecKind kind) {
	return type_strings[kind].c_str();
}

void destroy_expr(Expr* expr) {
	switch(expr->kind) {
    case Name: {

    } break;
    case Literal: {

    } break;
    case StructLiteral: {

    } break;
    case CompoundLiteral: {

    } break;
    case Unary: {

    } break;
    case Binary: {

    } break;
    case FnCall: {

    } break;
    case Field: {

    } break;
    case DotFnCall: {

    } break;
    case If: {

    } break;
    case IfLet: {

    } break;
    case While: {

    } break;
    case For: {

    } break;
    case Match: {

    } break;
    case Return: {

    } break;
    case Break: {

    } break;
    case Continue: {

    } break;
    case Block: {

    } break;
    case Binding: {

    } break;
    case In: {

    } break;
    case Tuple: {

    } break;
    case PatternExpr: {

    } break;
  }
}

void destroy_item(Item* item) {
	switch(item->kind) {
    case ItemLocal: {

    } break;
    case ItemFunction: {

    } break;
    case ItemStruct: {

    } break;
    case ItemTupleStruct: {

    } break;
    case ItemEnum: {

    } break;
    case ItemUse: {

    } break;
    case ItemModule: {

    } break;
    case ItemField: {

    } break;
  }
}

void destroy_stmt(Stmt* stmt) {
  switch(stmt->kind) {
    case ExprStmt:;
    case ItemStmt:;
  }
}

void destroy_typesepc(TypeSpec* spec) {
  switch(spec->kind) {
    case TypeSpecNone: {

    } break;
    case TypeSpecName: {

    } break;
    case TypeSpecFunc: {

    } break;
    case TypeSpecArray: {

    } break;
    case TypeSpecPtr: {

    } break;
    case TypeSpecRef: {

    } break;
    case TypeSpecMap: {

    } break;
    case TypeSpecTuple: {

    } break;
  }
}
