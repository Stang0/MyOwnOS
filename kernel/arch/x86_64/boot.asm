;hint 
;; dd is mean double word (4 bytes)
;; dw is mean word (2 bytes)
;; db is mean byte (1 byte)
;; mov is mean move   
;; call is mean call a function
;; lgdt is mean load global descriptor table
;; jmp is mean jump to a label


section .multiboot_header
header_start: ; tell the bootloader where the OS starts
    dd 0xe85250d6                ; Magic number (multiboot 2)
    dd 0                         ; 0 is mean i386 protected mode for multiboot 
    dd header_end - header_start ; length of kernel.elf

    ; standardize multiboot tell if lead the 4 slots of memory sum the sum should be 0
    ;slot 1 0xe85250d6
    ;slot 2 0
    ;slot 3 header_end - header_start
    ;slot 4 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
    ;the sum of all slots should be 0 
    ; Checksum = 0 - (MagicNumber + Architecture + Length) 
    ; why in code I use 0x100000000 ? 
    ; because  in hexadecimal numbers Maximum value is 0xffffffff if we add 1 to it. it will overflow to 0x100000000
    ; so 0x100000000 is mean 0 in decimal because it overflow 
    ; so it mean 0x100000000 = 0
    ; and why i use it ? 
    ; because in another case i use 0 Assembler will confused with it  
    ; so 0x100000000 is the safe value to use 
    ; show how to calculate it 
    ; 100000000 - (0xe85250d6 + 0 + 0x18)
    ; 1_0000_0000 -  e852_50ee 
    ; = 17ad af12
    ; and finally lead slot1 to slot4 sum should be 0x100000000
    ; to check it is os multiboot or not we use checksum
    
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

    ; End tag
    dw 0  ; type
    dw 0  ; flags
    dd 8  ; size get from tag dw(2) + dw(2) + dd(4) = 8 bytes
header_end:

section .text ; mean the code section to load in memory for Executable code 
global _start ; is mean the entry point of the linker to start from it 
extern kernel_main ; refernace to the external function kernel_main in kernel.cpp when linker link it will call kernel_main function

bits 32 ; mean the code is in 32 bit mode 
_start: ; This is the entry point of the kernel  when Bootloader load the kernel it will start from here
    mov esp, stack_top           ; set the stack pointer to the top of the stack
    mov edi, ebx                 ; Save Multiboot info pointer

    call check_multiboot         ; check if the kernel is loaded by a multiboot loader
    call check_cpuid             ; check if the CPU supports CPUID instruction
    call check_long_mode         ; check if the CPU supports long mode

    call set_up_page_tables      ; set up the page tables
    call enable_paging           ; enable paging

    lgdt [gdt64.pointer]        ; load the GDT
    jmp gdt64.code_segment:long_mode_start ; jump to the long mode start

; Prints "ERR: " and the error code in AL to VGA buffer 0xb8000
; ASCII 'E'=0x45, 'R'=0x52, ':'=0x3a
error: ; Error handler for Bootloader
    mov dword [0xb8000], 0x4f524f45 ;[0xb8000] is the VGA buffer  and 0x4f524f45 is the ASCII code for "ER" and hex red color
    mov dword [0xb8004], 0x4f3a4f52 ;[0xb8004] is the next 4 bytes of the VGA buffer to print "R:" and concat with "ER" =  "ERR:"
    mov dword [0xb8008], 0x4f204f20 ;[0xb8008] is the next 4 bytes of the VGA buffer to print "  " and concat with "ERR:" =  "ERR: "
    mov byte  [0xb800a], al    ; al is the 8 bit in RAX register for collect error code 
    hlt ; sleep the CPU

check_multiboot:
    cmp eax, 0x36d76289 ; compare eax with 0x36d76289
    jne .no_multiboot ; if not equal jump to .no_multiboot
    ret ; return 
.no_multiboot: 
    mov al, "M" ; move "M" to AL register
    jmp error ; jump to error

check_cpuid:
    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    pushfd ;push flag to stack 
    pop eax ; pop flag form stack to eax 
    mov ecx, eax ; move eax to ecx 
    xor eax, 1 << 21 ; 1 << 21 create 1 in bit 21  for xor  with eax to flip the bit 21  
    push eax  ;push eax to stack 
    popfd ;pop flag form stack  
    pushfd ;push flag to stack 
    pop eax ; pop flag form stack to eax 
    push ecx ;push ecx to stack 
    popfd ;pop flag form stack 
    cmp eax, ecx ; compare eax with ecx 
    je .no_cpuid ; if equal jump to .no_cpuid
    ret ; return 
