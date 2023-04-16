
//  caller's sp
//  |
//  |
//  V
//  uint32_t context_swtich(uint32_t target_sp)
    .global context_switch
context_switch:
    mov 4(%esp), %eax   // target sp
    mov 8(%esp), %ecx   // target sp
    push %ebx
    push %esi
    push %edi
    push %ebp

    mov %esp, (%ecx)
    mov %eax, %esp

    pop %ebp
    pop %edi
    pop %esi
    pop %ebx
    ret 

    // ^ A
    // v B
