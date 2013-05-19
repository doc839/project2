/* 
 * File:   main.c
 * Author: dennis o'connor
 *
 * Created on May 12, 2013, 4:41 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "tokenizer.h"
#include "utils.h"

#define BUF_SIZE 5

/*
 * 
 */
int main(int argc, char** argv) {

  TOKENIZER *tokenizer;
  char *string;
  char *tok;

  while(1) {
        
    string = getInput(BUF_SIZE);
    
    if(myStrCmp(string, "exit") == 0) {
        free(string);
        return 0;
    }
    
    /* tokenize string */
    printf( "Parsing '%s'\n", string );
    tokenizer = init_tokenizer( string );
    while( (tok = get_next_token( tokenizer )) != NULL ) {
      printf( "Got token '%s'\n", tok );
      free( tok );    /* free the token now that we're done with it */
    }
    free_tokenizer( tokenizer ); /* free memory */

    free(string);
  }

  printf( "\nBye!\n" );
  return 0;			/* all's well that end's well */
}

