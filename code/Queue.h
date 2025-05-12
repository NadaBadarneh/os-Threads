

#ifndef HW3_OS_QUEUE_H
#define HW3_OS_QUEUE_H
#include "segel.h"
#include "stdbool.h"

typedef struct request_t{
    int fd;
    struct timeval ArrivalTime;
    struct timeval dispachTime;
    bool check;
}* Request;

typedef struct node{
    Request r;
    struct node* first;
    struct node* last;
    struct node* prev;
    struct node* next;
}* Node;


typedef struct queue_t{
    Node  root;
    Node  last;
    int size;
    int members_num;
    bool empty;
    bool full;

 

}*Queue;




int init(Queue *temp, int n);
int get_fd(Request r);
int createRequest(int fd,struct timeval  time,Request * req_1);
int enqueue(Queue q,Request r_1);
Node dequeue(Queue q);
Node dequeue_last(Queue q);
int dequeue_fd(Queue q,int fd);
void get_Request(Node temp, Request *r_1);
void increase_r(Queue q);
void decrease_r(Queue q);
int removeHead(Queue queue);

#endif //HW3_OS_QUEUE_H

