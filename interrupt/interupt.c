#define IDT_DESC_CNT 0x30

static struct gate_desc[IDT_DESC_CNT];
intr_handler idt_table[IDT_DESC_CNT];

extern intr_handler intr_entry_table[IDT_DESC_CNT];

static void pic_init()
{
	outb(PIC_M_DATA, 0xfd);
	outb(PIC_M_DATA, 0xff);

	put_str(" pic_init done\n");
}

void register_handler(uint32_t vec_no, intr_handler handler)
{
	idt_table[vec_no] = handler;
}

