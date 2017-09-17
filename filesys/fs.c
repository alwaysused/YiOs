static void partition_format(struct partition* part)
{
	uint32_t boot_sector_sects = 1;
	uint32_t super_block_sects = 1;
	uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILE_PER_PART , BITS_PER_SECTOR);
	uint32_t inode_table_sects = DIV_ROUND_UP(MAX_FILE_PER_PART * sizeof (struct inode), SECTOR_SIZE);
	uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;

    uint32_t block_bitmap_sects;
    block_bitmap_sects = DIV_ROUND_UP(free_sects, BITS_PER_SECTOR);
    /* block_bitmap_bit_len是位图中位的长度,也是可用块的数量 */
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects; 
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len, BITS_PER_SECTOR);

    struct super_block sb;
    sb.magic = 0x19590318;
    sb.sec_cnt = part->sec_cnt;
    sb.inode_cnt = MAX_FILES_PER_PART;
    sb.part_lba_base = part->start_lba;

    sb.block_bitmap_lba = sb.part_lba_base + 2;	 // 第0块是引导块,第1块是超级块
    sb.block_bitmap_sects = block_bitmap_sects;

    sb.inode_bitmap_lba = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;

    sb.inode_table_lba = sb.inode_bitmap_lba + sb.inode_bitmap_sects;
    sb.inode_table_sects = inode_table_sects; 

    sb.data_start_lba = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no = 0;
    sb.dir_entry_size = sizeof(struct dir_entry);


    ide_write(hd, part->start_lba + 1, &sb, 1);

    uint32_t buf_size = (sb. block_bitmap_sects >= sb .inode_bitmap_sects? sb. block_bitmap_sects:sb .inode_bitmap_sects);
    buf_size = (buf_size >= sb .inode_table_sects? buf_size : sb .inode_table_sects) * SECTOR_SIZE;
    uint8_t *buf = (uint8_t *)sys_malloc(buf_size);

    buf[0]| = 0x01;
    uint32_t bitmap_last_byte = s. block_bitmap_sects / 8;
    uint32_t bitmap_last_bit = s.block_bitmap_sects % 8;

    buf[bitmap_last_byte + 1]| = 0xff;
    for (i = 0; i < bitmap_last_bit; ++i)
    	buf[bitmap_last_byte] &= ~(1 << i);

    ide_write(hd, sb .block_bitmap_lba, buf, sb. block_bitmap_sects);

    memset(buf, 0, sizeof buf);
    buf[0]| = 0x01;
    ide_write(hd, sb. inode_bitmap_lba, buf, sb. inode_table_sects);

    memset(buf, 0, sizeof buf);
    struct inode *i = (struct inode *) buf;
    i -> i_no = 0;
    i -> i_size = 2 * sizeof (struct dir_entry);
    i->i_sectors[0] = sb.data_start_lba;	     // 由于上面的memset,i_sectors数组的其它元素都初始化为0 
    ide_write(hd, sb.inode_table_lba, buf, sb.inode_table_sects);

    memcpy(p_de->filename, ".", 1);
    p_de->i_no = 0;
    p_de->f_type = FT_DIRECTORY;
    p_de++;

   /* 初始化当前目录父目录".." */
    memcpy(p_de->filename, "..", 2);
    p_de->i_no = 0;   // 根目录的父目录依然是根目录自己
    p_de->f_type = FT_DIRECTORY;

    ide_write(hd, sb.data_start_lba, buf, 1);

    sys_free(buf);

}

