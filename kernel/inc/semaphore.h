#include "semaphore_struct.h"

void sem_init(sem_t *, int, bool);
void sem_wait(sem_t *sem);
void sem_post(sem_t *sem);
void sem_destroy(sem_t *sem);
int sem_trywait(sem_t *sem);
