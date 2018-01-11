/* INCLUDE */
  #include "mpc.h"

/* Declare a buffer for user input of size 2048 */
  static char input[2048];
/* Définition de la structure lval (Lisp VALue) */
  enum Type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };
  typedef struct lval {
    int type;
    long num;
    char* err;
    char* sym;
    int count;
    struct lval** cell;
  }lval;

/* Déclare fonctions "lval" */
  lval* lval_num (long x);
  lval* lval_err (char* m);
  lval* lval_sym (char* s);
  lval* lval_sexpr (void);

  lval* lval_read (mpc_ast_t* t);
  lval* lval_read_num (mpc_ast_t* t);
  lval* lval_add (lval* v, lval* x);

  void lval_print (lval* v);
  void lval_expr_print(lval* v, char open, char close);
  void lval_println(lval* v);
  void lval_del(lval* v);

  lval* lval_eval_sexpr (lval* v);
  lval* lval_eval (lval* v);
  lval* lval_pop (lval* v, int i);
  lval* lval_take (lval* v, int i);
  lval* builtin_op (lval* a, char* op);

  lval* lval_qexpr (void);


int main(int argc, char** argv) {
/* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Qexpr = mpc_new("qexpr");
  mpc_parser_t* Expr  = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

/* Define them with the following Language */
mpca_lang(MPCA_LANG_DEFAULT,
  "                                          \
    number : /-?[0-9]+/ ;                    \
    symbol : \"list\" | \"head\" | \"tail\"  \
           | \"join\" | \"eval\"             \
           |'+' | '-' | '*' | '/' ;          \
    sexpr  : '(' <expr>* ')' ;               \
    qexpr  : '{' <expr>* '}' ;               \
    expr   : <number> | <symbol> |           \
             <sexpr>  | <qexpr>   ;          \
    lispy  : /^/ <expr>* /$/ ;               \
  ",
  Number, Symbol, Sexpr, Qexpr, Expr, Lispy);

/* Print Version and Exit Information */
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+c to Exit\n");

/* In a never ending loop */
  while (1) {
  /* Output our prompt */
    fputs("lispy> ", stdout);
  /* Read a line of user input of maximum size 2048 */
    fgets(input, 2048, stdin);

  /* Attempt to Parse the user Input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)){
      /* read the input */
      lval* x = lval_eval(lval_read(r.output));
      lval_println(x);
      lval_del(x);
  }
}
/* Undefine and Delete our Parsers */
  mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lispy);
  return 0;
}


/* Déclare fonctions "lval" */
lval* lval_num (long x){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_NUM;
    v->num = x;
    return v;
  }

lval* lval_err (char* m){
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;
    v->err = malloc(strlen(m) +1);
    strcpy(v->err, m);
    return v;
  }
