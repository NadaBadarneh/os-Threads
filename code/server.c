#include "segel.h"
#include "request.h"
#include "Queue.h"
#include <unistd.h>
#include "stdbool.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//
pthread_cond_t empty_queue;
pthread_cond_t full_queue;
pthread_cond_t vip_working;
pthread_cond_t vip_empty;
pthread_mutex_t lock_all;

bool vip_curr_working;


void* execute(void* args);

typedef struct args{
    Queue run_queue;
    Queue  wait_queue;
    Queue vip_queue;
    threads_stats thread;
}* Args;

int createThread(int id, threads_stats *t, Queue r_q, Queue w_q, Queue vip_q, int threads_num) {
    if (r_q == NULL || w_q == NULL ) {
        return -1;
    }
    *t = (threads_stats) malloc(sizeof(**t));
    threads_stats thread = *t;

    if (thread == NULL) {
        return -1;
    }
    thread->id = id;
    thread->count = 0;
    thread->dynm_req = 0;
    thread->stat_req = 0;

    if (id == threads_num) {
        thread->is_vip_thread = true;
    }
    else {
        thread->is_vip_thread = false;
    }

    Args x = (Args) malloc(sizeof(*x));
    x->thread = thread;
    x->wait_queue = w_q;
    x->run_queue = r_q;
    x->vip_queue = vip_q;
    if (pthread_create(&thread->m_thread, NULL, execute, x) != 0) {
        return -1;
    }
    return 1;

}
int addThread(threads_stats thread,int x){
    if(thread==NULL){
        return -1;
    }
    if(x==1){
        thread->stat_req++;
        thread->count++;
        return 1;
    }
    if(x==2){
        thread->dynm_req++;
        thread->count++;
        return 1;
    }
    thread->count++;
    return 1;

}

void *execute(void *args) {

    if (args == NULL) {
        return NULL;
    }
    Args x = args;
    threads_stats thread = x->thread;
    Queue w_queue = x->wait_queue;
    Queue r_queue = x->run_queue;
    Queue vip_queue = x->vip_queue;
    if (thread == NULL || w_queue == NULL || r_queue == NULL ) {
        return NULL;
    }
    Node temp = NULL;
    if (x->thread->is_vip_thread) {
        pthread_mutex_lock(&lock_all);
        while (1) {
            // pthread_mutex_lock(&waiting_q_lock);

            while (vip_queue->members_num == 0) {
                vip_curr_working = false;
                pthread_cond_signal(&vip_working);
                pthread_cond_wait(&vip_empty, &lock_all);
            }
            //pthread_cond_wait(&vip_empty, &waiting_q_lock);
            vip_curr_working = true;


            temp = dequeue(vip_queue);


            //enqueue(r_queue, temp->r);


            struct timeval t;
            gettimeofday(&t, NULL);

            Request r = NULL;
            get_Request(temp, &r);


            timersub(&t, &r->ArrivalTime, &r->dispachTime);
            int fd_r = get_fd(r);
            pthread_mutex_unlock(&lock_all);


            requestHandle(r, thread, w_queue);

            close(fd_r);
            pthread_mutex_lock(&lock_all);



            dequeue_fd(vip_queue, fd_r);
            pthread_cond_signal(&full_queue);

            // pthread_mutex_unlock(&waiting_q_lock);

        }
        pthread_mutex_unlock(&lock_all);

    } else {
        pthread_mutex_lock(&lock_all);

        while (1) {
            // pthread_mutex_lock(&waiting_q_lock);


            while (((vip_curr_working) || (x->vip_queue->members_num > 0) && (!thread->is_vip_thread))) {
                pthread_cond_wait(&vip_working, &lock_all);
            }

            while (w_queue->members_num == 0) {
                pthread_cond_wait(&empty_queue, &lock_all);
            }
            temp = dequeue(w_queue);


            enqueue(r_queue, temp->r);


            struct timeval t;
            gettimeofday(&t, NULL);

            Request r = NULL;
            get_Request(temp, &r);


            timersub(&t, &r->ArrivalTime, &r->dispachTime);
            int fd_r = get_fd(r);
            pthread_mutex_unlock(&lock_all);


            requestHandle(r, thread, w_queue);
            // not sure if needed, assumed accept in server.c does open
            close(fd_r);


            Node t_delete = NULL;

            pthread_mutex_lock(&lock_all);

            dequeue_fd(r_queue, fd_r);

            pthread_cond_signal(&full_queue);

            // pthread_mutex_unlock(&waiting_q_lock);


        }
        pthread_mutex_unlock(&lock_all);

    }
}

