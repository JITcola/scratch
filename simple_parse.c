/*****************************************************************************/
/*                          Simple Expression Parser                         */
/*                             Kevin Mazzarella                              */
/*                                                                           */
/* The following program asks the user to enter an arithmetic expression in  */
/* infix form, then produces and displays the fully-parenthesized, postfix,  */
/* and prefix forms of the expression. In addition to the four standard      */
/* arithmetic operators, expressions may contain exponents as well as        */ 
/* variable names (strings of letters).                                      */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define DEBUG
#define MAX_CHARS 1000

#define LPAREN 0
#define RPAREN 1
#define EXP    2
#define MUL    3
#define DIV    4
#define ADD    5
#define SUB    6
#define ATOM   7

#define NEXPR    0
#define NeXPR    1
#define NEXPRP   2
#define NeXPRP   3
#define NEXPRPP  4
#define NEXPRPPP 5
#define NLPAREN  6
#define NRPAREN  7
#define NEXP     8
#define NMUL     9
#define NDIV     10
#define NADD     11
#define NSUB     12
#define NATOM    13
#define NEPSILON 14

/* Debugging can be turned off by compiling with the line defining the      */
/* DEBUG macro removed. MAX_CHARS is the maximum number of characters the   */
/* user will be allowed to use in an expression input to the program.       */
/* Macros LPAREN through ATOM are defined so that token types can be        */
/* represented as numbers instead of strings. Similarly, macros NEXPR       */
/* through NEPSILON are defined so that parse tree nodes can have types     */
/* represented by numbers. The lexer uses struct tokens to represent the    */
/* tokens in the user input; it produces a linked list to pass to the       */
/* parser. The function create_token is used to create struct tokens,       */
/* dynamically allocating their memory and initializing them.               */

struct token {
   int type;
   char *string;
   struct token *prev;
   struct token *next;
};

struct token *create_token(int type, char *init_char, int token_len);
struct token *input_lexer(char *user_input);

/* The parser uses recursive descent to build a parse tree, whose nodes are */
/* struct pt_nodes. The function create_node works similarly to the         */
/* function create_token. The type member of struct pt_node is used to      */
/* store the value which the macros defined above associate with the        */
/* terminal or nonterminal in the grammar represented by the node. If the   */
/* node represents a terminal, the member string holds the lexeme of the    */
/* corresponding token. Member num_childs holds the number of children the  */
/* node has. When num_childs is nonzero the member child_ptrs is an array   */
/* of pointers to the node's children. Functions expr through epsilon are   */
/* used to parse the lexed input.                                           */

/* The function expr constructs a parse tree for expressions of the         */
/* language defined by the following grammar:                               */
/*                                                                          */
/* expr    ->  exprp Expr                                                   */
/* Expr    ->  + exprp Expr | - exprp Expr | epsilon                        */
/* exprp   ->  exprpp Exprp                                                 */
/* Exprp   ->  * exprpp Exprp | / exprpp Exprp | epsilon                    */
/* exprpp  ->  exprppp ^ exprpp | exprppp                                   */
/* exprppp ->  atom | ( expr )                                              */
/*                                                                          */
/* The expressions which can be derived from expr are arithmetic            */
/* expressions containing variable names, numbers, parentheses, and the     */
/* infix operators ^, *, /, +, and - . Expressions may not contain spaces,  */
/* and all characters in variable names must be letters. The grammar        */
/* has an odd-looking form because it was constructed by applying           */
/* transformations to a simpler grammar; the transformations eliminated     */
/* so-called "left-recursive" productions and ensured correct operator      */
/* precedence/associativity (all binary operators except for ^ associate    */
/* to the left, while ^ associates to the right).                           */

struct pt_node {
   int type;
   char *string;
   int num_childs;
   struct pt_node **child_ptrs;
};

struct pt_node *create_node(int type, char *string, int num_childs);
struct pt_node *expr(struct token **lexed_in_ptr);
struct pt_node *Expr(struct token **lexed_in_ptr);
struct pt_node *exprp(struct token **lexed_in_ptr);
struct pt_node *Exprp(struct token **lexed_in_ptr);
struct pt_node *exprpp(struct token **lexed_in_ptr);
struct pt_node *exprppp(struct token **lexed_in_ptr);
struct pt_node *lparen(struct token **lexed_in_ptr);
struct pt_node *rparen(struct token **lexed_in_ptr);
struct pt_node *expo(struct token **lexed_in_ptr);
struct pt_node *mul(struct token **lexed_in_ptr);
struct pt_node *quo(struct token **lexed_in_ptr);
struct pt_node *add(struct token **lexed_in_ptr);
struct pt_node *sub(struct token **lexed_in_ptr);
struct pt_node *atom(struct token **lexed_in_ptr);
struct pt_node *epsilon(void);