lval* lval_sym (char* s){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen (s) + 1);
  strcpy(v->sym, s);
  return v;
}
lval* lval_sexpr (void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}
lval* lval_qexpr (void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_QEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

  /* Print an "lval" */
void lval_expr_print(lval* v, char open, char close){
    putchar(open);
    for (int i = 0; i < v->count; i++){
      lval_print(v->cell[i]);
      if (i != v->count - 1){
        putchar(' ');
      }
    }
    putchar(close);
  }

void lval_print (lval* v){
    switch (v->type){
      case LVAL_NUM: printf ("%li", v->num); break;
      case LVAL_ERR: printf ("Error: %s", v->err);break;
      case LVAL_SYM: printf ("%s", v->sym );break;
      case LVAL_SEXPR: lval_expr_print(v, '(', ')');break;
      case LVAL_QEXPR: lval_expr_print(v, '{', '}');break;
    }
  }
  /* Print an "lval" followed by a newline */
void lval_println(lval* v){
    lval_print (v);
    putchar('\n');
  }
void lval_del(lval* v){
  switch (v->type){
    case LVAL_NUM: break;
    case LVAL_ERR: free (v->err);break;
    case LVAL_SYM: free (v->sym);break;
    case LVAL_SEXPR: 
    case LVAL_QEXPR:
    for (int i = 0; i < v->count ; i++){
      lval_del(v->cell[i]);
    }
    free (v->cell);
    break;
  }
  free(v);
}

/* READING EXPRESSIONS */
lval* lval_read (mpc_ast_t* t){
  if (strstr(t->tag, "number")){return lval_read_num(t);}
  if (strstr(t->tag, "symbol")){return lval_sym(t->contents);}
  lval* x = NULL;
  if ((strcmp(t->tag, ">") == 0) || (strstr(t->tag, "sexpr"))){x = lval_sexpr();}
  if (strstr(t->tag, "qexpr")){x = lval_qexpr();}

  for (int i = 0; i < t->children_num; i++){
  if ((strcmp(t->children[i]->contents, "(") == 0) ||
      (strcmp(t->children[i]->contents, ")") == 0) ||
      (strcmp(t->children[i]->contents, "{") == 0) ||
      (strcmp(t->children[i]->contents, "}") == 0) ||
      (strcmp(t->children[i]->tag, "regex" ) == 0))
  {continue;}
  x = lval_add (x, lval_read(t->children[i]));
}
  return x;
}
lval* lval_read_num (mpc_ast_t* t){
  errno = 0;
  long x = strtol (t->contents, NULL, 10);
  return (errno != ERANGE) ?
  lval_num(x) : lval_err("Invalid number");
}
lval* lval_add (lval* v, lval* x){
v->count ++;
v->cell= realloc(v->cell, sizeof (lval*) * v->count);
v->cell[v->count - 1] = x;
return v;
}

/* EVALUATE EXPRESSIONS */
lval* lval_eval_sexpr (lval* v){
  /* Evaluate Children */
  for (int i = 0; i < v->count; i++){
    v->cell[i] = lval_eval(v->cell[i]);
  }
  /* Error checking */
  for (int i = 0; i < v->count; i++){
    if (v->cell[i]->type == LVAL_ERR){return lval_take(v, i);}
  }

  /* Empty Expression */
  if (v->count == 0){return v;}

  /* Single expression */
  if (v->count == 1){return lval_take(v, 0);}

  /* We know we have a valid expression (number of child > 1) */
  /* Ensure first element is symbol */
  lval* f = lval_pop(v, 0);
  if (f->type != LVAL_SYM){
    lval_del(f);
    lval_del(v);
    return lval_err("S-expression Does not start with symbol!");
  }

  /* Call builtin calculator with operator */
  lval* result = builtin_op(v, f->sym);
  lval_del(f);
  return result;
}
lval* lval_eval (lval* v){
  /* Evaluate Sexpressions */
  if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
  /* All other lval types remain the same */
  return v;
}
lval* lval_pop (lval* v, int i){
  lval* x = v->cell[i];
  memmove(&v->cell[i], &v->cell[i + 1], sizeof (lval*) * (v->count - i - 1));
  v->count--;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  return x;
}

lval* lval_take (lval* v, int i){
  lval* x = lval_pop(v, i);
  lval_del (v);
  return x;
}

lval* builtin_op (lval* a, char* op){
  /* Ensure all arguments are numbers */
  for (int i = 0; i< a->count; i++){
    if (a->cell[i]->type != LVAL_NUM){
      lval_del(a);
      return lval_err("Cannot operate on non-numbers!");
    }
  }
  /* Pop the first element (after the operator) */
  lval* x = lval_pop(a, 0);
  /* if no more arg or sub-exp
  and op is negative
  -> unary negation */
  if ((strcmp(op, "-") == 0) && (a->count == 0)){
    x->num = -x->num;
  }
  /* while there are still elements remaining */
  while (a->count > 0){
    /* pop the next element */
    lval* y = lval_pop(a, 0);
    /* perform operation according to the operator */
    if (strcmp(op, "+") == 0){x->num += y->num; }
    if (strcmp(op, "-") == 0){x->num -= y->num; }
    if (strcmp(op, "*") == 0){x->num *= y->num; }
    if (strcmp(op, "/") == 0){
      /* if dividande == 0 -> delete args + error*/
      if (y->num == 0){
        lval_del(x);
        lval_del(y);
        x = lval_err("Division By Zero!");
        break;
      }
    x->num /= y->num;
    }
  }
  lval_del(a);
  return x;
}

lval* builtin_head (lval* a){
  /* Check Error Conditions*/
  if (a->count != 1){
    lval_del(a);
    return lval_err("Function 'head' passed too many arguments!");
  }
  if (a->cell[0]->type != LVAL_QEXPR){
    lval_del(a);
    return lval_err("Function 'head' passed incorrect types!");
  }
  if (a->cell[0]->count == 0){
    lval_del(a);
    return lval_err("Function 'head' passed '{}'!");
  }
  /* Otherwise take first argument */
  lval* v = lval_take(a, 0);

  /* Delete all elements that are not head and return  */
  while (v->count > 1){
    lval_del(lval_pop(v, 1));
  }
  return v;
}

lval* builtin_tail (lval* a){
  /* Check Error Conditions*/
  if (a->count != 1){
    lval_del(a);
    return lval_err("Function 'tail' passed too many arguments!");
  }
  if (a->cell[0]->type != LVAL_QEXPR){
    lval_del(a);
    return lval_err("Function 'tail' passed incorrect types!");
  }
  if (a->cell[0]->count == 0){
    lval_del(a);
    return lval_err("Function 'tail' passed '{}' !");
  }
    /* Otherwise take first argument */
  lval* v = lval_take(a, 0);

  /* Delete all elements that are not head and return  */
  lval_del(lval_pop(v, 0));
  return v;
}
