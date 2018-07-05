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

typedef enum Mutability {
  None,
  Immutable,
  Mutable,
} Mutability;

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
  Mutability mut;
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
      // Expr* size;
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

TypeSpec* new_typespec(TypeSpecKind kind, Mutability mut, SourceLoc loc);

TypeSpec* new_name_typespec(Ident* name, Mutability mut, SourceLoc loc);

TypeSpec* new_path_typespec(TypeSpec* parent, TypeSpec* elem, Mutability mut, SourceLoc loc);

TypeSpec* new_func_typespec(TypeSpec** args, TypeSpec* ret,
  SourceLoc loc);

TypeSpec* new_array_typespec(TypeSpec* elem, /* Expr* size,*/ SourceLoc loc);

TypeSpec* new_ptr_typespec(TypeSpec* elem, Mutability mut, SourceLoc loc);

TypeSpec* new_ref_typespec(TypeSpec* elem, Mutability mut, SourceLoc loc);

TypeSpec* new_map_typespec(TypeSpec* key, TypeSpec* val, Mutability mut, SourceLoc loc);

TypeSpec* new_tuple_typespec(TypeSpec** types, Mutability mut, SourceLoc loc);

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
  EXPRKIND(CompoundLiteral) \
  EXPRKIND(StructLiteral) \
  EXPRKIND(Unary) \
  EXPRKIND(Binary) \
  EXPRKIND(FnCall) \
  EXPRKIND(Field) \
  EXPRKIND(DotFnCall) \
  EXPRKIND(If) \
  EXPRKIND(MatchIf) \
  EXPRKIND(While) \
  EXPRKIND(For) \
  EXPRKIND(Return) \
  EXPRKIND(Break) \
  EXPRKIND(Continue) \
  EXPRKIND(Block) \
  EXPRKIND(Binding) \
  EXPRKIND(In) \
  EXPRKIND(Tuple) \
  EXPRKIND(PatternExpr) \
  EXPRKIND(Index) \
  EXPRKIND(TupleElem) \
  EXPRKIND(Assignment) \
  EXPRKIND(Range) \
  EXPRKIND(Cast) \
  EXPRKIND(Slice)

typedef enum ExprKind {
  #define EXPRKIND(n) n,
    EXPRKINDS
  #undef EXPRKIND
} ExprKind;

typedef struct Clause {
  Pattern** patterns;
  u32 num_patterns;
  Expr* body;
  SourceLoc loc;
} Clause;

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
      Expr** elems;
      u32 num_elems;
    } comp_lit;
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
      Clause** body;
      u32 num_body;
    } matchif_expr;
    struct {
      Expr* cond;
      Expr* body;
    } while_expr;
    struct {
      Pattern* pat;
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
    struct {
      Expr* operand;
      Expr* index;
    } index;
    struct {
      Expr* operand;
      Token elem;
    } tupleelem;
    struct  {
      Expr* start;
      Expr* end;
      Expr* step;
    } range;
    struct  {
      Expr* operand;
      Expr* start;
      Expr* end;
    } slice;
    struct {
      Expr* expr;
      TypeSpec* spec;
    } cast;
  };
} Expr;

Expr* new_expr(ExprKind kind, SourceLoc loc);
Expr* new_name(Ident* name, SourceLoc loc);
Expr* new_literal(Token token, SourceLoc loc);
Expr* new_compoundliteral(Expr** elems, SourceLoc loc);
Expr* new_structliteral(TypeSpec* name, Expr** member, SourceLoc loc);
Expr* new_unary(Token op, Expr* expr, SourceLoc loc);
Expr* new_binary(Token op, Expr* lhs, Expr* rhs, SourceLoc loc);
Expr* new_fncall(Expr* name, Expr** actuals, SourceLoc loc);
Expr* new_field(Expr* operand, Ident* name, SourceLoc loc);
Expr* new_dotfncall(Expr* operand, Expr* name, Expr** actuals,
  SourceLoc loc);
Expr* new_if(Expr* cond, Expr* body, Expr* else_if, SourceLoc loc);
Expr* new_matchif(Expr* cond, Clause** body, SourceLoc loc);
Expr* new_while(Expr* cond, Expr* body, SourceLoc loc);
Expr* new_for(Pattern* pat, Expr* cond, Expr* body, SourceLoc loc);
Expr* new_return(Expr** expr, SourceLoc loc);
Expr* new_break(Token token, SourceLoc loc);
Expr* new_continue(Token token, SourceLoc loc);
Expr* new_block(Stmt** stmts, SourceLoc loc);
Expr* new_binding(Expr* name, Expr* expr, SourceLoc loc);
Expr* new_in(Expr* in, Expr* expr, SourceLoc loc);
Expr* new_tuple(Expr** elem, SourceLoc loc);
Expr* new_assign(Token op, Expr* variable, Expr* value, SourceLoc loc);
Expr* new_index(Expr* operand, Expr* index, SourceLoc loc);
Expr* new_tupelelem(Expr* operand, Token elem, SourceLoc loc);
Expr* new_range(Expr* start, Expr* end, Expr* step, SourceLoc loc);
Expr* new_slice(Expr* operand, Expr* start, Expr* end, SourceLoc loc);
Expr* new_cast(Expr* expr, TypeSpec* spec, SourceLoc loc);

