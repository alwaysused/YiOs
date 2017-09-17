bool sync_dir_entry(struct dir* parent_dir, struct dir_entry* p_de, void* io_buf)
{
	struct inode* dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    ASSERT(dir_size % dir_entry_size == 0);	 // dir_size应该是dir_entry_size的整数倍

    uint32_t dir_entrys_per_sec = (512 / dir_entry_size);       // 每扇区最大的目录项数目
    int32_t block_lba = -1;


   /* 将该目录的所有扇区地址(12个直接块+ 128个间接块)存入all_blocks */
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0};

    while(block_idx < 12)
    {
    	all_blocks[block_idx] = parent_dir -> inode-> i_sectors[block_idx];
    }


    block_idx = 0;
    while (block_idx < 140)
    {
    	if (all_blocks[block_idx] == 0)
    	{
    		uint32_t alloc_block_idx = block_bitmap_alloc(cur_part);
    		bitmap_sync(cur_part, alloc_block_idx - cur_part ->sb .data_table_lba, BLOCK_BITMAP);
    		parent_dir -> i_sectors[block_idx] = all_blocks[block_idx] = alloc_block_idx;
    		if (block_idx < 12)
    		{
		
    		}
    		else if (block_idx == 12)
    		{
    			alloc_block_idx = -1;
    			alloc_block_idx = block_bitmap_alloc(cur_part);
    			bitmap_sync(cur_part, alloc_block_idx - cur_part ->sb .data_table_lba, BLOCK_BITMAP);
    			all_blocks[block_idx] = alloc_block_idx;
    			ide_write(cur_part -> my_disk, parent_dir -> i_sectors[block_idx], all_blocks + 12, 1);
    		}
    		memset(io_buf, 0, 512);
    		memcpy(io_buf, p_de, sizeof (struct dir_entry));
    		ide_write(cur_part -> my_disk, alloc_block_idx, io_buf, 1);
    		return true;
    	}

    	else 
    	{
    		ide_read(cur_part -> my_disk, all_blocks[block_idx], io_buf, 1);
    		struct dir_entry *dir_e = (struct dir_entry *) io_buf;
    		uint32_t idx = 0;
    		while (idx < dir_entrys_per_sec)
    		{
    			if ((dir_e + idx) -> file_type == FT_UNKNOWN)
    			{
    				memcpy(dir_e + idx, p_de, sizeof (struct dir_entry));
    				return true;
    			}
    			++idx;
    		}
    		++block_idx;

    	}
    }
}