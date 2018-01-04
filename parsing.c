/* INCLUDE */
  #include <stdio.h>
  #include "mpc.h"

/* Declare a buffer for user input of size 2048 */
  static char input[2048];

/* Définition de la structure lval (Lisp VALue) */
typedef struct {
  enum type { LVAL_NUM, LVAL_ERR } type;
  long num;
  enum err { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM } err;
}lval;

/* Déclare fonctions "lval" */
  lval lval_num (long x);
  lval lval_err (int x);
  void lval_print (lval v);

/* Declare fonction "eval_op" et "eval" */
  long eval (mpc_ast_t* t);
  long eval_op(long x, char* op, long y);

int main(int argc, char** argv) {
  /* Create Some Parsers */
    mpc_parser_t* Number   = mpc_new("number");
    mpc_parser_t* Operator = mpc_new("operator");
    mpc_parser_t* Expr     = mpc_new("expr");
    mpc_parser_t* Lispy    = mpc_new("lispy");

  /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
    "                                                               \
      number   : /-?[0-9]+(\\.[0-9]+)?/ ;                           \
      operator : '+' | '-' | '*' | '/' | '%' | '^' |                \
                \"add\" | \"sub\" | \"mul\" | \"div\" |            \
                \"mod\" | \"exp\" | \"min\" | \"max\" ;            \
      expr     : <number> | '(' <operator> <expr>+ ')' ;            \
      lispy    : /^/ <operator> <expr>+ /$/ ;                       \
    ",
    Number, Operator, Expr, Lispy);

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
        long result = eval(r.output);
        printf("%li\n", result);
        mpc_ast_delete(r.output);
    }
  }
  /* Undefine and Delete our Parsers */
    mpc_cleanup(4, Number, Operator, Expr, Lispy);
    return 0;

}


/* Définition eval et eval_op*/
  long eval(mpc_ast_t* t) {
    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number")) {
    return atoi(t->contents);
    }

    /* The operator is always second child. */
    char* op = t->children[1]->contents;

    /* We store the third child in `x` */
    long x = eval(t->children[2]);

    /* "-" signifie "négfatif" si un seul opérande 
    if ((strcmp(op, "-") == 0) && strstr(t->children[3]->contents, "")){
      return -x;
    } */

    /* Iterate the remaining children and combining. */
    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
      x = eval_op(x, op, eval(t->children[i]));
      i++;
    }

    return x;
  }

  long eval_op(long x, char* op, long y) {
    if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0) { return x + y; }
    if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0) { return x - y; }
    if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0) { return x * y; }
    if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0) { return x / y; }
    if (strcmp(op, "%") == 0 || strcmp(op, "mod") == 0) { return x % y; }
    if (strcmp(op, "^") == 0 || strcmp(op, "exp") == 0) {
      int total = 1;
      for (int i = 0; i < y; i++){
        total*= x;
      }
      return total ;}
    if (strcmp(op, "min") == 0 ) {return (x < y) ? x : y;}
    if (strcmp(op, "max") == 0 ) {return (x > y) ? x : y;}
    return 0;
  }

/* Déclare fonctions "lval" */
  lval lval_num (long x){
    lval v;
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
        switch (v.err)
          case LERR_DIV_ZERO: printf("Error: Division By Zero!"); break;
          case LERR_BAD_OP: printf("Error: Invalid Operator!"); break;
          case LERR_BAD_NUM: printf("Error: Invalid Number!");
    }
  }
  /* Print an "lval" followed by a newline */
  void lval_println(lval v){
    lval_print(v);
    putchar('\n'); 
  }
