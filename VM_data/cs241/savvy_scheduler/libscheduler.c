/**
* Savvy_scheduler Lab
* CS 241 - Spring 2018
*/

#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "print_functions.h"
typedef struct _job_info {
    int id;
	double arrived_time;
	double running_time;
	double remain_time;
	double start_time;
	double last_start_time;
    double priority;
    /* Add whatever other bookkeeping you need into this struct. */
} job_info;

priqueue_t pqueue;
scheme_t pqueue_scheme;
comparer_t comparision_func;
double total_response_time = 0;
double total_waiting_time = 0;
double total_turnaround_time = 0;
double job_count = 0;
void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
	job *job_a = (job*) a;
	job *job_b = (job*) b;
	job_info *a_info = (job_info*)job_a->metadata; 
	job_info *b_info = (job_info*)job_b->metadata; 
	if (a_info -> arrived_time < b_info -> arrived_time) return -1;
	else return 1;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
	job *job_a = (job*) a;
	job *job_b = (job*) b;
	job_info *a_info = (job_info*)job_a->metadata; 
	job_info *b_info = (job_info*)job_b->metadata; 
	if (a_info -> priority < b_info -> priority) return -1;
    else if (a_info -> priority > b_info -> priority)return 1;
	else return break_tie(a,b);
}

int comparer_psrtf(const void *a, const void *b) {
    job *job_a = (job*) a;
	job *job_b = (job*) b;
	job_info *a_info = (job_info*)job_a->metadata; 
	job_info *b_info = (job_info*)job_b->metadata; 
	//printf("remain:%f,%f",a_info -> remain_time,b_info -> remain_time);
	if (a_info -> remain_time < b_info -> remain_time) return -1;
    else if (a_info -> remain_time > b_info -> remain_time)return 1;
	else return break_tie(a,b);
}

int comparer_rr(const void *a, const void *b) {
    return 0;
}

int comparer_sjf(const void *a, const void *b) {
	job *job_a = (job*) a;
	job *job_b = (job*) b;
	job_info *a_info = (job_info*)job_a->metadata; 
	job_info *b_info = (job_info*)job_b->metadata; 
	if (a_info -> running_time < b_info -> running_time) return -1;
    else if (a_info -> running_time > b_info -> running_time)return 1;
	else return break_tie(a,b);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO complete me!
	job_info *info = malloc (sizeof(job_info));
	info -> id = job_number;
	info -> arrived_time = time;
	info -> running_time = sched_data -> running_time;
	info -> priority = sched_data -> priority;
	info -> remain_time = sched_data -> running_time;
	info -> last_start_time = -1;
	info -> start_time = -1;
	newjob->metadata = (void*)info;
	priqueue_offer(&pqueue, (void*)newjob);
	job_count ++;
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO complete me!
	switch (pqueue_scheme) {
		case FCFS:
		case PRI:
		case SJF:
			if (!job_evicted ){
				//int_write(1,1);
				break;
			}
			//int_write(1,2);
			return job_evicted;
		
		case PSRTF:
			if (job_evicted){
				job_info *info = (job_info*)job_evicted->metadata; 
				if (info -> last_start_time != -1){
					double t = time - info->last_start_time;
					info -> remain_time -= t;
					info -> last_start_time = -1;
				}
				priqueue_offer(&pqueue, (void*)job_evicted);			
			}
			if (priqueue_peek(&pqueue)){
				job* newjob = (job*)priqueue_poll(&pqueue);
				job_info *info = (job_info*)newjob->metadata;
				if (info -> start_time == -1)
					info -> start_time = time;
				if (info -> last_start_time == -1)
					info -> last_start_time = time;
				return newjob;
			}
			else return NULL;
			//info -> remain_time -=
		case PPRI:
		case RR:
			if (job_evicted)
				priqueue_offer(&pqueue, (void*)job_evicted);
			break;
		default:
			printf("Did not recognize scheme\n");
			exit(1);
    }	
	if (priqueue_peek(&pqueue)){
		job* newjob = (job*)priqueue_poll(&pqueue);
		job_info *info = (job_info*)newjob->metadata;
		if (info -> start_time == -1)
			info -> start_time = time;
		return newjob;
	}
	else return NULL;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO complete me!
	job_info *info = (job_info*)job_done->metadata;
	double turnaround_time = time - info->arrived_time;
	total_turnaround_time += turnaround_time;
	double response_time = info->start_time - info->arrived_time;
	total_response_time += response_time;
	double waiting_time = (time - info->arrived_time) - info->running_time;
	total_waiting_time += waiting_time;
	free(job_done->metadata);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO complete me!
    return total_waiting_time/job_count;
}

double scheduler_average_turnaround_time() {
    // TODO complete me!
    return total_turnaround_time/job_count;
}

double scheduler_average_response_time() {
    // TODO complete me!
    return total_response_time/job_count;
}

void scheduler_show_queue() {
    // Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}
