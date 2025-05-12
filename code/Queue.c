
#include "Queue.h"

void get_Request(Node temp, Request * r_1){
    if (temp== NULL){
       return;
    }
    *r_1=temp->r;
}
int get_fd(Request r){
    if (r!=NULL){
        r->check=r->fd;
        return r->fd;

    }
    return -1;
}

int create_node(Request r,  Node *nada){

    *nada=(Node)malloc(sizeof(**nada));
     Node n=*nada;
    
    if(!n){
        return -1;
    }
    n->r=r;
    n->next= NULL;
    n->prev=NULL;
    n->first=NULL;
    n->last=NULL;

    return 1;


}

int init(Queue *nada,int n){
     *nada= (Queue)malloc(sizeof(**nada));
     Queue q=*nada;
     
    if(q==NULL){
        q->full=true;
        q->empty=false;
        return -1;
     }

    q->last=NULL;
    q->root=NULL;
    q->size=n;
    q->members_num=0;
    q->full=false;
    q->empty=true;

    return 1;
}

int enqueue(Queue q,Request r_1){
    if (q==NULL){
        dequeue(q);
        return -1;
    }
    Queue  temp_queue=NULL;

    Node n=NULL;
    int t=create_node(r_1,&n);
    if (t==-1){
        return -1;
    }
    dequeue(temp_queue);

   if (q->last== NULL){
       q->root=n;
       q->last=n;
       q->members_num++;
        
       return 1;
   }
   n->prev=q->last;
   q->last->next=n;
   q->last=n;

    q->members_num++;

    return 1;

}



int createRequest(int fd,struct timeval  time,Request *req_1){
    *req_1=(Request) malloc(sizeof(**req_1));
     Request req=*req_1;
    if(req==NULL){
        return -1;
    }
    req->fd=fd;
    req->ArrivalTime=time;
    // not picked up by a thread yet so no dispatch time
    req->dispachTime=time;
    return 1;

}

Node dequeue(Queue q)
{
    if (q==NULL || q->members_num==0){
        return NULL;
    }

    Node temp=q->root->next;
    Node removed_node=(q->root);
    if (temp != NULL){
        temp->prev=NULL;
        q->members_num--;
        q->root=temp;
        return removed_node;
    }
    q->root=NULL;
    q->last=NULL;
    q->members_num--;
    return removed_node;
}

Node dequeue_last(Queue q)
{
    if (q==NULL || q->members_num==0){
        return NULL;
    }

    Node temp=q->last->prev;
    Node removed_node=(q->last);
    if (temp != NULL){
        temp->next=NULL;
        q->members_num--;
        q->last=temp;
        return removed_node;
    }
    q->root=NULL;
    q->last=NULL;
    q->members_num--;
    return removed_node;
}

int dequeue_fd(Queue q,int fd){
    if (q==NULL){
        return -1;
    }
    Node temp=q->root;
    while(temp != NULL){
        if (temp->r->fd==fd){
           break;
          }
          
        temp=temp->next;
    }
    if(temp==NULL){return -1;}
      if(temp->prev)
        temp->prev->next = temp->next;
    if(temp->next)
        temp->next->prev = temp->prev;
    if(q->root == temp)
        q->root = temp->next;
    if(q->last == temp)
        q->last = temp->prev;

      q->members_num--;
return 1;

}


//
//int removeHead(Queue queue)
//{
//    // Assumes queue->mutex is locked.
//    if(queue == NULL)
//    {
//        return 0;
//    }
//
//	if(queue->members_num == 0)
//        return 0;
//
//    Node tmp = queue->last;
//    queue->last = queue->last->prev;
//    queue->last->next = NULL;
//    Close(tmp->r->fd);
//    //destroyNode(tmp);
//    queue->members_num--;
//    return 1;
//}
//
//
//
//int removeItem(Queue queue, int index)
//{
//    // Assumes queue->mutex is locked.
//    if(queue == NULL)
//        return 0;
//    if(index >= queue->members_num || index < 0)
//        return 0;
//
//    Node prev = queue->root;
//
//    for(int i = 0; i < index; i++)
//    {
//        prev = prev->next;
//    }
//
//    Node toDelete = prev->next;
//    prev->next = toDelete->next;
//    if(toDelete->next != NULL)
//        toDelete->next->prev = prev;
//    else
//        queue->last = toDelete->prev;
//    Close(toDelete->r->fd);
//    //delete(toDelete);
//    queue->members_num--;
//    return 1;
//}
//
//
//int removeRandomHalf(Queue queue)
//{
//    // Assumes queue->mutex is locked.
//    if(queue == NULL)
//        return 0;
//    int numToDelete = (queue->members_num + 1) / 2;
//    int finalRes = 1;
//    srand(time(NULL));
//    for(int i = 0; i < numToDelete; i++)
//    {
//        int index = ((int)rand()) % queue->members_num;
//
//        int res = removeItem(queue, index);
//        if(res == 0)
//            finalRes = 0;
//    }
//    return finalRes;


