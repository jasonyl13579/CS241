/**
* Extreme Edge Cases Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
	char** x = camelCaser("Hello.world.");
	char** y = camelCaser("The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.");
	char** z = camelCaser ("&&afaf Hhaifhiefl1alfj@jason*\naf%s");
//	while (*x){
	//char *y = *x;
	//printf("%s\n",*x);
	destroy(x);
	destroy(y);
	destroy(z);
//	}
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.
    return 1;
}
