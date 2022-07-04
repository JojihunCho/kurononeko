/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4
#define MAX_SCOPES 1000
/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

ScopeList scopes[MAX_SCOPES], scopeStack[MAX_SCOPES];
int cntScope = 0, cntScopeStack = 0, location[MAX_SCOPES];

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode *treeNode )
{ int h = hash(name);
  ScopeList nowScope = sc_top();
  BucketList l = nowScope->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = nowScope->hashTable[h];
    nowScope->hashTable[h] = l; }
  else /* found in table, so just add line number */
  { /*LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  */}
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char *name )
{ BucketList l = get_bucket(name);
  if (l != NULL)
    return l->memloc;
  return -1;
}

void st_add_lineno ( char *name, int lineno )
{ BucketList bl = get_bucket(name);
  LineList ll = bl->lines;
  while (ll->next != NULL)
    ll = ll->next;
  ll->next = (LineList)malloc(sizeof(struct LineListRec));
  ll = ll->next;
  ll->lineno = lineno;
  ll->next = NULL;
}

int st_lookup_excluding_parent ( char * name )
{ int h = hash(name);
  ScopeList nowScope = sc_top();
  BucketList l =  nowScope->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return -1;
  else return l->memloc;
}

BucketList get_bucket ( char *name )
{ int h = hash(name);
  ScopeList nowScope = sc_top();
  while (nowScope != NULL)
  {
    BucketList l = nowScope->hashTable[h];
    while ((l != NULL) && (strcmp(name, l->name) != 0))
      l = l->next;
    if (l != NULL)
      return l;
    nowScope = nowScope->parent;
  }
  return NULL;
}


/* Stack for static scope */
ScopeList sc_create ( char *funcName )
{ ScopeList newScope;
  newScope = (ScopeList)malloc(sizeof(struct ScopeListRec));
  newScope->funcName = funcName;
  newScope->nestedLevel = cntScopeStack;
  newScope->parent = sc_top();
  scopes[cntScope++] = newScope;

  return newScope;
}

ScopeList sc_top ( void )
{ if (cntScopeStack <= 0)
    return NULL;
  return scopeStack[cntScopeStack - 1];
}

void sc_pop ( void )
{ if (cntScopeStack > 0)
    cntScopeStack--;
}

void sc_push ( ScopeList scope )
{ scopeStack[cntScopeStack] = scope;
  location[cntScopeStack] = 0;
  cntScopeStack++;
}

int addLocation ( void )
{ return location[cntScopeStack - 1]++;
}


/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab ( FILE *listing )
{
  print_SymTab(listing);
  fprintf(listing, "\n");
  print_FuncTab(listing);
  fprintf(listing, "\n");
  print_Func_globVar(listing);
  fprintf(listing, "\n");
  print_SC(listing);
} /* printSymTab */

void print_SymTab(FILE * listing)
{ int i, j;
  fprintf(listing, "\n< Symbol Table >\n");
  fprintf(listing, "Variable Name  Variable Type  Scope Name  Location  Line Numbers\n");
  fprintf(listing, "-------------  -------------  ----------  --------  ------------\n");
  
  for (i = 0; i < cntScope; i++){
    ScopeList nowScope = scopes[i];
    BucketList *hashTable = nowScope->hashTable;

    for (j=0;j<SIZE;++j){
      if(hashTable[j] != NULL){
        BucketList l = hashTable[j];
        TreeNode *node = l->treeNode;
        while (l != NULL){
          LineList t = l->lines;
          fprintf(listing,"%-15s",l->name);
          
          switch (node->nodekind)
          {
          case ExpK:
            switch (node->kind.exp)
            {
              case FunDK:
                fprintf(listing, "%-15s", "Function");
                break;
              case VarDK:
                switch (node->type)
                {
                case Void:
                  fprintf(listing, "%-15s", "Void");
                  break;
                case Integer:
                  fprintf(listing, "%-15s", "Integer");
                  break;
                case IntegerArr:
                  fprintf(listing, "%-15s", "IntegerArr");
                  break;
                default:
                  break;
                }
                break;
              case ParamK:
                switch (node->type)
                {
                case Integer:
                  fprintf(listing, "%-15s", "Integer");
                  break;
                case IntegerArr:
                  fprintf(listing, "%-15s", "IntegerArr");
                  break;
                default:
                  break;
                }
                break;
              default:
                break;
            }
            break;
          default:
            break;
          }

          fprintf(listing, "%-12s", nowScope->funcName);
          fprintf(listing, "%-10d", l->memloc);

          while (t != NULL){
            fprintf(listing, "%4d", t->lineno);
            t = t->next;
          }

          fprintf(listing, "\n");
          l = l->next;
        }
      }
    }
  }
} /* print-SymTab */

