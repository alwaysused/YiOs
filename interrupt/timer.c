uint32_t ticks;

static void intr_timer_handler()
{
	struct task_struct* thread = runnning_thread();
	cur_thread -> elapsed_ticks ++;
	if (cur_thread->ticks == 0)
		schedule();
	else
		cur_thread ->ticks--;
}

void timer_init()
{
	put_str("timer_init start\n");

	register_handler(0x20, intr_timer_handler);
}


