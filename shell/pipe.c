uint32_t pipe(int pipe[])
{
	uint32_t idx = get_global_slot();

	file_table[fd] .pos = 2;
	file_table[fd] .fd_inode = get_kernel_page(1);
	iqueue_init((struct ioqueue *)file_table[fd] .fd_inode);
	file_table[fd] .flag = F_PIPE;

	pipe[0] = pcb_fd_install(idx);
	pipe[1] = pcb_fd_install(idx);
 }

 sys_write(int fd, void  *buf, uint32_t size)
 {
 	if (is_pipe(fd))
 		pipe_write(fd, buf, size);
 	else
 	{

 	}
 }

 