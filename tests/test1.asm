section .text
global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32    ; Space for variables x, y, z, result

    ; x = 5
    mov QWORD [rbp-8], 5

    ; y = 3
    mov QWORD [rbp-16], 3

    ; z = x + y * 2
    mov rax, QWORD [rbp-16]  ; Load y
    imul rax, 2              ; y * 2
    add rax, QWORD [rbp-8]   ; Add x
    mov QWORD [rbp-24], rax  ; Store in z

    ; if (z > 10)
    mov rax, QWORD [rbp-24]
    cmp rax, 10
    jle .else_branch

    ; result = z - 5
    mov rax, QWORD [rbp-24]
    sub rax, 5
    mov QWORD [rbp-32], rax
    jmp .endif_branch

.else_branch:
    ; result = z + 5
    mov rax, QWORD [rbp-24]
    add rax, 5
    mov QWORD [rbp-32], rax

.endif_branch:

.while_start:
    ; while (result > 0)
    mov rax, QWORD [rbp-32]
    cmp rax, 0
    jle .while_end

    ; result = result - 1
    mov rax, QWORD [rbp-32]
    sub rax, 1
    mov QWORD [rbp-32], rax
    
    jmp .while_start

.while_end:
    mov rsp, rbp
    pop rbp
    xor eax, eax
    ret