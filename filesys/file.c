struct file file_table[MAX_FILE_OPEN];

int32_t get_free_slot_in_global(void) 
{
	int32_t idx = 3;
	while (idx < MAX_FILE_OPEN)
	{
		if (file_table[idx]. fd_inode == NULL)
			break;
		++idx;
	}
	if (idx == MAX_FILE_OPEN)
		return -1;
}
int32_t pcb_fd_install(int32_t globa_fd_idx) 
{
   struct task_struct* cur = running_thread();
   uint8_t local_fd_idx = 3; // 跨过stdin,stdout,stderr
   while (local_fd_idx < MAX_FILES_OPEN_PER_PROC) {
      if (cur->fd_table[local_fd_idx] == -1) {	// -1表示free_slot,可用
	 cur->fd_table[local_fd_idx] = globa_fd_idx;
	 break;
      }
      local_fd_idx++;
   }
   if (local_fd_idx == MAX_FILES_OPEN_PER_PROC) {
      printk("exceed max open files_per_proc\n");
      return -1;
   }
   return local_fd_idx;
}

void bitmap_sync(struct partition* part, uint32_t bit_idx, uint8_t btmp_type) 
{
   uint32_t off_size = bit_idx / 4096;
   uint32_t local_sec = off_size * BLOCK_SIZE;


   switch( btmp_type)
   {
        case INODE_BITMAP:
            ide_write(part -> my_disk, part->inode_bitmap_lba + off_size, local_sec, 1);
            break;
        case BLOCK_BITMAP:
            ide_write(part -> my_disk, part->block_bitmap_lba + off_size, local_sec, 1);    
            break;
   }
}



//图里的根目录一个块，有两个目录项的数据
int32_t file_create(struct dir* parent_dir, char* filename, uint8_t flag)
{
    void *io_buf = sys_malloc(1024);

    int32_t inode_no = innode_bitmap_alloc(cur_part);

    struct inode* new_file_inode = (struct inode*)sys_malloc(sizeof(struct inode)); 

    inode_init(inode_no, new_file_inode);

    int fd_idx = get_free_slot_in_global();
    file_table[fd_idx] .fd_inode = new_file_inode;
    file_table[fd_idx] .fd_pos = 0;
    file_table[fd_idx].fd_flag = flag;
    file_table[fd_idx].fd_inode->write_deny = false;

    struct dir_entry new_dir_entry;

    memset(&new_dir_entry, 0, sizeof(struct dir_entry));

    create_dir_entry(filename, inode_no, FT_REGULAR, &new_dir_entry);

    if (!sync_dir_entry(parent_dir, &new_dir_entry, io_buf))
    {
        printk("sync_dir_entry error\n");
    }
    memset(io_buf, 0, 1024);
   /* b 将父目录i结点的内容同步到硬盘 */
    inode_sync(cur_part, parent_dir->inode, io_buf);

    memset(io_buf, 0, 1024);
   /* c 将新创建文件的i结点内容同步到硬盘 */
    inode_sync(cur_part, new_file_inode, io_buf);

   /* d 将inode_bitmap位图同步到硬盘 */
    bitmap_sync(cur_part, inode_no, INODE_BITMAP);

    list_push(&cur_part->open_inodes, &new_file_inode->inode_tag);
    new_file_inode->i_open_cnts = 1;

    sys_free(io_buf);
    return pcb_fd_install(fd_idx);

}
#define secs_need(size) (size / SECTOR_SIZE + 1)
//函数file_open,定任务，变量的定义
int32_t file_write(struct file* file, const void* buf, uint32_t count)
{
    //搞清inode是否需要分配

    struct inode * p_inode = file -> fd_inode ;
    uint32_t used_size = p_inode ->i_size;
    uint32_t update_size = used_size + count;
    uint32_t idx = 0;
    if (p_inode -> i_sectors[0] == 0)
    {
         idx = block_bitmap_allco(cur_part);
        sync_block_bitmap(cur_part -> my_disk, idx - cur_part -> sb->data_table_lba, BLOCK_BITMAP);
        p_inode -> i_sectors[0] = idx;
    }
    bool alloc_need = secs_need(used_size) == secs_need(update_size);
    if (!alloc_need)
    {
    //接着后面写，后面在哪，i_size / sec + 1得到快数，-1 得到块组里的idx，(),i_size % sec
//靠感觉自己不加检测的先把代码搞出来，再去看书对答案，不好吧。
        idx = secs_need(used_size) - 1;
        uint32_t sec_idx = p_inode -> i_sectors[idx];
        uint32_t off_bytes = used_size % SECTOR_SIZE;
        uint8_t * io_buf = sys_malloc(512);
        ide_read(cur_part -> my_disk, sec_idx, io_buf, 1);
        memcpy(io_buf + off_bytes, buf, size);
        ide_write();


    }
}



void read()

{   

    uint32_t  *all_blocks = sys_malloc(560);

    uint32_t start_sec_idx = file->pos / SECTOR_SIZE;
    uint32_t end_sec_idx = (file->pos + count) / SECTOR_SIZE;
    uint32_t extra_idx = 0;

    if (start_sec_idx == end_sec_idx)
    {
        if (start_sec_idx < 12)
            all_blocks[start_sec_idx] = file->fd_inode->i_sectors[start_sec_idx];
        else 
        {   
            extra_idx = file->fd_inode->i_sectors[start_sec_idx];
            ide_read(cur_part->my_disk, extra_idx, all_blocks + 12, 1);
        }
    }
    else 
    {
        if (start_sec_idx < 12 && end_sec_idx < 12)
        {
            for (uint32_t i = start_sec_idx; i <= end_sec_idx; ++i)
                all_blocks[i] = file->fd_inode->i_sectors[i];
        }
        else if (start_sec_idx <12 && end_sec_idx >= 12)
        {
            for (uint32_t i = start_sec_idx; i < 12; ++i)
                all_blocks[i] = file->fd_inode->i_sectors[i];
            extra_idx = file->fd_inode->i_sectors[start_sec_idx];
            ide_read(cur_part->my_disk, extra_idx, all_blocks + 12, 1);
        }
        else if (start_sec_idx >=12 && end_sec_idx >= 12)
        {
            extra_idx = file->fd_inode->i_sectors[start_sec_idx];
            ide_read(cur_part->my_disk, extra_idx, all_blocks + 12, 1);
        }
    }

    uint32_t left_size = count;
    uint32_t sec_lba_idx, sec_idx, off_size, sec_left_size, chunk_size;
    while (left_size > 0)
    {
        sec_lba_idx = file->pos / SECTOR_SIZE;
        sec_idx = all_blocks[sec_lba_idx];
        off_size = file->pos % SECTOR_SIZE;
        sec_left_size = SECTOR_SIZE - off_size;
        chunk_size = sec_left_size > left_size? left_size : sec_left_size;
        memset(io_buf, 0 , SECTOR_SIZE);
        ide_read(cur_part -> my_disk, sec_idx, io_buf, 1);
        memcpy(buf, io_buf + off_size, chunk_size);

        file->pos += chunk_size;
        left_size -= chunk_size;
        buf += chunk_size;
    }
}






