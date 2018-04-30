#ifndef AST_H_
#define AST_H_

#include "common.h"
#include "token.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Item Item;
typedef struct Type Type;
typedef struct File File;
typedef struct Pattern Pattern;
typedef struct TypeSpec TypeSpec;


// node strings

// extern std::string> item_strings;
// extern std::string> expr_strings;
// extern std::string> type_strings;
// extern std::string> stmt_strings;





#define TYPESPECKINDS \
  TYPESPECKIND(TypeSpecNone) \
  TYPESPECKIND(TypeSpecName) \
  TYPESPECKIND(TypeSpecPath) \
  TYPESPECKIND(TypeSpecFunc) \
  TYPESPECKIND(TypeSpecArray) \
  TYPESPECKIND(TypeSpecPtr) \
  TYPESPECKIND(TypeSpecRef) \
  TYPESPECKIND(TypeSpecMap) \
  TYPESPECKIND(TypeSpecTuple)


typedef enum TypeSpecKind {
#define TYPESPECKIND(n) n,
  TYPESPECKINDS
#undef TYPESPECKIND
} TypeSpecKind;

typedef enum Mutablity {
  None,
  Immutable,
  Mutable,
} Mutablity;

typedef struct SourceLoc {
  File* file;
  u64 line;
  u64 column;
  u64 span;
} SourceLoc;

typedef struct Ident {
  SourceLoc loc;  
  char* value;
} Ident;

Ident* new_ident(const char* name, SourceLoc loc);


typedef struct TypeSpec {
  TypeSpecKind kind;
  Mutablity mut;
  SourceLoc loc;
  Type* type;

  union {
    struct {
      Ident* name;
      // generic params
    } name;
    struct {
      TypeSpec* parent;
      TypeSpec* elem;
    } path;
    struct {
      TypeSpec** args;
      u32 num_args;
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
      TypeSpec** types;
      u32 num_types;
    } tuple;
  };
} TypeSpec;

SourceLoc new_sourceloc(File* file, u64 line, u64 column, u64 span);

TypeSpec* new_typespec(TypeSpecKind kind, Mutablity mut, SourceLoc loc);

TypeSpec* new_name_typespec(Ident* name, Mutablity mut, SourceLoc loc);

TypeSpec* new_path_typespec(TypeSpec* parent, TypeSpec* elem, Mutablity mut, SourceLoc loc);

TypeSpec* new_func_typespec(TypeSpec** args, TypeSpec* ret,
  SourceLoc loc);

TypeSpec* new_array_typespec(TypeSpec* elem, Expr* size, Mutablity mut, SourceLoc loc);

TypeSpec* new_ptr_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc);

TypeSpec* new_ref_typespec(TypeSpec* elem, Mutablity mut, SourceLoc loc);

TypeSpec* new_map_typespec(TypeSpec* key, TypeSpec* val, Mutablity mut, SourceLoc loc);

TypeSpec* new_tuple_typespec(TypeSpec** types, Mutablity mut, SourceLoc loc);

#define STMTKINDS \
  STMTKIND(ExprStmt) \
  STMTKIND(SemiStmt) \
  STMTKIND(ItemStmt)

typedef enum StmtKind {
  #define STMTKIND(n) n,
    STMTKINDS
  #undef STMTKIND
} StmtKind;

typedef struct Stmt {
  StmtKind kind;
  SourceLoc loc;


  union{
    Expr* expr;
    Expr* semi;
    Item* item;
  };
} Stmt;

Stmt* new_stmt(StmtKind kind, SourceLoc loc);

Stmt* new_expr_stmt(Expr* expr, SourceLoc loc);

Stmt* new_semi_stmt(Expr* expr, SourceLoc loc);

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
  EXPRKIND(PatternExpr) \
  EXPRKIND(Assignment)

typedef enum ExprKind {
  #define EXPRKIND(n) n,
    EXPRKINDS
  #undef EXPRKIND
} ExprKind;

typedef struct Expr {
  ExprKind kind;
  SourceLoc loc;

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
      Expr* name;
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
    struct {
      Token op;
      Expr* variable;
      Expr* value;
    } assign;
  };
} Expr;

Expr* new_expr(ExprKind kind, SourceLoc loc);
Expr* new_name(Ident* name, SourceLoc loc);
Expr* new_literal(Token token, SourceLoc loc);
Expr* new_structliteral(TypeSpec* name, Expr** member, SourceLoc loc);
Expr* new_compoundliteral(Expr** member, SourceLoc loc);
Expr* new_unary(Token op, Expr* expr, SourceLoc loc);
Expr* new_binary(Token op, Expr* lhs, Expr* rhs, SourceLoc loc);
Expr* new_fncall(Expr* name, Expr** actuals, SourceLoc loc);
Expr* new_field(Expr* operand, Ident* name, SourceLoc loc);
Expr* new_dotfncall(Expr* operand, Expr* name, Expr** actuals,
  SourceLoc loc);