.no_cpuid:
    mov al, "C" ; move "C" to AL register
    jmp error ; jump to error

check_long_mode:
    ; Test if extended processor info in available
    mov eax, 0x80000000 ; call cpuid to get the highest input value for extended processor info
    cpuid ; call cpuid to get the highest input value for extended processor info
    cmp eax, 0x80000001 ; compare eax with 0x80000001
    jb .no_long_mode ; if less jump to .no_long_mode

    ; Test if long mode is available
    mov eax, 0x80000001 ; call cpuid to get the highest input value for extended processor info
    cpuid ; call cpuid to get the highest input value for extended processor info into edx register 
    test edx, 1 << 29 ; check in bit 29 of edx register because bit29 is the long mode bit 
    jz .no_long_mode ; if 0  jump to .no_long_mode
    ret ; return 
.no_long_mode:
    mov al, "L" ; move "L" to AL register
    jmp error ; jump to error

set_up_page_tables:  ;; this function is to set up the page tables to tell cpu how to map the memory  
    ; 1. Map PML4 entry to PDP
    mov eax, pdp_table ; move pdp_table address to eax register  
    or eax, 0b11 ; set bit 0 and bit 1 to 1 
    mov [pml4_table], eax ; move eax to pml4_table

    ; 2. Map PDP entry to PD
    mov eax, pd_table ;move pd_table address to eax register 
    or eax, 0b11 ; set bit 0 and bit 1 to 1 
    mov [pdp_table], eax ; move eax to pdp_table

    ; 3. Map PD entry to PT
    mov eax, pt_table ;move pt_table address to eax register
    or eax, 0b11 ; set bit 0 and bit 1 to 1 
    mov [pd_table], eax ; move eax to pd_table

    ; 4. Identity map the first 2MB using the PT (512 entries x 4KB = 2MB)
    mov ecx, 0         ; Counter for use in loop example  calculate memory address mov eax , 0x1000(4kb) * ecx (counter) = start address of page
.map_pt_entry:
    mov eax, 0x1000    ; 4KB size per page
    mul ecx            ; eax = 0x1000 * ecx (start address of page) get real Address in memory page 
    or eax, 0b11       ; present + writable
    mov [pt_table + ecx * 8], eax ; Write entry (64-bit entries, so * 8)

    inc ecx ; increment counter
    cmp ecx, 512 ; compare counter with 512
    jne .map_pt_entry ; if not equal jump to .map_pt_entry

    ret

enable_paging:
    ; Load P4 to CR3
    mov eax, pml4_table ; move pml4_table address to eax register 
    mov cr3, eax ; move eax to cr3

    ; Enable PAE-flag in CR4 (Physical Address Extension) in 64bit required enable PAE  and PAE is bit5
    mov eax, cr4 ; move cr4 to eax register
    or eax, 1 << 5 ; set bit 5 to 1
    mov cr4, eax ; move eax to cr4

    ; Set the long mode bit in the EFER MSR (Model Specific Register)
    mov ecx, 0xC0000080 ; EFER MSR is 0xC0000080  EFER is cpu register that control long mode
    rdmsr ; read EFER MSR
    or eax, 1 << 8 ; set bit 8 to 1
    wrmsr ; write EFER MSR

    ; Enable paging in the CR0 register CR0 is cpu register that control paging and bit31 is paging bit
    mov eax, cr0 ; move cr0 to eax register
    or eax, 1 << 31 ; set bit 31 to 1
    mov cr0, eax ; move eax to cr0

    ret

bits 64

long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Fix RDI for kernel argument (if needed)
    ; In previous step we moved ebx to edi in 32-bit.
    ; RDI's lower 32 bits are EDI.
    mov rdi, rdi  ; move rdi to rdi  in 64-bit mode has rule that rdi is the first argument of function 

    call kernel_main ; call kernel_main function 
    hlt ; halt the CPU

section .bss
align 4096 
pml4_table:
    resb 4096   ; reserve 4096 bytes for pml4_table
pdp_table:
    resb 4096   ; reserve 4096 bytes for pdp_table
pd_table:
    resb 4096   ; reserve 4096 bytes for pd_table
pt_table:
    resb 4096
stack_bottom:
    resb 4096 * 4
stack_top:

section .rodata
gdt64:
    dq 0 ; Zero entry
.code_segment: equ $ - gdt64
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; Code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64
