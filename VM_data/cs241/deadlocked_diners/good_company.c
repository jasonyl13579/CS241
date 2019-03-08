/**
* Deadlocked Diners Lab
* CS 241 - Spring 2018
*/

#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *work_interns(void *p) {
	Company *company = (Company *)p;
    pthread_mutex_t *left_intern, *right_intern, *temp_intern;
	int tries_left;
	int failed;
    while (running) {
		//Company_have_board_meeting(company);
		usleep(rand()%700 + 2000*Company_get_billable_days(company));
        left_intern = Company_get_left_intern(company);
        right_intern = Company_get_right_intern(company);
		if (left_intern == right_intern){
			Company_have_board_meeting(company);
			return NULL;
		}
		tries_left = 5;
		do{
			pthread_mutex_lock(left_intern);
			failed = (tries_left>0)? pthread_mutex_trylock(right_intern):pthread_mutex_lock(right_intern);
			if (failed){
				pthread_mutex_unlock(left_intern);
				temp_intern = left_intern;
				left_intern = right_intern;
				right_intern = temp_intern;
				tries_left -= 1;
			}
		}while(failed && running);
		if (!failed){
			 Company_hire_interns(company);
			 usleep(rand()%700 + 2000*Company_get_billable_days(company));
			 pthread_mutex_unlock(left_intern);
			 pthread_mutex_unlock(right_intern);
		}
        
	}
    return NULL;
}
