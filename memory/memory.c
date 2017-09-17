#define PAGE_SIZE　4096
#define MEM_BITMAP_BASE 0xc009a000
#define K_HEAP_START 0xc0100000

struct pool {
   struct bitmap pool_bitmap;	 // 本内存池用到的位图结构,用于管理物理内存
   uint32_t phy_addr_start;	 // 本内存池所管理物理内存的起始地址
   uint32_t pool_size;		 // 本内存池字节容量
   struct lock lock;
};
struct pool ker_pool, user_pool;
struct virtual_addr kernel_vaddr;

enum pool_flag
{
	PF_KERNEL = 1;
	PF_USER = 2;
};
static void* vaddr_get(enum pool_flag ,int pg_cnt)
{	
	uint32_t vaddr_start = 0, bit_idx_start = 0;
	uint32_t cnt = 0;
	if (pool_flag == PF_KERNEL)
	{
		bit_idx_start = bitmap_scan(&kernel_vaddr. vaddr_bitmap, pg_cnt);
		while(cnt < pg_cnt)
			bitmap_set(&kernel_vaddr. vaddr_bitmap, bit_idx_start + cnt++, 1);
		vaddr_start = kernel_vaddr. vaddr_start + bit_idx_start * PAGE_SIZE;
			

	}
	else
	{
		struct task_struct* cur = running_thread();
		bit_idx_start = bitmap_scan(&cur->userprog_vaddr. vaddr_bitmap, pg_cnt);

		while(cnt < pg_cnt) {
	 bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx_start + cnt++, 1);
      }
      vaddr_start = cur->userprog_vaddr.vaddr_start + bit_idx_start * PG_SIZE;
	}
	return vaddr_start;
}
void pte_ptr(uint32_t vaddr)
{
	return (uint32_t*)(0xffc00000 + (vaddr&0xffc00000)>>10 +PDE_IDX(vaddr) * 4);

}
void pde_ptr(uint32_t vaddr)
{
	return (uint32_t* )(0xfffff000 + PTE_IDX(vaddr) * 4);
}

void mem_pool_init(uint32_t all_mem)
{	
	put_str(" mem_pool_init start\n");
	int used_mem = 256 * PAGE_SIZE + 0x100000;
	int free_mem = all_mem - use_mem;
	int free_page = free_mem / PAGE_SIZE;

	int ker_page = free_page / 2;
	int user_page = free_page - ker_page;

	ker_pool. pool_size = ker_page * PAGE_SIZE;
	user_pool.  pool_size= user_page * PAGE_SIZE;

	ker_pool. phy_addr_start = used_mem;//
	user_pool. phy_addr_start = ker_pool. phy_addr_start + ker_page * PAGE_SIZE;

	ker_pool. pool_bitmap. bits = (void*) MEM_BITMAP_BASE;//
	ker_pool. pool_bitmap. btmp_bytes_len = ker_page / 8;
	user_pool. pool_bitmap. bit = ker_pool. pool_bitmap. bits + ker_pool. pool_bitmap. btmp_bytes_len;
	user_pool. pool_bitmap. btmp_bytes_len = user_size / 8;
   	
   	put_str("      kernel_pool_bitmap_start:");put_int((int)kernel_pool.pool_bitmap.bits);
   	put_str(" kernel_pool_phy_addr_start:");put_int(kernel_pool.phy_addr_start);
   	put_str("\n");
   	put_str("      user_pool_bitmap_start:");put_int((int)user_pool.pool_bitmap.bits);
   	put_str(" user_pool_phy_addr_start:");put_int(user_pool.phy_addr_start);
   	put_str("\n");

   	bitmap_init(&kernel_pool.pool_bitmap);
   	bitmap_init(&user_pool.pool_bitmap); 

   	kernel_vaddr. vaddr_bitmap. bits = (void*) (user_pool. pool_bitmap. bits + user_pool. pool_bitmap. btmp_bytes_len);
   	kernel_vaddr. vaddr_bitmap. btmp_bytes_len = kernel_pool.pool_bitmap. btmp_bytes_len;
   	kernel_vaddr. vaddr_start = K_HEAP_START;

   	bitmap_init(&kernel_vaddr.vaddr_bitmap);
   	put_str("   mem_pool_init done\n");

}

void* pmalloc (struct pool* pool)
{
	uint32_t phy_idx = bitmap_scan(pool->pool_bitmap, 1);
	bitmap_set(&pool->pool_bitmap, phy_idx, 1);
	return pool->phy_addr_start + phy_idx * PAGE_SIZE;
}

static void* page_table_add(void* vaddr, void* phy_addr)
{
	void* pde = PDE_IDX(vaddr);
	void* pte = PTE_IDX(vaddr);

	if (*pde & 0x00000001)
	{
		if (!(*pte & 0x00000001))
		{
			*pte = phy_addr|PG_US_U|PG_RW_W|PG_P_1;

		}
		else
			{PANIC("pte repeat");}

	}
	else if (!(*pde & 0x00000001))
	{
		*pde = pmalloc(&ker_pool);
		memset((void*) pte& 0xfffff000, 0, PAGE_SIZE);

		*pte = phy_addr|PG_US_U|PG_RW_W|PG_P_1;
		
	}
}

