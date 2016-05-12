#include <stdio.h>
#include "queue.h"
#define count 50


void queue_init(struct queue_t *q){
  int i;
  q->index = -1;
  for(i=0;i<count;i++){
    q->values[i]=0;
  }
}


int push(struct queue_t *q, char value){
  if(q->index>=(count-1)){
    return -1; // full
  }
  q->index++;
  q->values[q->index]=value;
  return q->index;
}


char pop(struct queue_t *q){
  if(q->index<=-1){
    return 0; //empty
  }
  q->index--;
  char temp = q->values[0];
  int i;
  for(i=1;i<count;i++){
    q->values[i-1]=q->values[i];
  }
  return temp;
}

int buffer_size(struct queue_t *q){
  return q->index + 1;
}
