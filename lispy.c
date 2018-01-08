/* INCLUDE */
  #include <stdio.h>
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
    int count
    struct lval** cell
  }lval;

/* Déclare fonctions "lval" */
  lval lval_num (long x);
  lval lval_err (int x);
  void lval_print (lval v);
  void lval_println(lval v);

/* Declare fonction "eval_op" et "eval" */
  lval eval(mpc_ast_t* t);
  lval eval_op(lval x, char* op, lval y);

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
      /* print the RESULT */
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
  }
}
/* Undefine and Delete our Parsers */
  mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);
  return 0;
}


/* Définition eval et eval_op*/
lval eval(mpc_ast_t* t) {

  if (strstr(t->tag, "number")) {
    /* Check if there is some error in conversion */
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  char* op = t->children[1]->contents;
  lval x = eval(t->children[2]);

  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }

  return x;
  }

lval eval_op(lval x, char* op, lval y) {
  /* If either value is an error return it */
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }

  /* Otherwise do maths on the number values */
  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); }
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); }
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); }
  if (strcmp(op, "/") == 0) {
    /* If second operand is zero return error */
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }

  return lval_err(LERR_BAD_OP);
  }

/* Déclare fonctions "lval" */
lval* lval_num (long x){
    lval* v = malloc(sizeof(lval));
    v.type = LVAL_NUM;
    v.num = x;
    return v;
  }

lval lval_err (int x){
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
  }
  
  /* Print an "lval" */
void lval_print (lval v){
    switch (v.type){
      case LVAL_NUM:
      printf ("%li",v.num);
      break;
      case LVAL_ERR:
        switch (v.err){
          case LERR_DIV_ZERO: printf("Error: Division By Zero!"); break;
          case LERR_BAD_OP: printf("Error: Invalid Operator!"); break;
          case LERR_BAD_NUM: printf("Error: Invalid Number!");
      }
    }
  }
  /* Print an "lval" followed by a newline */
void lval_println(lval v){
    lval_print(v);
    putchar('\n'); 
  }