// HW3: Parse the new arguments too
//void getargs(int *port, int argc, char *argv[])
//{
//    if (argc < 2) {
//	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//	exit(1);
//    }
//    *port = atoi(argv[1]);
//}

// void* execute(void* args) {
//     if (args == NULL) {
//         return NULL;
//     }
//     Args x = args;
//     threads_stats thread = x->thread;
//     Queue w_queue = x->wait_queue;
//     Queue r_queue = x->run_queue;
//     if (thread == NULL || w_queue == NULL || r_queue == NULL) {
//         return NULL;
//     }
//     Node temp = NULL;
//     while (1) {
//         pthread_mutex_lock(&waiting_q_lock);
//
//         while (w_queue->members_num == 0) {
//             pthread_cond_wait(&empty_queue, &waiting_q_lock);
//         }
//         temp=dequeue(w_queue);
//
//
//         enqueue(r_queue, temp->r);
//
//
//         struct timeval t;
//         gettimeofday(&t, NULL);
//
//         Request r=NULL;
//         get_Request(temp,&r);
//
//
//
//         timersub(&t,&r->ArrivalTime,&r->dispachTime);
//         int fd_r = get_fd(r);
//         pthread_mutex_unlock(&waiting_q_lock);
//
//
//         requestHandle(r,thread);
// // not sure if needed, assumed accept in server.c does open
//         close(fd_r);
//
//
//         Node t_delete = NULL;
//
//         pthread_mutex_lock(&waiting_q_lock);
//
//         dequeue_fd(r_queue,fd_r);
//
//         pthread_cond_signal(&full_queue);
//
//         pthread_mutex_unlock(&waiting_q_lock);
//
//
//
//
//     }
// }
// HW3: Parse the new arguments too
//void getargs(int *port, int argc, char *argv[])
//{
//    if (argc < 2) {
//	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
//	exit(1);
//    }
//    *port = atoi(argv[1]);
//}

void getargs(int *port, int argc, char *argv[],int* threads_num,int* queue_size,char* sched)
{
    if (argc <5) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    *port = atoi(argv[1]);
    *threads_num= atoi(argv[2]);
    *queue_size=atoi(argv[3]);
    for(int i=0;i<7;i++){
        if(argv[4][i]=='\0'){
            break;
        }
        sched[i]=argv[4][i];

    }

    if(*threads_num<1|| *queue_size<1){
        exit(1);
    }

}


