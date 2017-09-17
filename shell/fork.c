
//进程里只有全局数组，万一某一个size的block用完了，使用一个块，block里链表会pop
//怎么访问arena
void copy_pcb_vaddrbitmap_stack0(struct task_struct* child_thread, struct task_struct* parent_thread)
{
	memcpy(child_thread, parent_thread, PAGE_SIZE);

	child_thread -> pid = fork_pid();
	child_thread ->elapsed_ticks = 0;
	child_thread->parent_pid = parent_thread->pid;
    child_thread->general_tag.prev = child_thread->general_tag.next = NULL;
    child_thread->all_list_tag.prev = child_thread->all_list_tag.next = NULL;
    lock_desc_init(child_thread->u_block_desc);
    //uint32_t bitmap_len = DIV_ROUND_UP()
    uint32_t bitmap_pg_cnt = parent_thread->userprog_vaddr.vaddr_bitmap.bitmap_bytes_len;
    void *vaddr_bitmap = get_kernel_pages(bitmap_pg_cnt);
    memcpy(vaddr_btmp, child_thread->userprog_vaddr.vaddr_bitmap.bits, bitmap_pg_cnt * PG_SIZE);
    child_thread->userprog_vaddr.vaddr_bitmap.bits = vaddr_btmp;
    

}

static int32_t copy_process(struct task_struct* child_thread, struct task_struct* parent_thread)
{
	//整个页表；
	//bims,目录表，栈，是在
	//我觉得代码段没有被复制，因为eip被复制，栈被复制，变量也被复制了，堆里分配(对应虚拟地址之前就复制了，对应的物理页也被分配了。)的也被
	//虚拟内存
	void *buf_page = get_kernel_pages(1);

	copy_pcb_vaddrbitmap_stack0(child_thread, parent_thread) ;

	copy_body_stack3(child_thread, parent_thread, buf_page);

	child_thread->pgdir = create_page_dir();

	build_child_stack(child_thread);

	update_inode_open_cnts(child_thread);
	mfree_page(PF_KERNEL, buf_page, 1);
}

void build_child_stack()
{

	struct intr_stack *intr0stack = (struct intr0stack)((uint32_t)child_thread  + PAGE_SIZE - sizeof (struct intr_stack));
	intr0stack -> eax = 0;
	uint32_t *addr_for_ret = (uint32_t *)intr0stack;

	*addr_for_ret = (uint32_t)intr_exit;
	uint32_t *ebp_ptr_in_thread = (uint32_t) intr_exit - 5;

	child_thread ->self_kthread = ebp_ptr_in_thread;
}

static void update_inode(struct task_struct *thread)
{
	uint32_t fd = 3, file_idx = -1;
	while (fd < MAXFILES_OPEN_PROC)
	{
		if ( (file_idx = thread->fd_table[idx]) != -1)
			file_table[file_idx]->fd_inode->i_open_cnts ++;

		++ fd;
	}
}