/* The function concat returns the concatenation of str1 and str2 as a new  */
/* string. The function pre_compl_par takes as argument a pointer to the    */
/* root node of a parse tree produced by expr and returns a string          */
/* containing the completely parenthesized form of the expression           */
/* to which the parse tree corresponds. Since pre_compl_par may return an   */
/* expression with a pair of parentheses around it, strip_parens can be     */
/* called on the string returned by pre_compl_par to produce an expression  */
/* guaranteed to be free of an unnecessary pair of surrounding parentheses. */
/* The function compl_par is the composition of strip_parens and            */
/* pre_compl_par, returning the fully-parenthesized form of the input       */
/* expression without surrounding parentheses.                              */

char *concat(char *str1, char *str2);
char *pre_compl_par(struct pt_node *head);
char *strip_parens(char *pre_compl_par);
char *compl_par(struct pt_node *head);

/* The function postfix takes the parse tree produced by expr for an         */
/* expression with infix binary operators and returns the same expression    */
/* with postfix binary operators. The function prefix is similar, returning  */
/* the expression with prefix binary operators.                              */

char *postfix(struct pt_node *head);
char *prefix(struct pt_node *head);

int main(void) {

   char user_input[MAX_CHARS];
   int i;
   struct token *lexed_input = NULL;
   struct pt_node *head = NULL;

   printf("\nPlease enter an arithmetic expression in infix form. The expression may\n"
          "contain integer numbers, variable names, parentheses, and the operators\n"
          "^ (exponentiation), * (multiplication), / (division), + (addition), and\n"
          "- (subtraction). Variable names may contain lower-case letters and\n"
          "upper-case letters, but may not contain any other type of character. The\n"
          "expression must not contain any spaces.\n"
          "\nExample:\n"
          "   (a+3)+var^(b+282*c)\n\n"
          ">> ");
   if (fgets(user_input, MAX_CHARS, stdin) == NULL) {
      printf("Error receiving input!\n");
      return 1;
   }
   for (i = 0; i < MAX_CHARS - 1 && user_input[i] != '\n'; ++i)
      ;
   user_input[i] = '\0';

   /* The variable user_input is now a null-terminated array of chars
      which holds the expression the user has entered.                */

   lexed_input = input_lexer(user_input);
   head = expr(&lexed_input);
   printf("\nThe fully-parenthesized form of the expression:\n");
   printf("     %s\n", compl_par(head));
   printf("\nThe expression with postfix binary operators:\n");
   printf("     %s\n", postfix(head));
   printf("\nThe expression with prefix binary operators:\n");
   printf("     %s\n\n", prefix(head));

   return 0;
}

struct token *create_token(int type, char *init_char, int token_len) {

   struct token *result;
   char *string;
   int i;

   result  = (struct token *)malloc(sizeof(struct token));
   string = (char *)malloc((token_len + 1) * sizeof(char));
   if (result == NULL || string == NULL) {
      printf("Failed to create new token!\n");
      return NULL;
   }
   for (i = 0; i < token_len; ++i)
      string[i] = init_char[i];
   string[i] = '\0';
   
   result->type = type;
   result->string = string;
   result->prev = NULL;
   result->next = NULL;

   return result;
}

struct token *input_lexer(char *user_input) {

   struct token *result = NULL;
   struct token *new_token_ptr = NULL;
   int token_len;
   char *first_char;

   if (*user_input == '\0')
      return result;

