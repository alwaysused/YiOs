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