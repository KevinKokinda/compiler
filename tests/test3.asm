section .text
global main

main:
    push rbp
    mov rbp, rsp
    sub rsp, 32    ; Space for variables x, y, z, result

    ; x = 4
    mov QWORD [rbp-8], 4

    ; y = 2
    mov QWORD [rbp-16], 2

    ; z = x << y
    mov rax, QWORD [rbp-8]   ; Load x into rax
    mov rcx, QWORD [rbp-16]  ; Load y into rcx for shift
    shl rax, cl             ; Shift left
    mov QWORD [rbp-24], rax  ; Store result in z

    ; if (z == 16)
    mov rax, QWORD [rbp-24]
    cmp rax, 16
    jne .else_branch

    ; result = 1
    mov QWORD [rbp-32], 1
    jmp .endif_branch

.else_branch:
    ; result = 0
    mov QWORD [rbp-32], 0

.endif_branch:
    mov rsp, rbp
    pop rbp
    xor eax, eax
    ret