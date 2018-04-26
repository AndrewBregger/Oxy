#ifndef AST_H_
#define AST_H_

#include "common.h"
#include "token.h"
#include <vector>

struct Expr;
struct Stmt;
struct Item;
struct Type;

struct File;
struct Pattern;

typedef std::vector<Item*> ItemSet;

// node strings

// extern std::vector<std::string> item_strings;
// extern std::vector<std::string> expr_strings;
// extern std::vector<std::string> type_strings;
// extern std::vector<std::string> stmt_strings;





#define TYPESPECKINDS \
  TYPESPECKIND(TypeSpecNone) \
  TYPESPECKIND(TypeSpecName) \
  TYPESPECKIND(TypeSpecFunc) \
  TYPESPECKIND(TypeSpecArray) \
  TYPESPECKIND(TypeSpecPtr) \
  TYPESPECKIND(TypeSpecRef) \
  TYPESPECKIND(TypeSpecMap) \
  TYPESPECKIND(TypeSpecTuple)


enum TypeSpecKind {
#define TYPESPECKIND(n) n,
  TYPESPECKINDS
#undef TYPESPECKIND
};

enum Mutablity {
  None,
  Immutable,
  Mutable,
};

struct SourceLoc {
  File* file;
  u64 line;
  u64 column;
  u64 span;
};

struct Ident {
  SourceLoc loc;  
  std::string value;
};

Ident* new_ident(const std::string& name, SourceLoc loc);


struct TypeSpec {
  TypeSpec() {}
  TypeSpecKind kind;
  Mutablity mut;
  SourceLoc loc;
  Type* type;

  union {
    Ident* name;
    struct {
      std::vector<TypeSpec*> args;
      TypeSpec* ret;
    } funct;
    struct {
      TypeSpec* elem;
      Expr* size;
    } array;
    struct {
      TypeSpec* elem;
    } ptr;
    struct {
      TypeSpec* elem;
    } ref;
    struct {
      TypeSpec* key;
      TypeSpec* value;
    } map;
    struct {
      std::vector<TypeSpec*> types;
    } tuple;
  };
};

SourceLoc new_sourceloc(File* file, u64 line, u64 column, u64 span);


TypeSpec* new_typespec(TypeSpecKind kind, Mutablity mut, SourceLoc loc);

TypeSpec* new_name_typespec(Ident* name, Mutablity mut, SourceLoc loc);

TypeSpec* new_func_typespec(const std::vector<TypeSpec*>& args, TypeSpec* ret,
  SourceLoc loc);

TypeSpec* new_array_typespec(TypeSpec* elem, Expr* size, Mutablity mut, SourceLoc loc);

TypeSpec* new_ptr_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc);

TypeSpec* new_ref_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc);

TypeSpec* new_map_typespec(TypeSpec* key, TypeSpec* val, Mutablity mut, SourceLoc loc);

TypeSpec* new_tuple_typespec(std::vector<TypeSpec*> types, Mutablity mut, SourceLoc loc);

#define STMTKINDS \
  STMTKIND(ExprStmt) \
  STMTKIND(ItemStmt)

enum StmtKind {
  #define STMTKIND(n) n,
    STMTKINDS
  #undef STMTKIND
};

struct Stmt {
  StmtKind kind;
  SourceLoc loc;

  Stmt() {}

  union{
    Expr* expr;
    Item* item;
  };
};

Stmt* new_stmt(StmtKind kind, SourceLoc loc);

Stmt* new_expr_stmt(Expr* expr, SourceLoc loc);

Stmt* new_item_stmt(Item* item, SourceLoc loc);

#define EXPRKINDS \
  EXPRKIND(Name) \
  EXPRKIND(Literal) \
  EXPRKIND(StructLiteral) \
  EXPRKIND(CompoundLiteral) \
  EXPRKIND(Unary) \
  EXPRKIND(Binary) \
  EXPRKIND(FnCall) \
  EXPRKIND(Field) \
  EXPRKIND(DotFnCall) \
  EXPRKIND(If) \
  EXPRKIND(IfLet) \
  EXPRKIND(While) \
  EXPRKIND(For) \
  EXPRKIND(Match) \
  EXPRKIND(Return) \
  EXPRKIND(Break) \
  EXPRKIND(Continue) \
  EXPRKIND(Block) \
  EXPRKIND(Binding) \
  EXPRKIND(In) \
  EXPRKIND(Tuple) \
  EXPRKIND(PatternExpr)

enum ExprKind {
  #define EXPRKIND(n) n,
    EXPRKINDS
  #undef EXPRKIND
};

struct Expr {
  ExprKind kind;
  SourceLoc loc;