void overloadhandler(int fd,Queue w_queue,Queue  r_queue, char* sched, struct timeval t){
    if(strcmp(sched, "block") == 0)
    {
        Request request = NULL;

        while((w_queue->members_num +r_queue->members_num) >= w_queue->size)
        {

            pthread_cond_wait(&full_queue, &lock_all);


        }


        if(createRequest(fd, t, &request) == -1)
            exit(-1);
        if(enqueue(w_queue, request) == -1)
            exit(-1);

        pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&lock_all);

        return;
    }
    if(strcmp(sched, "dt") == 0)
    {
        //pthread_mutex_unlock(&waiting_q_lock);


        Close(fd);
        pthread_cond_signal(&empty_queue);

        pthread_mutex_unlock(&lock_all);
        return;
    }
    if(strcmp(sched, "dh") == 0)
    {


        Node n=NULL;
        n=dequeue(w_queue);
        if(n==NULL){
            exit(-1);
        }
        close(n->r->fd);




        Request request = NULL;
        if(createRequest(fd, t, &request) == -1)
            exit(-1);

        if(enqueue(w_queue, request) == -1)
            exit(-1);

        pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&lock_all);

        return;

    }

    if(strcmp(sched, "bf") == 0)
    {

        while((w_queue->members_num +r_queue->members_num) != 0)
        {

            pthread_cond_wait(&full_queue, &lock_all);


        }
        Close(fd);
        pthread_cond_signal(&empty_queue);

        pthread_mutex_unlock(&lock_all);
        return;
    }


    if(strcmp(sched, "random")==0){
      if(w_queue->members_num == 0){
        close(fd);
        pthread_cond_signal(&empty_queue);
        pthread_mutex_unlock(&lock_all);
        }
        else{
            int half_size = ((w_queue->members_num) % 2 == 0) ? (w_queue->members_num) / 2 : (w_queue->members_num / 2)+1;
            for (int i = 0; i < half_size; i++) {
                int index = (rand() % (w_queue->members_num));

                Node iter = w_queue->root;
                for(int i = 0; i < index; i++)
                {
                    iter = iter->next;
                }

                int to_delete = iter->r->fd;
                dequeue_fd(w_queue, to_delete);
                Close(to_delete);
            }
             Request request = NULL;
             if(createRequest(fd, t, &request) == -1){
                    exit(-1);
             }
             if(enqueue(w_queue, request) == -1){
                     exit(-1);
             }
             pthread_cond_signal(&empty_queue);
             pthread_mutex_unlock(&lock_all);
         }
}
}

 void normal_handle(Queue q,Queue q2,Request r) {
    if (getRequestMetaData(r->fd)) {
        if (enqueue(q, r) == -1) {
            return ;
        }
        pthread_cond_signal(&vip_empty);

    } else {
        if (enqueue(q2, r) == -1) {
            return ;

        }
        pthread_cond_signal(&empty_queue);

        // pthread_mutex_unlock(&waiting_q_lock);
    }
    pthread_mutex_unlock(&lock_all);

}





int main(int argc, char *argv[]) {
    int listenfd, connfd, port, clientlen,threads_num,queue_size;
    char sched[8]={0};
    struct sockaddr_in clientaddr;
    getargs(&port, argc, argv,&threads_num,&queue_size,sched);
    Queue  running_queue=NULL;
    Queue waiting_queue=NULL;
    Queue vip_queue=NULL;
    pthread_mutex_init(&lock_all,NULL);
    pthread_cond_init(&empty_queue,NULL);
    pthread_cond_init(&full_queue,NULL);
    pthread_cond_init(&vip_empty, NULL);
    pthread_cond_init(&vip_working, NULL);

    if (init(&vip_queue, (queue_size)) == -1||init(&waiting_queue,(queue_size))==-1||init(&running_queue,threads_num)==-1) {
        return -1;
    }
    for(int i=0;i<threads_num+1;i++){
        threads_stats thread=NULL;
        if(createThread(i,&thread,running_queue,waiting_queue,vip_queue,threads_num)==-1){
            return -1;
        }
    }
    listenfd = Open_listenfd(port);

    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

        struct timeval  arrive;
        gettimeofday(&arrive, NULL);

        pthread_mutex_lock(&lock_all);
        if(running_queue->members_num + waiting_queue->members_num >= queue_size) {
            overloadhandler(connfd,waiting_queue,running_queue,sched,arrive);
        }
        else{
            Request r=NULL;

            if(createRequest(connfd,arrive,&r)==-1){
                return -1;
            }

            normal_handle(vip_queue,waiting_queue,r);






            //
            // HW3: In general, don't handle the request in the main thread.
            // Save the relevant info in a buffer and have one of the worker threads ``
            // do the work.
            //
            //
            // HW3: Create some threads...

        }
    }
}



    


 
