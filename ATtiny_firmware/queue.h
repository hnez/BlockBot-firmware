#ifndef _QUEUE_H_
#define _QUEUE_H_
#define count 50

struct queue_t{
  char values[count];
  int index;
};

void queue_init(struct queue_t *q);
int push(struct queue_t *q, char value);
char pop(struct queue_t *q);
int buffer_size(struct queue_t *q);
#endif
