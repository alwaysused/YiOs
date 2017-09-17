#ifndef _THREAD_H
#define _THREAD_H
#define PG_SIZE 4096
typedef void thread_func(void*);
enum task_status {
   TASK_RUNNING,
   TASK_READY,
   TASK_BLOCKED,
   TASK_WAITING,
   TASK_HANGING,
   TASK_DIED
};
struct intr_stack
{
    uint32_t vec_no;	 // kernel.S 宏VECTOR中push %1压入的中断号
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp_dummy;	 // 虽然pushad把esp也压入,但esp是不断变化的,所以会被popad忽略
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;

    uint32_t err_code;		 // err_code会被压入在eip之后
    void (*eip) (void);
    uint32_t cs;
    uint32_t eflags;
    void* esp;
    uint32_t ss;
};    	

struct thread_stack
{
	uint32_t ebp;
    uint32_t ebx;
    uint32_t edi;
    uint32_t esi;

    void (*eip)(thread_func* func, void* arg);
    void* unused_arg;
    thread_func* func;
    void* arg;

}
struct task_struct {
	 uint32_t* self_kstack;	 // 各内核线程都用自己的内核栈
	 enum task_status status;
	 char name[16];
	 uint8_t priority;
	 uint8_t ticks;	   // 每次在处理器上执行的时间嘀嗒数


	 uint32_t elapsed_ticks;

	 struct list_elem general_tag;				    

	 struct list_elem all_list_tag;

	 uint32_t* pgdir;             
	 uint32_t stack_magic;	
};
void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);
struct task_struct* running_thread(void);
void schedule(void);
void thread_init(void);
#endif