   while (*user_input != '\0') {

      if (isalpha(*user_input)) {
         first_char = user_input;
         ++user_input;
         while (isalpha(*user_input))
            ++user_input;
         token_len = user_input - first_char;
         new_token_ptr = create_token(ATOM, first_char, token_len);
         new_token_ptr->prev = result;
         if (result != NULL)
            result->next = new_token_ptr;
         result = new_token_ptr;
      } else if (isdigit(*user_input)) {
         first_char = user_input;
         ++user_input;
         while (isdigit(*user_input))
            ++user_input;
         token_len = user_input - first_char;
         new_token_ptr = create_token(ATOM, first_char, token_len);
         new_token_ptr->prev = result;
         if (result != NULL)
            result->next = new_token_ptr;
         result = new_token_ptr;
      } else {
         token_len = 1;
         switch (*user_input) {
            case '(': new_token_ptr = create_token(LPAREN, user_input, token_len);
                      break;
            case ')': new_token_ptr = create_token(RPAREN, user_input, token_len);
                      break;
            case '^': new_token_ptr = create_token(EXP, user_input, token_len);
                      break;
            case '*': new_token_ptr = create_token(MUL, user_input, token_len);
                      break;
            case '/': new_token_ptr = create_token(DIV, user_input, token_len);
                      break;
            case '+': new_token_ptr = create_token(ADD, user_input, token_len);
                      break;
            case '-': new_token_ptr = create_token(SUB, user_input, token_len);
                      break;
            default : printf("Invalid input!\n");
                      return NULL;
         }
      new_token_ptr->prev = result;
      if (result != NULL)
         result->next = new_token_ptr;
      result = new_token_ptr;
      ++user_input;
      }
   }

   while (result->prev != NULL)
      result = result->prev;
   return result;
}

struct pt_node *create_node(int type, char *string, int num_childs) {
   struct pt_node *result;
   result = (struct pt_node *)malloc(sizeof(struct pt_node));
   if (result == NULL) {
      printf("Failed to allocate memory for new node!\n");
      return NULL;
   }
   if (num_childs != 0) {
      result->child_ptrs = (struct pt_node **)malloc(num_childs * sizeof(struct pt_node *));
      if (result->child_ptrs == NULL) { 
         printf("Failed to allocate memory for new node!\n");
         return NULL;
      }
   } else result->child_ptrs = NULL;
   result->type = type;
   result->string = string;
   result->num_childs = num_childs;
   return result;
}

struct pt_node *expr(struct token **lexed_in_ptr) {
   struct pt_node *result;
   result = create_node(NEXPR, "", 2);
   if (result == NULL) {
      printf("Failed to allocate memory for expr!\n");
      return NULL;
   }
   result->child_ptrs[0] = exprp(lexed_in_ptr);
   result->child_ptrs[1] = Expr(lexed_in_ptr);
   return result;
}

struct pt_node *Expr(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) return NULL;
   if (*lexed_in_ptr == NULL) {
      result = create_node(NeXPR, "", 1);
      if (result == NULL) {
         printf("Failed to allocate memory for Expr!\n");
         return NULL;
      }
      result->child_ptrs[0] = epsilon();
      return result;
   }
   if ((*lexed_in_ptr)->type == ADD ||
       (*lexed_in_ptr)->type == SUB) {
      result = create_node(NeXPR, "", 3);
      if (result == NULL) {
         printf("Failed to allocate memory for Expr!\n");
         return NULL;
      }
      switch((*lexed_in_ptr)->type) {
         case ADD:
                   result->child_ptrs[0] = add(lexed_in_ptr);
                   break;
         case SUB:
                   result->child_ptrs[0] = sub(lexed_in_ptr);
                   break;
      }
      result->child_ptrs[1] = exprp(lexed_in_ptr);
      result->child_ptrs[2] = Expr(lexed_in_ptr);
      return result;
   } else {
      result = create_node(NeXPR, "", 1);
      if (result == NULL) {
         printf("Failed to allocate memory for Expr!\n");
         return NULL;
      }
      result->child_ptrs[0] = epsilon();
      return result;
   }
}


struct pt_node *exprp(struct token **lexed_in_ptr) {
   struct pt_node *result;
   result = create_node(NEXPRP, "", 2);
   if (result == NULL) {
      printf("Failed to allocated memory for exprp!\n");
      return NULL;
   }
   result->child_ptrs[0] = exprpp(lexed_in_ptr);
   result->child_ptrs[1] = Exprp(lexed_in_ptr);
   return result;
}

