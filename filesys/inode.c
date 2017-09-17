struct inode_position
{
	bool two_sec;
	uint32_t sec_lba;
	uint32_t off_size;
};
//抄，函数参数不是我能定的，但我能根据这个思考，接下来自己想
//内核空间是内核空间，所以在指针（地址）访问时，必然会访问到c00100000上的地址，对应的页目录项已经写好了；所以物理地址都是相同的
//第一个sys_malloc的已经在该物理地址里写下了数据，所以之后访问的都会看到同样的数据。相同的内核段（c01000)会访问到
//相同的页目录项。
static void inode_locate(struct partition* part, uint32_t inode_no, struct inode_position* inode_pos) 
{
	uint32_t off_size = inode_no * sizeof (struct inode);
	uint32_t off_sec_size = off_size % SECTOR_SIZE;

	uint32_t left_bytes = 512 - off_sec_size;
	if (left_bytes < sizeof (struct inode))
		inode_pos -> two_sec = true;
	else 
		inode_pos ->two_sec = false;

	inode_pos->sec_lba = part -> sb -> inode_table_lba + off_sec / SECTOR_SIZE;
	inode_pos -> off_size = off_sec_size;
}

struct inode* inode_open(struct partition* part, uint32_t inode_no) 
{
	struct list_elem* elem = part -> open_inode. head->next;
	struct inode *ptinode;
	while (elem != part -> &open_inode ->tail)
	{	
		ptinode = elem2entry(struct inode, inode_tag, elem);
		if (ptinode->inode_no == inode_no)
			return ptinode;
		++elem;
	}

	struct inode_position inode_pos;
	inode_locate(part, inode_no, &inode_pos)；
//可以的
	struct task_struct thread = running_thread();
	uint32_t *pg_ptr = thread ->pg_dir;
	thread ->pg_dir = NULL;
	struct inode *p2inode = sys_malloc(sizeof (struct inode));
	thread ->pg_dir = pg_ptr;

	if (!inode_pos. two_sec)
	{
		uint32_t *buf = sys_malloc(512);
		idt_read(part -> my_disk, inode_pos. sec_lba, buf, 1);

		mem_cpy(p2inode, buf + inode_pos .off_size, sizeof (struct inode));
	}
	else 
	{

	}
	sys_free(buf);

	list_push(&part->open_inode, &p2inode -> inode_tag);
	return p2inode;

}



