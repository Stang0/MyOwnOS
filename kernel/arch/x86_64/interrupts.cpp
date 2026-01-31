#include "interrupts.hpp"
#include "ports.hpp"
#include "console.hpp"

// Access assembly stubs
extern "C" {
    void isr0(); void isr1(); void isr2(); void isr3(); void isr4(); void isr5(); void isr6(); void isr7();
    void isr8(); void isr9(); void isr10(); void isr11(); void isr12(); void isr13(); void isr14(); void isr15();
    void isr16(); void isr17(); void isr18(); void isr19(); void isr20(); void isr21(); void isr22(); void isr23();
    void isr24(); void isr25(); void isr26(); void isr27(); void isr28(); void isr29(); void isr30(); void isr31();
    // -------------------------
    void irq0(); void irq1(); void irq2(); void irq3(); void irq4(); void irq5(); void irq6(); void irq7();
    void irq8(); void irq9(); void irq10(); void irq11(); void irq12(); void irq13(); void irq14(); void irq15();
}

IdtEntry idt[256];
IdtPtr idt_ptr;
IsrHandler irq_routines[16] = {0};

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].isr_low = (base & 0xFFFF);
    idt[num].kernel_cs = sel;
    idt[num].ist = 0;
    idt[num].attributes = flags;
    idt[num].isr_mid = (base >> 16) & 0xFFFF;
    idt[num].isr_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

void pic_remap() {
    uint8_t a1, a2;

    a1 = inb(0x21); // Save masks
    a2 = inb(0xA1);

    outb(0x20, 0x11); io_wait(); // Start init sequence
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait(); // Vector offset 32 (Master)
    outb(0xA1, 0x28); io_wait(); // Vector offset 40 (Slave)
    outb(0x21, 0x04); io_wait(); // Tell Master about Slave at IRQ2
    outb(0xA1, 0x02); io_wait(); // Tell Slave its cascade identity
    outb(0x21, 0x01); io_wait(); // 8086 mode
    outb(0xA1, 0x01); io_wait();
    outb(0x21, a1);   io_wait(); // Restore masks
    outb(0xA1, a2);   io_wait();
}

void init_interrupts() {
    idt_ptr.limit = sizeof(IdtEntry) * 256 - 1;
    idt_ptr.base = (uint64_t)&idt;

    // Remap PIC to avoid conflict with exceptions
    pic_remap();

    // Fill IDT with ISRs (0-31)
    idt_set_gate(0, (uint64_t)isr0, 0x08, 0x8E);
    idt_set_gate(1, (uint64_t)isr1, 0x08, 0x8E);
    idt_set_gate(2, (uint64_t)isr2, 0x08, 0x8E);
    idt_set_gate(3, (uint64_t)isr3, 0x08, 0x8E);
    idt_set_gate(4, (uint64_t)isr4, 0x08, 0x8E);
    idt_set_gate(5, (uint64_t)isr5, 0x08, 0x8E);
    idt_set_gate(6, (uint64_t)isr6, 0x08, 0x8E);
    idt_set_gate(7, (uint64_t)isr7, 0x08, 0x8E);
    idt_set_gate(8, (uint64_t)isr8, 0x08, 0x8E);
    idt_set_gate(9, (uint64_t)isr9, 0x08, 0x8E);
    idt_set_gate(10, (uint64_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint64_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint64_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint64_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint64_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint64_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint64_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint64_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint64_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint64_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint64_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint64_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint64_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint64_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint64_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint64_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint64_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint64_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint64_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint64_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint64_t)isr31, 0x08, 0x8E);

    // IRQs (32-47)
    idt_set_gate(32, (uint64_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)irq1, 0x08, 0x8E);
    idt_set_gate(34, (uint64_t)irq2, 0x08, 0x8E);
    idt_set_gate(35, (uint64_t)irq3, 0x08, 0x8E);
    idt_set_gate(36, (uint64_t)irq4, 0x08, 0x8E);
    idt_set_gate(37, (uint64_t)irq5, 0x08, 0x8E);
    idt_set_gate(38, (uint64_t)irq6, 0x08, 0x8E);
    idt_set_gate(39, (uint64_t)irq7, 0x08, 0x8E);
    idt_set_gate(40, (uint64_t)irq8, 0x08, 0x8E);
    idt_set_gate(41, (uint64_t)irq9, 0x08, 0x8E);
    idt_set_gate(42, (uint64_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)irq15, 0x08, 0x8E);

    asm volatile("lidt %0" : : "m"(idt_ptr));
}

void irq_install_handler(int irq, IsrHandler handler) {
    if (irq >= 0 && irq < 16) {
        irq_routines[irq] = handler;
    }
}

extern "C" void isr_handler(Registers* regs) {
    kprint("Received Interrupt: ");
    // Convert int_no to string manually or print custom message
    // Just a placeholder for exceptions
    panic("Unhandled Exception");
}

extern "C" void irq_handler(Registers* regs) {
    IsrHandler handler = irq_routines[regs->int_no - 32]; // Get handler for IRQ by index // Function pointer for interrupt handler typedef void (*IsrHandler)(Registers* regs);
    if (handler) {
        handler(regs); // Call handler
    }

    // Send EOI to PICs
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20); // Slave PIC WHEN IRQ > 7 EOI 
    }
    outb(0x20, 0x20); // Master PIC EOI 
}
