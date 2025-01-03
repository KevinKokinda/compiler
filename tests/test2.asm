section .text
global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 40    ; Space for variables a, b, max, count, sum

    ; a = 10
    mov QWORD [rbp-8], 10

    ; b = 20
    mov QWORD [rbp-16], 20

    ; max = 0
    mov QWORD [rbp-24], 0

    ; if (a > b)
    mov rax, QWORD [rbp-8]
    cmp rax, QWORD [rbp-16]
    jle .else_branch

    ; max = a
    mov rax, QWORD [rbp-8]
    mov QWORD [rbp-24], rax
    jmp .endif_branch

.else_branch:
    ; max = b
    mov rax, QWORD [rbp-16]
    mov QWORD [rbp-24], rax

.endif_branch:
    ; count = max
    mov rax, QWORD [rbp-24]
    mov QWORD [rbp-32], rax

    ; sum = 0
    mov QWORD [rbp-40], 0

.while_start:
    ; while (count > 0)
    mov rax, QWORD [rbp-32]
    cmp rax, 0
    jle .while_end

    ; sum = sum + count
    mov rax, QWORD [rbp-40]
    add rax, QWORD [rbp-32]
    mov QWORD [rbp-40], rax

    ; count = count - 1
    mov rax, QWORD [rbp-32]
    sub rax, 1
    mov QWORD [rbp-32], rax

    jmp .while_start

.while_end:
    mov rsp, rbp
    pop rbp
    xor eax, eax
    ret