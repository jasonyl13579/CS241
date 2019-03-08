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
char *testcases[]= {"Hello.world.",
					"The Heisenbug is an incredible creature. Facenovel servers get their power from its indeterminism. Code smell can be ignored with INCREDIBLE use of air freshener. God objects are the new religion.",
					" &&afaf handCODE@jason*%s%d...",
					"space test      i am AN UIUC student >241 sO/ difficult",
					"123\\06789.",
					"\\n\\n",
					"",
					NULL
					};
char *testans1[]={"hello","world"};
char *testans2[]={"theHeisenbugIsAnIncredibleCreature",
				"facenovelServersGetTheirPowerFromItsIndeterminism",
				"codeSmellCanBeIgnoredWithIncredibleUseOfAirFreshener",
				"godObjectsAreTheNewReligion"};	
char *testans3[]={
        "",
        "",
        "afafHandcode",
        "jason",
        "",
        "s",
        "d",
        "",
        "",
};
char *testans4[]={
        "spaceTestIAmAnUiucStudent",
        "241So"
};
char *testans5[]={
        "123",
		"06789"
};
char *testans6[]={
        "",
        "n"
};
int cases = 8;
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
	char** x[16];  
	int success = 1;
	for (int i=0;i<cases;i++){
		x[i] = camelCaser(testcases[i]);
		//destroy(x[i]);
	}
	int size =(int)( sizeof(testans1)/sizeof(testans1[0]));
	//printf("%s:%s\n",x[0][0],testans1[0]);
	//printf("%s:%s%d\n",x[0][1],testans1[1],size);
	for (int i=0;i<size;i++){
		if(strcmp(testans1[i],x[0][i])!=0){success = 0;puts("1");}
	}
	size =(int)( sizeof(testans2)/sizeof(testans2[0]));
	for (int i=0;i<size;i++){
		if(strcmp(testans2[i],x[1][i])!=0){success = 0;puts("2");}
	}
	size =(int)( sizeof(testans3)/sizeof(testans3[0]));
	for (int i=0;i<size;i++){
		if(strcmp(testans3[i],x[2][i])!=0){success = 0;puts("3");}
	}
	size =(int)( sizeof(testans4)/sizeof(testans4[0]));
	for (int i=0;i<size;i++){
		if(strcmp(testans4[i],x[3][i])!=0){success = 0;puts("4");}
	}
	size =(int)( sizeof(testans5)/sizeof(testans5[0]));
	for (int i=0;i<size;i++){
		if(x[4][i]!= NULL && strcmp(testans5[i],x[4][i])!=0){success = 0;puts("5");}
	}
	size =(int)( sizeof(testans6)/sizeof(testans6[0]));
	for (int i=0;i<size;i++){
		if(x[5]!= NULL && x[5][i]!= NULL && strcmp(testans6[i],x[5][i])!=0){success = 0;puts("6");}
	}
	if (x[6]!= NULL && *x[6]!= NULL)success = 0;
	if (x[7] != NULL)success = 0;
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.
    for (int i=0;i<cases;i++){
		destroy(x[i]);
	}
	if (success ==1)return 1;
	else return 0;
}
