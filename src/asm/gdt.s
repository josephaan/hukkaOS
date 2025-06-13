global gdt_flush       ; allows c code to link to this

gdt_flush:
    extern gdt_pointer
    lgdt [gdt_pointer]      ; Load the new GDT pointer
    mov rax, 0x10        ; Data selector
    mov ss, rax
    mov es, rax
    mov ds, rax
    mov fs, rax
    mov gs, rax
    push 0x08            ; Code selector
    push .farjmp
    o64 retf             ; Perform an indirect 16:64 JMP using retf to jump to .farjmp label
.farjmp:
    ret


