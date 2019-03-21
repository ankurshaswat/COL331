struct msg {
  int sender_pid;
  char msg[8];
  int bufferPosition;
  struct msg *next;
};

struct queue
{
    struct msg *head;
    struct msg *tail;
};