void print_FuncTab(FILE *listing)
{
  int i, j, k, p;
  fprintf(listing, "\n< Function Table >\n");
  fprintf(listing, "Function Name  Scope Name  Return Type  Parameter Name  Parameter Type\n");
  fprintf(listing, "-------------  ----------  -----------  --------------  --------------\n");

  for (i = 0; i < cntScope; i++){
    ScopeList nowScope = scopes[i];
    BucketList *hashTable = nowScope->hashTable;

    for (j = 0; j < SIZE; j++){
      if (hashTable[j] != NULL){
        BucketList l = hashTable[j];
        TreeNode *node = l->treeNode;

        while (l != NULL){
          switch (node->nodekind)
          {
            case ExpK:
              if (node->kind.exp == FunDK){
                fprintf(listing, "%-15s", l->name);
                fprintf(listing, "%-12s", nowScope->funcName);
                switch (node->type)
                {
                case Void:
                  fprintf(listing, "%-13s", "Void");
                  break;
                case Integer:
                  fprintf(listing, "%-13s", "Integer");
                  break;
                case IntegerArr:
                  fprintf(listing, "%-13s", "IntegerArr");
                  break;
                default:
                  break;
                }

                int noParam = 1;
                for (k = 0; k < cntScope; k++){
                  ScopeList paramScope = scopes[k];
                  if (strcmp(paramScope->funcName, l->name) != 0) continue;
                  BucketList *paramhashTable = paramScope->hashTable;

                  for (p = 0; p < SIZE; p++){
                    if (paramhashTable[p] != NULL){
                      BucketList pbl = paramhashTable[p];
                      TreeNode *pnode = pbl->treeNode;

                      while (pbl != NULL){
                        if (pnode->nodekind == ExpK){
                          if (pnode->kind.exp == ParamK){
                            noParam = 0;
                            fprintf(listing, "\n");
                            fprintf(listing, "%-40s", "");
                            fprintf(listing, "%-16s", pbl->name);

                            switch (node->type)
                            {
                            case Integer:
                              fprintf(listing, "%-15s", "Integer");
                              break;
                            case IntegerArr:
                              fprintf(listing, "%-15s", "IntegerArr");
                              break;
                            default:
                              break;
                            }
                          }
                        }
                        pbl = pbl->next;
                      }
                    }
                  }
                  break;
                }
                if (noParam)
                {
                  fprintf(listing, "%-16s", "");
                  if (strcmp(l->name, "output") != 0)
                    fprintf(listing, "%-14s", "Void");
                  else
                    fprintf(listing, "\n%-56s%-14s", "", "Integer");
                }

                fprintf(listing, "\n");
              }
              break;
            default:
              break;
          }
          l = l->next;
        }
      }
    }
  }
}


void print_Func_globVar(FILE *listing)
{
  int i, j;
  fprintf(listing, "\n< Function and Global Variables >\n");
  fprintf(listing, "   ID Name      ID Type    Data Type\n");
  fprintf(listing, "-------------  ---------  -----------\n");

  for (i = 0; i < cntScope; i++){
    ScopeList nowScope = scopes[i];
    if (strcmp(nowScope->funcName, "global") != 0) continue;

    BucketList *hashTable = nowScope->hashTable;

    for (j = 0; j < SIZE; j++){
      if (hashTable[j] != NULL){
        BucketList l = hashTable[j];
        TreeNode *node = l->treeNode;

        while (l != NULL){
          if (node->nodekind == ExpK){
            switch (node->kind.exp)
            {
            case FunDK:
              fprintf(listing, "%-15s", l->name);
              fprintf(listing, "%-11s", "Function");
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-11s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-11s", "Integer");
                break;
              case IntegerArr:
                fprintf(listing, "%-11s", "IntegerArr");
                break;
              default:
                break;
              }
              fprintf(listing, "\n");
              break;
            case VarDK:
              fprintf(listing, "%-11s", "Variable");
              switch (node->type)
              {
              case Void:
                fprintf(listing, "%-11s", "Void");
                break;
              case Integer:
                fprintf(listing, "%-11s", "Integer");
                break;
              case IntegerArr:
                fprintf(listing, "%-11s", "IntegerArr");
                break;
              default:
                break;
              }
              fprintf(listing, "\n");
              break;
            default:
              break;
            }
            l = l->next;
          }
        }
      }
    }
    break;
  }
}

void print_SC(FILE *listing)
{
  int i, j;
  fprintf(listing, "\n< Function Parameters and Local Variables >\n");
  fprintf(listing, "  Scope Name    Nested Level     ID Name      Data Type \n");
  fprintf(listing, "--------------  ------------  -------------  -----------\n");

  for (i = 0; i < cntScope; i++){
    ScopeList nowScope = scopes[i];
    if (strcmp(nowScope->funcName, "global") == 0)
      continue;
    BucketList *hashTable = nowScope->hashTable;

    int noParamVar = 1;

    for (j = 0; j < SIZE; j++){
      if (hashTable[j] != NULL){
        BucketList l = hashTable[j];
        TreeNode *node = l->treeNode;

        while (l != NULL){
          if(node->nodekind == ExpK){
            if(node->kind.exp == VarDK || node->kind.exp == ParamK){
              noParamVar = 0;
              fprintf(listing, "%-16s", nowScope->funcName);
              fprintf(listing, "%-14d", nowScope->nestedLevel);
              fprintf(listing, "%-15s", node->attr.name);
              switch (node->type)
              {
                case Integer:
                  fprintf(listing, "%-11s", "Integer");
                  break;
                case IntegerArr:
                  fprintf(listing, "%-11s", "IntegerArr");
                default:
                 break;
              }
              fprintf(listing, "\n");
            }
          }
          l = l->next;
        }
      }
    }
    if (!noParamVar) fprintf(listing, "\n");
  }
}