void* malloc_page(enum pool_flag pfg, uint32_t pg_cnt)
{
	uint32_t vaddr =vaddr_get(pfg, pg_cnt);
	uint32_t cnt = pg_cnt;
	uint32_t vaddr_start = vaddr;
	struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
	while(cnt++ < pg_cnt)
	{
		uint32_t phy_addr = pmalloc(mem_pool);
		page_table_add(vaddr, phy_addr);
		vaddr += PAGE_SIZE;
	}
	return (void*) vaddr_start;
}
void* get_kernel_page(uint32_t cnt)
{
	return malloc_page(PF_KERNEL, cnt);
}

void mem_init()
{	
	put_str("mem_init start\n");
	uint32_t mem_bytes_total = (*(uint32_t*)(0xb00));
	mem_pool_init(mem_bytes_total);
	put_str("mem_init done\n");
}

void* get_user_page(uint32_t pg_cnt)
{
	lock_acquire(&user_pool. lock);
	void* vaddr = malloc_page(PF_USER, pgcnt);
	memset(vaddr, 0, pg_cnt * PG_SIZE);
	lock_release(& user_pool.lock);
	return addr;
}
uint32_t addr_v2p(uint32_t vaddr) {
   uint32_t* pte = pte_ptr(vaddr);
/* (*pte)的值是页表所在的物理页框地址,
 * 去掉其低12位的页表项属性+虚拟地址vaddr的低12位 */
   return ((*pte & 0xfffff000) + (vaddr & 0x00000fff));
}
void* get_a_page(enum pool_flags pf, uint32_t vaddr) {
   struct pool* mem_pool = pf & PF_KERNEL ? &kernel_pool : &user_pool;
   lock_acquire(&mem_pool->lock);

   /* 先将虚拟地址对应的位图置1 */
   struct task_struct* cur = running_thread();
   int32_t bit_idx = -1;

/* 若当前是用户进程申请用户内存,就修改用户进程自己的虚拟地址位图 */
   if (cur->pgdir != NULL && pf == PF_USER) 
   {
      bit_idx = (vaddr - cur->userprog_vaddr.vaddr_start) / PG_SIZE;
      ASSERT(bit_idx > 0);
      bitmap_set(&cur->userprog_vaddr.vaddr_bitmap, bit_idx, 1);

   } else if (cur->pgdir == NULL && pf == PF_KERNEL){
/* 如果是内核线程申请内核内存,就修改kernel_vaddr. */
      bit_idx = (vaddr - kernel_vaddr.vaddr_start) / PG_SIZE;
      ASSERT(bit_idx > 0);
      bitmap_set(&kernel_vaddr.vaddr_bitmap, bit_idx, 1);
   } else {
      PANIC("get_a_page:not allow kernel alloc userspace or user alloc kernelspace by get_a_page");
   }

   void* page_phyaddr = palloc(mem_pool);
   if (page_phyaddr == NULL) {
      return NULL;
   }
   page_table_add((void*)vaddr, page_phyaddr); 
   lock_release(&mem_pool->lock);
   return (void*)vaddr;
}


struct 
void sys_malloc(uint32_t size)
{	
	enum pool_flags PF;
	struct pool *mem_pool;
	uint32_t pool_size;
	struct mem_block_desc *descs;
	struct arena *a;
	struct mem_block *b;

	struct task_struct *thread = running_thread();

	if (thread->pgdir == NULL)
	{
	   PF = PF_KERNEL; 
       pool_size = kernel_pool.pool_size;
       mem_pool = &kernel_pool;
       descs = k_block_descs;
	}
	else 
	{
		PF = PF_USER;
		pool_size = user_pool. pool_size;
		mem_pool = &user_pool;
		descs = thread -> u_block_decs;
	}

	if (!(size > 0 && size < pool_size))
		return NULL;
	lock_acquire(&mem_pool);
	if (size > 1024)
	{
		int pg_cnt = DIV_ROUND_U(size, PAGE_SIZE);
		a = malloc_page(pf, pg_cnt);
	 	a->desc = NULL;
		a->cnt = page_cnt;
	 	a->large = true;
	 	return (void*) (a+1);
	}
	else 
	{
		uint32_t desc_idx = 0;
		for (desc_idx = 0; desc_idx < DESC_CNT; ++desc_idx)
		{
			if ( size <= descs[desc_idx])
				break;
		}
		if (empty(descs[desc_idx] .free_list))
		{
			a = malloc_page(pf, 1);

			a->cnt = descs[desc_idx] .blocks_per_arena;
			a->desc = &descs[desc_idx];
			a -> large = false;
			b = (struct mem_block *)a+1;

			for ( int i = 0; i < a-> cnt; ++i)
			{
				list_append(&descs[desc_idx]. free_list, &b->free_elem);
				b += descs[desc_idx]. block_size; 
			}

			b = elem2entr(struct mem_block, free_elem, list_pop(descs[desc_idx]. free_list));
			a = block2arena(b);
			-- a->cnt;
			lock_release(&mem_pool .lock); 
		}
	}


}
#define block2arena(b) b & 0xffff000