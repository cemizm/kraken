//#define APPLE_SEMAPHORE_H

#ifndef APPLE_SEMAPHORE_H
#define APPLE_SEMAPHORE_H

#include <dispatch/dispatch.h>

#define sem_t dispatch_semaphore_t

#define sem_init(sem, np, value) \
		*sem = dispatch_semaphore_create(value);

#define sem_wait(sem) \
		dispatch_semaphore_wait(*sem, DISPATCH_TIME_FOREVER);

#define sem_post(sem) \
		dispatch_semaphore_signal(*sem);

#define sem_destroy(sem) \
		dispatch_release(*sem);
#endif
