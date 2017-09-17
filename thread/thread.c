#include "thread.h"

struct task_struct* main_thread;
struct list thread_ready_list;
struct list thread_all_list;
static struct list_elem* thread_tag;

extern void switch_to(struct task_struct* cur, struct task_struct* next);

struct task_struct* running_thread() {
   uint32_t esp; 
   asm ("mov %%esp, %0" : "=g" (esp));
  /* 取esp整数部分即pcb起始地址 */
   return (struct task_struct*)(esp & 0xfffff000);
}

static void kernel_thread(thread_func* function, void* func_arg) {
/* 执行function前要开中断,避免后面的时钟中断被屏蔽,而无法调度其它线程 */
   intr_enable();
   function(func_arg); 
}

void init_thread(struct task_struct* pthread,char* name, int prio)
{
	memset(pthread, 0, sizeof *pthread);
	strcpy(pthread->name, name);

	if (pthread = main_thread)
	{
		pthread->status = TASK_RUNNING;
	}
	else 
	{
		pthread -> status = TASK_READY;
	}
	pthread->self_kstack = (uint32_t*) (pthread + PG_SIZE);
	pthread -> priority = prio;
	pthread -> tick = prio;
	pthread -> elapsed_tics = 0;
	pthread ->pgdir = NULL;
	pthread -> stack_magic = 0x19870916;
}
void thread_create(struct task_struct* pthread, thread_func func, void* arg)
{
	pthread -> self_kstack -= sizeof (struct intr_stack);
	pthread -> self_kstack -= sizeof (struct thread_stack);

	struct thread_stack* kthread_stack = (struct thread_stack*)pthread ->self_kstack;
	kthread_stack ->eip = kernel_thread;
	kthread_stack -> arg = arg;
	kernel_thread -> func = func;
	kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;


}
struct task_struct* thread_start(char* name, int proi, thread_func func, void* arg)
{
	struct task_struct* thread = get_kernel_page(1);

	init_thread(thread, name, prio);
	thread_creat(thread, function, func_arg);

	list_append(&thread_ready_list, thread->general_tag);
	list_append(&thread_all_list, thread->general_tag);

	return thread;
}

static void make_main_thread()
{
	main_thread = running_thread();
	init_thread(main_thread, "main", 31);

	list_append(&thread_all_list, &main_thread->all_list_tag);
}

void schedule()
{
	ASSERT(intr_get_status() == INTR_OFF);
	struct task_struct* cur = running_thread();//忽略esp在调换中的动态变化
	//只考虑开始人为的设置，比如load.S设置的
	if (cur -> status == TASK_RUNNING)
	{
		list_append(&thread_ready_list, &cur -> general_tag);
		cur -> ticks = cur -> priority;
		cur -> status = TASK_READY;

	} 
	else
	{}
	thread_tag = NULL;

	thread_tag = list_pop(&thread_ready_list);
	struct task_struct* next = ele2entry(struct task_struct, thread_tag,  thread_tag);
	next -> status = TASK_RUNNING;
	switch_to(cur, next);


}


void thread_block(enum task_status stat)
{
	enum intr_status old_status = intr_disable();
	struct task_struct cur_thread = running _thread();
	cur_thread->status = stat;
	schedule();
	intr_set_status(old_status);
}//
void thread_unblock(struct task_struct* pthread)
{
	enum intr_status old_status = intr_disable();
	if (pthread->status != TASK_READY)
	{
		list_push(&thread_ready_list, &pthread->general_tag);
		pthread->status = TASK_READY;
	}
	intr_set_status(old_status);
}
void thread_init()
{
	put_str("thread_init start\n");
	list_init(&thread_ready_list);
	list_init(&thread_all_list);
	make_main_thread();
	put_str("thread_init done\n");
}



