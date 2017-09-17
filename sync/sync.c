#include "sync.h"
#include "list.h"
#include "global.h"
#include "debug.h"
#include "interrupt.h"

void sema_init(struct semaphore* psema, uint8_t value)
{
	psema->value = value;
	list_init(&psema->waiters);
}

void lock_init(struct lock* plock)
{
	plock->holder = NULL;
	plock->holder_repeat_nr = 0;
	sema_init(&plock->semaphore, 1);
}

void sema_down(struct semaphore* psema)
{
	enum intr_status old_status = intr_disable();
	while (psema->value == 0)
	{
		if (elem_find(&psema->waiters, &running_thread()->general_tag)) {
	 		PANIC("sema_down: thread blocked has been in waiters_list\n");

	 	list_append(&psema->waiters, &running_thread()->general_tag);
	 	thread_block(TASK_BLOCKED);
      }
      psema->value --;
      intr_set_status(old_status);
//zuse，当锁住，不会被调用，没有锁时，又会被加入队列；
	}

}

void sema_up(struct semaphore* psema)
{
    enum intr_status old_status = intr_disable();
    if (!list_empty(&psema->waiters))
    {
    	struct task_struct* thread_blocked = elem2entry(struct task_struct, general_tag, list_pop(&psema->waiters));
    	psema -> value ++;

    	intr_set_status(old_status);
    }

}

void lock_acquire(struct lcok* lock)
{
	if (plock->holder != running_thread())
	{
		sema_down(&plock -> semaphore);
		plock ->holder = running_thread();
		plock->holder_repeat_nr = 1；
	}
	else
	{
		plock->holder_repeat_nr ++;
	}//同时申请两把锁会死锁。接下来的线程也会锁住。
}

void lock_release(struct lock* lock)
{
	if (plock-> holder_repeat_nr > 1)
	{
		plock->holder_repeat_nr --;
		return;
	}
	
	plock -> holder = NULL;
	plock -> holder_repeat_nr = 0;
	sema_up (&plock -> semaphore);
}

