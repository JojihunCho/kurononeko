/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

/* counter for variable memory locations */
static int location = 0;
static ScopeList globalScope = NULL;
static char *funcName;
static int inScopeBefore = FALSE;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void typeError(TreeNode *t, char *message)
{
  fprintf(listing, "Error: Type error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static void symbolError(TreeNode *t, char *message)
{
  fprintf(listing, "Error: Symbol error at line %d: %s\n", t->lineno, message);
  Error = TRUE;
}

static void undeclaredError(TreeNode *t)
{
  if (t->kind.exp == CallK)
    fprintf(listing, "Error: Undeclared Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  else if (t->kind.exp == VarAccK)
    fprintf(listing, "Error: Undeclared Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  Error = TRUE;
}

static void redefinedError(TreeNode *t)
{
  if (t->kind.exp == FunDK)
    fprintf(listing, "Error: Redefined Function \"%s\" at line %d\n", t->attr.name, t->lineno);
  else if (t->kind.exp == VarDK)
    fprintf(listing, "Error: Redefined Variable \"%s\" at line %d\n", t->attr.name, t->lineno);
  Error = TRUE;
}

static void funcDeclNotGlobal(TreeNode *t)
{
  fprintf(listing, "Error: Function Definition is not allowed at line %d (name : %s)\n", t->lineno, t->attr.name);
  Error = TRUE;
}

static void voidVarError(TreeNode *t, char *name)
{
  fprintf(listing, "Error: Variable Type cannot be Void at line %d (name : %s)\n", t->lineno, name);
  Error = TRUE;
}

static void arrayindexError(TreeNode *t, char *message)
{
  fprintf(listing, "Error: Invalid array indexing at lind %d (name: \"%s\"). indices should be integer\n", t->lineno, t->attr.name);
  Error = TRUE;
}

static void insertIOFuncNode(void)
{
  TreeNode *func;
  TreeNode *typeSpec;
  TreeNode *param;
  TreeNode *compStmt;

  func = newExpNode(FunDK);
  func->type = Integer;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  func->lineno = 0;
  func->attr.name = "input";
  func->child[0] = NULL;
  func->child[1] = compStmt;

  st_insert("input", 0, addLocation(), func);

  func = newExpNode(FunDK);
  func->type = Void;

  param  = newExpNode(ParamK);
  param->attr.name = "";
  param->type = Integer;

  func->lineno = 0;
  func->attr.name = "output";
  func->child[0] = param;
  func->child[1] = compStmt;

  st_insert("output", 0, addLocation(), func);
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if(inScopeBefore) inScopeBefore = FALSE;
          else {
            ScopeList scope = sc_create(funcName);
            sc_push(scope);
            location++;
          }
          t->attr.scope = sc_top();
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case VarAccK:
        case CallK:
        {
          if (st_lookup(t->attr.name) == -1) undeclaredError(t);
          else st_add_lineno(t->attr.name,t->lineno);
          break;
        }
          
        case FunDK:
        {
          funcName = t->attr.name;
          if (st_lookup_excluding_parent(t->attr.name) >= 0) {
            redefinedError(t);
            break;
          }
          if (sc_top() != globalScope){
            funcDeclNotGlobal(t);
            break;
          }
          st_insert(funcName, t->lineno, addLocation(), t);
          sc_push(sc_create(funcName));
          inScopeBefore = TRUE;
          break;
        }
          
        case VarDK:
        {
          char * name = t->attr.name;
          if (st_lookup_excluding_parent(name) < 0) st_insert(name, t->lineno, addLocation(), t);
          else redefinedError(t);
          break;
        }
          
        case ParamK:
        {
          if(t->type == Void) break;
          if (st_lookup(t->attr.name) == -1) st_insert(t->attr.name, t->lineno, addLocation(), t);
          break;
        }
          
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void afterInsertNode(TreeNode *t)
{
  if (t->nodekind == StmtK && t->kind.stmt == CompK)
    sc_pop();
}
/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{
  globalScope = sc_create("global");
  sc_push(globalScope);
  insertIOFuncNode();
  traverse(syntaxTree, insertNode, afterInsertNode);
  if (TraceAnalyze)
  { printSymTab(listing);
  }
  sc_pop();
}

static void beforeCheckNode(TreeNode *t)
{
  switch (t->nodekind)
  {
  case ExpK:
    switch (t->kind.exp)
    {
    case FunDK:
      funcName = t->attr.name;
      break;
    default:
      break;
    }
    break;
  case StmtK:
    switch (t->kind.stmt)
    {
    case CompK:
      sc_push(t->attr.scope);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ 
  switch (t->nodekind)
  {
    case StmtK:
      switch (t->kind.stmt)
      {
        case CompK:
        {
          sc_pop();
          break;
        }
          
        case IfK:
        {
          if (t->child[0] == NULL) typeError(t, "expected expression");
          else if (t->child[0]->type == Void) typeError(t->child[0], "invalid if condition type");
          break;
        }
          
        case WhileK:
        {
          if (t->child[0] == NULL) typeError(t, "expected expression");
          else if (t->child[0]->type == Void) typeError(t->child[0], "invalid loop condition type");
          break;
        }
          
        case AssignK:
        {
          if (t->child[0]->type == Void || t->child[1]->type == Void)
            typeError(t->child[0], "invalid variable type");
          else if (t->child[0]->type == IntegerArr && t->child[0]->child[0] == NULL)
            typeError(t->child[0], "invalid variable type");
          else if (t->child[1]->type == IntegerArr && t->child[1]->child[0] == NULL)
            typeError(t->child[0], "invalid variable type");
          else
            t->type = t->child[0]->type;
          break;
        }
          
        case ReturnK:
        {
          TreeNode *retFunc = get_bucket(funcName)->treeNode;
          if ((retFunc->type == Void && t->child[0] != NULL) || (retFunc->type != Void && (t->child[0] == NULL) || (retFunc->type != t->child[0]->type)))
          typeError(t, "invalid return type");
          break;
        }
          
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      {
        case OpK:
        {
          ExpType lType, rType;
          TokenType op;

          lType = t->child[0]->type;
          rType = t->child[1]->type;
          op = t->attr.op;

          if (lType == IntegerArr && t->child[0]->child[0] != NULL) lType = Integer;
          if (rType == IntegerArr && t->child[1]->child[0] != NULL) rType = Integer;

          if ((lType == Void || rType == Void) || (lType != rType)) typeError(t, "invalid expression");
          else t->type = Integer;
          break;
        }
          
        case ConstK:
        {
          t->type = Integer;
          break;
        }

        case VarDK:
        {   
            if (t->type == Void) {
            char *name;
            name = t->attr.name;
            voidVarError(t, name);
          }
          break;
        }
          
        case CallK:
        {
          BucketList l = get_bucket(t->attr.name);
          TreeNode *funcNode = NULL;
          TreeNode *arg;
          TreeNode *param;

          if (l == NULL) break;
          funcNode = l->treeNode;
          arg = t->child[0];
          param = funcNode->child[0];

          if (funcNode->kind.exp != FunDK){
            typeError(t, "invalid expression");
            break;
          }
        
          while (arg != NULL) {
            if (param == NULL || arg->type == Void) {
              typeError(arg, "invalid function call");
              break;
            }
            ExpType pType = param->type;
            ExpType aType = arg->type;
            if (aType == IntegerArr && arg->child[0] != NULL) aType = Integer;

            if (pType != aType) {
              typeError(arg, "invalid function call");
              break;
            }
            else{
              arg = arg->sibling;
              param = param->sibling;
            }
          }
          if (arg == NULL && param != NULL && param->child[0]->type != VOID) typeError(t->child[0], "invalid function call");
          
          t->type = funcNode->type;
          break;
        }
          
        case VarAccK:
        {
          BucketList l = get_bucket(t->attr.name);
          if (l == NULL) break;

          TreeNode *symbolNode = NULL;
          symbolNode = l->treeNode;

          if((t->child[0] != NULL) && (symbolNode->type != IntegerArr)) typeError(t, "invalid expression");
          if((t->child[0] != NULL) && (t->child[0]->type != Integer)) arrayindexError(t, "invalid expression");
          else t->type = symbolNode->type;

          break;
        }
          
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ sc_push(globalScope);
  traverse(syntaxTree, beforeCheckNode, checkNode);
  sc_pop();
}