  Expr() {}
  union {
    Ident* name;
    Token literal;
    struct {
      TypeSpec* name;
      Expr** members;
      u32 num_members;
    } struct_lit;
    struct {
      Expr** members;
      u32 num_members;
    } compound_lit;
    struct {
      Token op;
      Expr* expr;
    } unary;
    struct {
      Token op;
      Expr* lhs;
      Expr* rhs;
    } binary;
    struct {
      Expr* name;
      Expr** actuals;
      u32 num_actuals;
    } fncall;
    struct {
      Expr* operand;
      Ident* name;
    } field;
    struct {
      Expr* operand;
      Ident* name;
      Expr** actuals;
      u32 num_actuals;
    } dotcall;
    // standard if expression
    struct {
      Expr* cond;
      Expr* body;
      Expr* else_if;
    } if_expr;
    // match if expression
    struct {
      Expr* cond;
      Expr** body;
      u32 num_body;
    } matchif_expr;
    struct {
      Expr* cond;
      Expr* body;
    } while_expr;
    struct {
      Expr* cond;
      Expr* body;
    } for_expr;
    Token continue_expr;
    Token break_expr;
    struct {
      Expr** exprs;
      u32 num_exprs;
    } return_expr;
    struct {
      Stmt** stmts;
      u32 num_stmts;
    } block;
    struct {
      Expr* name;
      Expr* binding;
    } binding;
    struct {
      Expr* in;
      Expr* expr;
    } in;
    struct {
      Expr** elems;
      u32 num_elems;
    } tuple;
    struct {
      Pattern* pat;
    } pattern;
  };
};

Expr* new_expr(ExprKind kind, SourceLoc loc);
Expr* new_name(Ident* name, SourceLoc loc);
Expr* new_literal(Token token, SourceLoc loc);
Expr* new_structliteral(TypeSpec* name, std::vector<Expr*> member, SourceLoc loc);
Expr* new_compoundliteral(std::vector<Expr*> member, SourceLoc loc);
Expr* new_unary(Token op, Expr* expr, SourceLoc loc);
Expr* new_binary(Token op, Expr* lhs, Expr* rhs, SourceLoc loc);
Expr* new_fncall(Expr* name, std::vector<Expr*> actuals, SourceLoc loc);
Expr* new_field(Expr* operand, Ident* name, SourceLoc loc);
Expr* new_dotfncall(Expr* operand, Ident* name, std::vector<Expr*> actuals,
  SourceLoc loc);
Expr* new_if(Expr* cond, Expr* body, Expr* else_if, SourceLoc loc);
Expr* new_iflet();
Expr* new_while(Expr* cond, Expr* body, SourceLoc loc);
Expr* new_for(Expr* cond, Expr* body, SourceLoc loc);
Expr* new_match();
Expr* new_return(std::vector<Expr*> expr, SourceLoc loc);
Expr* new_break(Token token, SourceLoc loc);
Expr* new_continue(Token token, SourceLoc loc);
Expr* new_block(std::vector<Stmt*> stmts, SourceLoc loc);
Expr* new_binding(Expr* name, Expr* expr, SourceLoc loc);
Expr* new_in(Expr* in, Expr* expr, SourceLoc loc);
Expr* new_tuple(std::vector<Expr*> elem, SourceLoc loc);

#define ITEMKINDS \
  ITEMKIND(ItemLocal) \
  ITEMKIND(ItemFunction) \
  ITEMKIND(ItemStruct) \
  ITEMKIND(ItemTupleStruct) \
  ITEMKIND(ItemEnum) \
  ITEMKIND(ItemUse) \
  ITEMKIND(ItemModule) \
  ITEMKIND(ItemField)

enum ItemKind {
#define ITEMKIND(n) n,
  ITEMKINDS
#undef ITEMKIND
};

struct Item {
  ItemKind kind;
  SourceLoc loc;

  Item() {}

  union {
    struct {
      Mutablity mut;
      std::vector<Ident*> names;
      TypeSpec* type;
      Expr* init;
    } local;
    struct {
      Ident* name;
      std::vector<Item*> arguments;
      TypeSpec* ret;
    } function;
    struct {
      Ident* name;
      std::vector<Item*> fields;
    } structure;
    struct {
      Ident* name;
      std::vector<TypeSpec*> fields;
    } tuplestruct;
    struct {
      std::vector<Item*> elems;
    } enumeration;
    struct {
      Ident* name;
      std::vector<Expr*> members;
    } use;
    struct {
      Ident* name;
      std::vector<Item*> members;
    } module;
    struct {
      TypeSpec* type;
      Ident* name;
      Expr* init;
    } field;
  };
};

Item* new_item(ItemKind kind, SourceLoc loc);
Item* new_itemlocal(std::vector<Ident*> names, TypeSpec* type, Expr* init, Mutablity mut, SourceLoc loc);
Item* new_itemfunction(Ident* name, std::vector<Item*> arguments, TypeSpec* ret, SourceLoc loc);
Item* new_itemstruct(Ident* name, std::vector<Item*> fields, SourceLoc loc);
Item* new_itemtuplestruct(Ident* name, std::vector<TypeSpec*> fields, SourceLoc loc);
Item* new_itemenum(std::vector<Item*> elems, SourceLoc loc);
Item* new_itemuse(Ident* name, std::vector<Expr*> memebers, SourceLoc loc);
Item* new_itemmodule(Ident* name, std::vector<Item*> memebers, SourceLoc loc);
Item* new_itemfield(TypeSpec* type, Ident* name, Expr* init, SourceLoc loc);

#define PATTERNKINDS


enum PatternKind {

};

struct Pattern {
  PatternKind kind;
  SourceLoc loc;

  union {

  };
};

const char* item_string(ItemKind kind);
const char* expr_string(ExprKind kind);
const char* stmt_string(StmtKind kind);
const char* typespec_string(TypeSpecKind kind);

void destroy_expr(Expr* expr);
void destroy_item(Item* expr);
void destroy_stmt(Stmt* expr);
void destroy_typesepc(TypeSpec* expr);

#endif