struct pt_node *Exprp(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) return NULL;
   if (*lexed_in_ptr == NULL) {
      result = create_node(NeXPRP, "", 1);
      if (result == NULL) {
         printf("Failed to allocate memory for Exprp!\n");
         return NULL;
      }
      result->child_ptrs[0] = epsilon();
      return result;
   }
   if ((*lexed_in_ptr)->type == MUL ||
       (*lexed_in_ptr)->type == DIV) {
      result = create_node(NeXPRP, "", 3);
      if (result == NULL) {
         printf("Failed to allocate memory for Exprp!\n");
         return NULL;
      }
      switch((*lexed_in_ptr)->type) {
         case MUL:
                   result->child_ptrs[0] = mul(lexed_in_ptr);
                   break;
         case DIV:
                   result->child_ptrs[0] = quo(lexed_in_ptr);
                   break;
      }
      result->child_ptrs[1] = exprpp(lexed_in_ptr);
      result->child_ptrs[2] = Exprp(lexed_in_ptr);
      return result;
   } else {
      result = create_node(NeXPRP, "", 1);
      if (result == NULL) {
         printf("Failed to allocate memory for Expr!\n");
         return NULL;
      }
      result->child_ptrs[0] = epsilon();
      return result;
   }
}


struct pt_node *exprpp(struct token **lexed_in_ptr) {
   struct pt_node *result;
   struct pt_node *temp_result;
   if (lexed_in_ptr == NULL) return NULL;
   if (*lexed_in_ptr == NULL) return NULL;
   if ((*lexed_in_ptr)->type == ATOM) {
      if ((*lexed_in_ptr)->next != NULL &&
          (*lexed_in_ptr)->next->type == EXP) {
         result = create_node(NEXPRPP, "", 3);
         if (result == NULL) {
            printf("Failed to allocate memory for exprpp!\n");
            return NULL;
         }
         result->child_ptrs[0] = exprppp(lexed_in_ptr);
         result->child_ptrs[1] = expo(lexed_in_ptr);
         result->child_ptrs[2] = exprpp(lexed_in_ptr);
         return result;
      } else {
         result = create_node(NEXPRPP, "", 1);
         if (result == NULL) {
            printf("Failed to allocate memory for exprpp!\n");
            return NULL;
         }
         result->child_ptrs[0] = exprppp(lexed_in_ptr);
         return result;
      }
   } else {
      struct token *lookahead = *lexed_in_ptr;
      while (lookahead != NULL && lookahead->type != RPAREN)
         lookahead = lookahead->next;
      if (lookahead == NULL) {
         printf("Invalid input!\n");
         return NULL;
      } else {
         if (lookahead->next != NULL && lookahead->next->type == EXP) {
            result = create_node(NEXPRPP, "", 3);
            if (result == NULL) {
               printf("Failed to allocate memory for exprpp!\n");
               return NULL;
            }
            result->child_ptrs[0] = exprppp(lexed_in_ptr);
            result->child_ptrs[1] = expo(lexed_in_ptr);
            result->child_ptrs[2] = exprpp(lexed_in_ptr);
            return result;
         } else {
            result = create_node(NEXPRPP, "", 1);
            if (result == NULL) {
               printf("Failed to allocate memory for exprpp!\n");
               return NULL;
            }
            result->child_ptrs[0] = exprppp(lexed_in_ptr);
            return result;
         }
      } 
   }
}

struct pt_node *exprppp(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) return NULL;
   if (*lexed_in_ptr == NULL) return NULL;
   if ((*lexed_in_ptr)->type == ATOM) {
      result = create_node(NEXPRPPP, "", 1);
      if (result == NULL) {
         printf("Failed to allocate memory for exprppp!\n");
         return NULL;
      }
      result->child_ptrs[0] = atom(lexed_in_ptr);
      return result;
   } else {
      result = create_node(NEXPRPPP, "", 3);
      if (result == NULL) {
         printf("Failed to allocate memory for exprppp!\n");
         return NULL;
      }
      result->child_ptrs[0] = lparen(lexed_in_ptr);
      result->child_ptrs[1] = expr(lexed_in_ptr);
      result->child_ptrs[2] = rparen(lexed_in_ptr);
      return result;
   }
}

