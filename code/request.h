#ifndef __REQUEST_H__
#define __REQUEST_H__
#include "Queue.h"
#include "stdbool.h"


typedef struct Threads_stats{
	int id;
    int count;
	int stat_req;
	int dynm_req;
	int total_req;
    pthread_t m_thread;
	bool is_vip_thread;
} * threads_stats;

// handle a request
void requestHandle(Request request, threads_stats t_stats, Queue waiting_queue);

//  Returns True/False if realtime event
int getRequestMetaData(int fd);

#endif