Expr* new_if(Expr* cond, Expr* body, Expr* else_if, SourceLoc loc);
Expr* new_iflet();
Expr* new_while(Expr* cond, Expr* body, SourceLoc loc);
Expr* new_for(Expr* cond, Expr* body, SourceLoc loc);
Expr* new_match();
Expr* new_return(Expr** expr, SourceLoc loc);
Expr* new_break(Token token, SourceLoc loc);
Expr* new_continue(Token token, SourceLoc loc);
Expr* new_block(Stmt** stmts, SourceLoc loc);
Expr* new_binding(Expr* name, Expr* expr, SourceLoc loc);
Expr* new_in(Expr* in, Expr* expr, SourceLoc loc);
Expr* new_tuple(Expr** elem, SourceLoc loc);
Expr* new_assign(Token op, Expr* variable, Expr* value, SourceLoc loc);

#define ITEMKINDS \
  ITEMKIND(ItemLocal) \
  ITEMKIND(ItemFunction) \
  ITEMKIND(ItemStruct) \
  ITEMKIND(ItemTupleStruct) \
  ITEMKIND(ItemEnum) \
  ITEMKIND(ItemUse) \
  ITEMKIND(ItemModule) \
  ITEMKIND(ItemField)

typedef enum ItemKind {
#define ITEMKIND(n) n,
  ITEMKINDS
#undef ITEMKIND
} ItemKind;

typedef struct Item {
  ItemKind kind;
  SourceLoc loc;


  union {
    struct {
      Mutablity mut;
      Ident** names;
      u32 num_names;
      TypeSpec* type;
      Expr* init;
    } local;
    struct {
      Ident* name;
      Item** arguments;
      u32 num_args;
      TypeSpec* ret;
    } function;
    struct {
      Ident* name;
      Item** fields;
      u32 num_fields;
    } structure;
    struct {
      Ident* name;
      TypeSpec** fields;
      u32 num_fields;
    } tuplestruct;
    struct {
      Item** elems;
      u32 num_elems;
    } enumeration;
    struct {
      Ident* name;
      Expr** members;
      u32 num_members;
    } use;
    struct {
      Ident* name;
      Item** members;
      u32 num_members;
    } module;
    struct {
      TypeSpec* type;
      Ident* name;
      Expr* init;
    } field;
  };
} Item;

Item* new_item(ItemKind kind, SourceLoc loc);
Item* new_itemlocal(Ident** names, TypeSpec* type, Expr* init, Mutablity mut, SourceLoc loc);
Item* new_itemfunction(Ident* name, Item** arguments, TypeSpec* ret, SourceLoc loc);
Item* new_itemstruct(Ident* name, Item** fields, SourceLoc loc);
Item* new_itemtuplestruct(Ident* name, TypeSpec** fields, SourceLoc loc);
Item* new_itemenum(Item** elems, SourceLoc loc);
Item* new_itemuse(Ident* name, Expr** memebers, SourceLoc loc);
Item* new_itemmodule(Ident* name, Item** memebers, SourceLoc loc);
Item* new_itemfield(TypeSpec* type, Ident* name, Expr* init, SourceLoc loc);

#define PATTERNKINDS \
  PATTERNKIND(WildCard)\
  PATTERNKIND(StructPattern) \
  PATTERNKIND(TuplePattern) \
  PATTERNKIND(RefPatter) \
  PATTERNKIND(PointerPattern) \
  PATTERNKIND(IdentPattern)


typedef enum PatternKind {
  #define PATTERNKIND(x) x,
  PATTERNKINDS
  #undef PATTERNKIND
} PatternKind;

typedef struct Pattern {
  PatternKind kind;
  SourceLoc loc;

  union {
    Token wildcard;
    Ident* ident;
    struct {
      TypeSpec* path;
      Pattern** elems;
      u32 num_elems; 
    } structure;
    struct {
      TypeSpec* path;
      Pattern** elems;
      u32 num_elems; 
    } tuple;
    struct {
      Mutablity mut;
      Pattern* pat;
    } ref;
    struct {
      Mutablity mut;
      Pattern* pat;
    } ptr;
  };
} Pattern;


const char* item_string(ItemKind kind);
const char* expr_string(ExprKind kind);
const char* stmt_string(StmtKind kind);
const char* typespec_string(TypeSpecKind kind);

void destroy_expr(Expr* expr);
void destroy_item(Item* expr);
void destroy_stmt(Stmt* expr);
void destroy_typesepc(TypeSpec* expr);

#endif