struct pt_node *lparen(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match left parenthesis!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == LPAREN) {
      result = create_node(NLPAREN, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for lparen!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match left parenthesis!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *rparen(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match right parenthesis!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == RPAREN) {
      result = create_node(NRPAREN, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for rparen!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match right parenthesis!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *expo(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match exponentiation operator!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == EXP) {
      result = create_node(NEXP, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for exp!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match exponentiation operator!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *mul(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match multiplication operator!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == MUL) {
      result = create_node(NMUL, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for mul!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match multiplication operator!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *quo(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match division operator!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == DIV) {
      result = create_node(NDIV, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for div!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match division operator!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *add(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match addition operator!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == ADD) {
      result = create_node(NADD, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for add!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match addition operator!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *sub(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match subtraction operator!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == SUB) {
      result = create_node(NSUB, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for sub!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match subtraction operator!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *atom(struct token **lexed_in_ptr) {
   struct pt_node *result;
   if (lexed_in_ptr == NULL) {
      printf("Invalid input!\n");
      return NULL;
   }
   if (*lexed_in_ptr == NULL) {
      printf("Failed to match atom!\n");
      return NULL;
   }
   if ((*lexed_in_ptr)->type == ATOM) {
      result = create_node(NATOM, (*lexed_in_ptr)->string, 0);
      if (result == NULL) {
         printf("Failed to allocate memory for atom!\n");
         return NULL;
      }
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return result;
   } else {
      printf("Failed to match atom!\n");
      *lexed_in_ptr = (*lexed_in_ptr)->next;
      return NULL;
   }
}

struct pt_node *epsilon(void) {
   struct pt_node *result;
   result = create_node(NEPSILON, "", 0);
   if (result == NULL) {
      printf("Failed to allocate memory for epsilon!\n");
      return NULL;
   }
   return result;
}

char *concat(char *str1, char *str2) {
   char *result;
   int i, j;
   result = (char *)malloc((strlen(str1) + strlen(str2) + 1) * sizeof(char));
   if (result == NULL) {
      printf("Failed to allocate memory for new string!\n");
      return "";
   }
   j = 0;
   for (i = 0; i < strlen(str1); ++i, ++j)
      result[j] = str1[i];
   for (i = 0; i < strlen(str2); ++i, ++j)
      result[j] = str2[i];
   result[j] = '\0';
   return result;
}

char *pre_compl_par(struct pt_node *head) {
   char *result = "";
   struct pt_node *dummy;
   if (head == NULL) {
      printf("Invalid input!\n");
      return "";
   }
   if (head->type == NEXPR &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, "(");
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while (dummy->num_childs == 3) {
         result = concat(result, "(");
         dummy = dummy->child_ptrs[2];
      }
      result = concat(result, pre_compl_par(head->child_ptrs[0]));
      result = concat(result, head->child_ptrs[1]->child_ptrs[0]->string);
      result = concat(result, pre_compl_par(head->child_ptrs[1]->child_ptrs[1]));
      result = concat(result, ")");
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while (dummy->num_childs != 1) {
         result = concat(result, dummy->child_ptrs[0]->string);
         result = concat(result, pre_compl_par(dummy->child_ptrs[1]));
         result = concat(result, ")");
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRP &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, "(");
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while (dummy->num_childs == 3) {
         result = concat(result, "(");
         dummy = dummy->child_ptrs[2];
      }
      result = concat(result, pre_compl_par(head->child_ptrs[0]));
      result = concat(result, head->child_ptrs[1]->child_ptrs[0]->string);
      result = concat(result, pre_compl_par(head->child_ptrs[1]->child_ptrs[1]));
      result = concat(result, ")");
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while(dummy->num_childs != 1) {
         result = concat(result, dummy->child_ptrs[0]->string);
         result = concat(result, pre_compl_par(dummy->child_ptrs[1]));
         result = concat(result, ")");
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRPP &&
       head->num_childs == 3) {
      result = concat(result, "(");
      result = concat(result, pre_compl_par(head->child_ptrs[0]));
      result = concat(result, "^");
      result = concat(result, pre_compl_par(head->child_ptrs[2]));
      result = concat(result, ")");
      return result;
   }
   if (head->type == NADD ||
       head->type == NSUB ||
       head->type == NMUL ||
       head->type == NDIV ||
       head->type == NATOM)
      return head->string;
   if (head->type == NEXPRPPP &&
       head->num_childs == 1)
      return pre_compl_par(head->child_ptrs[0]);
   if (head->type == NEXPR ||
       head->type == NEXPRP ||
       head->type == NEXPRPP)
      return pre_compl_par(head->child_ptrs[0]);
   if (head->type == NEXPRPPP)
      return pre_compl_par(head->child_ptrs[1]);
}

char *strip_parens(char *pre_compl_par) {
   char *result = "";
   char *temp = pre_compl_par;
   if (*pre_compl_par == '(') {
      ++temp;
      while (*temp != '\0')
         ++temp;
      --temp;
      *temp = '\0';
      result = concat(result, pre_compl_par + 1);
      return result;
   }
   else
      return pre_compl_par;
}

char *compl_par(struct pt_node *head) {
   return strip_parens(pre_compl_par(head));
}

char *postfix(struct pt_node *head) {
   char *result = "";
   struct pt_node *dummy;
   if (head == NULL) {
      printf("Invalid input!\n");
      return "";
   }
   if (head->type == NEXPR &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, postfix(head->child_ptrs[0]));
      result = concat(result, postfix(head->child_ptrs[1]->child_ptrs[1]));
      result = concat(result, concat(head->child_ptrs[1]->child_ptrs[0]->string, " "));
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while (dummy->num_childs != 1) {
         result = concat(result, postfix(dummy->child_ptrs[1]));
         result = concat(result, concat(dummy->child_ptrs[0]->string, " "));
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRP &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, postfix(head->child_ptrs[0]));
      result = concat(result, postfix(head->child_ptrs[1]->child_ptrs[1]));
      result = concat(result, concat(head->child_ptrs[1]->child_ptrs[0]->string, " "));
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while(dummy->num_childs != 1) {
         result = concat(result, postfix(dummy->child_ptrs[1]));
         result = concat(result, concat(dummy->child_ptrs[0]->string, " "));
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRPP &&
       head->num_childs == 3) {
      result = concat(result, postfix(head->child_ptrs[0]));
      result = concat(result, postfix(head->child_ptrs[2]));
      result = concat(result, "^ ");
      return result;
   }
   if (head->type == NATOM)
      return concat(head->string, " ");
   if (head->type == NEXPRPPP &&
       head->num_childs == 1)
      return postfix(head->child_ptrs[0]);
   if (head->type == NEXPR ||
       head->type == NEXPRP ||
       head->type == NEXPRPP)
      return postfix(head->child_ptrs[0]);
   if (head->type == NEXPRPPP)
      return postfix(head->child_ptrs[1]);
}

char *prefix(struct pt_node *head) {
   char *result = "";
   struct pt_node *dummy;
   if (head == NULL) {
      printf("Invalid input!\n");
      return "";
   }
   if (head->type == NEXPR &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, concat(head->child_ptrs[1]->child_ptrs[0]->string, " "));
      result = concat(result, prefix(head->child_ptrs[0]));
      result = concat(result, prefix(head->child_ptrs[1]->child_ptrs[1]));
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while (dummy->num_childs != 1) {
         result = concat(concat(dummy->child_ptrs[0]->string, " "), result);
         result = concat(result, prefix(dummy->child_ptrs[1]));
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRP &&
       head->child_ptrs[1]->num_childs == 3) {
      result = concat(result, concat(head->child_ptrs[1]->child_ptrs[0]->string, " "));
      result = concat(result, prefix(head->child_ptrs[0]));
      result = concat(result, prefix(head->child_ptrs[1]->child_ptrs[1]));
      dummy = head->child_ptrs[1]->child_ptrs[2];
      while(dummy->num_childs != 1) {
         result = concat(concat(dummy->child_ptrs[0]->string, " "), result);
         result = concat(result, prefix(dummy->child_ptrs[1]));
         dummy = dummy->child_ptrs[2];
      }
      return result;
   }
   if (head->type == NEXPRPP &&
       head->num_childs == 3) {
      result = concat(result, "^ ");
      result = concat(result, prefix(head->child_ptrs[0]));
      result = concat(result, prefix(head->child_ptrs[2]));
      return result;
   }
   if (head->type == NATOM)
      return concat(head->string, " ");
   if (head->type == NEXPRPPP &&
       head->num_childs == 1)
      return prefix(head->child_ptrs[0]);
   if (head->type == NEXPR ||
       head->type == NEXPRP ||
       head->type == NEXPRPP)
      return prefix(head->child_ptrs[0]);
   if (head->type == NEXPRPPP)
      return prefix(head->child_ptrs[1]);
}
