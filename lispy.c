/* INCLUDE */
  #include "mpc.h"

/* Declare a buffer for user input of size 2048 */
  static char input[2048];
/* Définition de la structure lval (Lisp VALue) */
  enum Type { LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR };
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

/* Declare fonction "eval_op" et "eval" */
/*   lval eval(mpc_ast_t* t);
  lval eval_op(lval x, char* op, lval y); */

int main(int argc, char** argv) {
/* Create Some Parsers */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");

/* Define them with the following Language */
mpca_lang(MPCA_LANG_DEFAULT,
  "                                          \
    number : /-?[0-9]+/ ;                    \
    symbol : '+' | '-' | '*' | '/' ;         \
    sexpr  : '(' <expr>* ')' ;               \
    expr   : <number> | <symbol> | <sexpr> ; \
    lispy  : /^/ <expr>* /$/ ;               \
  ",
  Number, Symbol, Sexpr, Expr, Lispy);

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
      lval* x = lval_read(r.output);
      lval_println(x);
      lval_del(x);
  }
}
/* Undefine and Delete our Parsers */
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
  return 0;
}


/* Définition eval et eval_op*/
// lval eval(mpc_ast_t* t) {

//   if (strstr(t->tag, "number")) {
//     /* Check if there is some error in conversion */
//     errno = 0;
//     long x = strtol(t->contents, NULL, 10);
//     return (errno != ERANGE) ?
//      lval_num(x) : lval_err("LERR_BAD_NUM");
//   }

//   char* op = t->children[1]->contents;
//   lval x = eval(t->children[2]);

//   int i = 3;
//   while (strstr(t->children[i]->tag, "expr")) {
//     x = eval_op(x, op, eval(t->children[i]));
//     i++;
//   }

//   return x;
//   }

// lval eval_op(lval x, char* op, lval y) {
//   /* If either value is an error return it */
//   if (x.type == LVAL_ERR) { return x; }
//   if (y.type == LVAL_ERR) { return y; }

//   /* Otherwise do maths on the number values */
//   if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
//   if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
//   if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
//   if (strcmp(op, "/") == 0) {
//     /* If second operand is zero return error */
//     return y.num == 0
//       ? lval_err("LERR_DIV_ZERO")
//       : lval_num(x.num / y.num);
//   }

//   return lval_err("LERR_BAD_OP");
//   }

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
    case LVAL_SEXPR: for (int i = 0; i < v->count ; i++){
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

  for (int i = 0; i < t->children_num; i++){
  if ((strcmp(t->children[i]->contents, ")") == 0) ||
  ((strcmp(t->children[i]->contents, "(") == 0)) ||
  ((strcmp(t->children[i]->tag, "regex") == 0))
  ){continue;}
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