Clause* new_clause(Pattern** pattern, Expr* body, SourceLoc loc);

#define ITEMKINDS \
  ITEMKIND(ItemLocal) \
  ITEMKIND(ItemAlias) \
  ITEMKIND(ItemFunction) \
  ITEMKIND(ItemStruct) \
  ITEMKIND(ItemTupleStruct) \
  ITEMKIND(ItemEnum) \
  ITEMKIND(ItemUse) \
  ITEMKIND(ItemModule) \
  ITEMKIND(ItemField) \
  ITEMKIND(ItemName)

// ItemName is only used in Enums

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
      Mutability mut;
      // Pattern** names;
      // u32 num_names;
      Pattern* name;
      TypeSpec* type;
      Expr* init;
    } local;
    struct {
      Ident* name;
      TypeSpec* type;
    } alias;
    struct {
      Ident* name;
      Item** arguments;
      u32 num_args;
      TypeSpec* ret;
      Expr* body;
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
      Ident* name;
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
    struct {
      Ident* name;
      Expr* value;
    } name;
  };
} Item;

Item* new_item(ItemKind kind, SourceLoc loc);
Item* new_itemlocal(Pattern* name, TypeSpec* type, Expr* init, Mutability mut, SourceLoc loc);
Item* new_itemalias(Ident* name, TypeSpec* type, SourceLoc loc);
Item* new_itemfunction(Ident* name, Item** arguments, TypeSpec* ret, Expr* body, SourceLoc loc);
Item* new_itemstruct(Ident* name, Item** fields, SourceLoc loc);
Item* new_itemtuplestruct(Ident* name, TypeSpec** fields, SourceLoc loc);
Item* new_itemenum(Ident* name, Item** elems, SourceLoc loc);
Item* new_itemuse(Ident* name, Expr** memebers, SourceLoc loc);
Item* new_itemmodule(Ident* name, Item** memebers, SourceLoc loc);
Item* new_itemfield(TypeSpec* type, Ident* name, Expr* init, SourceLoc loc);
Item* new_itemname(Ident* name, Expr* value, SourceLoc loc);

#define PATTERNKINDS \
  PATTERNKIND(WildCard)\
  PATTERNKIND(StructPattern) \
  PATTERNKIND(TuplePattern) \
  PATTERNKIND(RefPattern) \
  PATTERNKIND(PointerPattern) \
  PATTERNKIND(IdentPattern) \
  PATTERNKIND(LiteralPattern) \
  PATTERNKIND(RangePattern)


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
    Token literal;
    Ident* ident;
    struct {
      TypeSpec* path;
      Pattern** elems;
      u32 num_elems;
    } structure;
    struct {
      Pattern** elems;
      u32 num_elems;
    } tuple;
    struct {
      Mutability mut;
      Pattern* pat;
    } ref;
    struct {
      Mutability mut;
      Pattern* pat;
    } ptr;
    struct {
      Pattern* start;
      Pattern* end;
    } range;
  };
} Pattern;

Pattern* new_pattern(PatternKind kind, SourceLoc loc);

Pattern* new_wildcard(Token token, SourceLoc loc);

Pattern* new_ident_pat(Ident* ident, SourceLoc loc);

Pattern* new_struct_pat(TypeSpec* spec, Pattern** elems, SourceLoc loc);

Pattern* new_tuple_pat(Pattern** elems, SourceLoc loc);

Pattern* new_ref_pat(Mutability mut, Pattern* pat, SourceLoc loc);

Pattern* new_ptr_pat(Mutability mut, Pattern* pat, SourceLoc loc);

Pattern* new_literal_pat(Token token, SourceLoc loc);

Pattern* new_range_pat(Pattern* start, Pattern* end, SourceLoc loc);

const char* item_string(ItemKind kind);
const char* expr_string(ExprKind kind);
const char* stmt_string(StmtKind kind);
const char* pattern_string(PatternKind kind);
const char* typespec_string(TypeSpecKind kind);


typedef Item** ItemSet;
typedef struct Scope Scope;

typedef struct AstFile {
  ItemSet items;
  File* file;
  Scope* scope;
  u32 uid;
} AstFile;

AstFile* new_ast_file(File* file);

void add_item(AstFile* file, Item* item);

u32 ast_num_items(AstFile* file);

void destroy_expr(Expr* expr);
void destroy_item(Item* expr);
void destroy_stmt(Stmt* expr);
void destroy_typesepc(TypeSpec* expr);

#endif
