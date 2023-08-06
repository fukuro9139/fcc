init_board:
        mov     QWORD PTR [rdi], 0
        mov     rax, rdi
        lea     rdi, [rdi+8]
        mov     QWORD PTR [rdi+240], 0
        and     rdi, -8
        sub     rax, rdi
        lea     ecx, [rax+256]
        xor     eax, eax
        shr     ecx, 3
        rep stosq
        ret
.LC0:
        .string "No.%d\n"
print_board:
        push    r12
        mov     esi, DWORD PTR count[rip]
        xor     eax, eax
        push    rbp
        mov     rbp, rdi
        mov     edi, OFFSET FLAT:.LC0
        push    rbx
        lea     r12, [rbp+32]
        call    printf
.L4:
        xor     ebx, ebx
        jmp     .L7
.L5:
        mov     edi, 42
        add     ebx, 1
        call    putchar
        cmp     ebx, 8
        je      .L11
.L7:
        cmp     DWORD PTR [rbp+0], ebx
        jne     .L5
        mov     edi, 81
        add     ebx, 1
        call    putchar
        cmp     ebx, 8
        jne     .L7
.L11:
        mov     edi, 10
        add     rbp, 4
        call    putchar
        cmp     rbp, r12
        jne     .L4
        pop     rbx
        mov     edi, 10
        pop     rbp
        pop     r12
        jmp     putchar
change_board:
        push    r15
        movdqa  xmm0, XMMWORD PTR .LC1[rip]
        push    r14
        movsx   r14, esi
        push    r13
        mov     r13d, edx
        push    r12
        mov     r12, rdi
        push    rbp
        mov     rbp, r14
        push    rbx
        mov     ebx, ecx
        movaps  XMMWORD PTR [rsp-72], xmm0
        lea     r11, [rsp-72]
        lea     r15, [rsp-8]
        movdqa  xmm0, XMMWORD PTR .LC2[rip]
        movaps  XMMWORD PTR [rsp-56], xmm0
        movdqa  xmm0, XMMWORD PTR .LC3[rip]
        movaps  XMMWORD PTR [rsp-40], xmm0
        movdqa  xmm0, XMMWORD PTR .LC4[rip]
        movaps  XMMWORD PTR [rsp-24], xmm0
.L13:
        movsx   rsi, DWORD PTR [r11+4]
        movsx   rdx, DWORD PTR [r11]
        lea     eax, [rsi+r13]
        lea     r8, [rsi+rdx*8]
        mov     r10, rdx
        mov     r9, rsi
        lea     ecx, [rdx+rbp]
        movsx   rsi, eax
        add     rdx, r14
        sal     r8, 2
        lea     rdx, [rsi+rdx*8]
        lea     rsi, [r12+rdx*4]
        mov     edx, 7
.L15:
        mov     edi, eax
        or      edi, ecx
        cmp     edi, 7
        ja      .L14
        add     DWORD PTR [rsi], ebx
.L14:
        add     ecx, r10d
        add     eax, r9d
        add     rsi, r8
        sub     edx, 1
        jne     .L15
        add     r11, 8
        cmp     r11, r15
        jne     .L13
        pop     rbx
        pop     rbp
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        ret
set_queen.part.0:
        push    r15
        movsx   rax, edx
        xor     r15d, r15d
        push    r14
        push    r13
        push    r12
        mov     r12, rsi
        push    rbp
        mov     rbp, rax
        push    rbx
        mov     rbx, rax
        lea     rax, [rdi+rax*4]
        lea     r14d, [rbp+1]
        sal     rbx, 5
        add     rbx, rsi
        sub     rsp, 24
        mov     QWORD PTR [rsp+8], rdi
        mov     QWORD PTR [rsp], rax
        jmp     .L23
.L20:
        add     r15, 1
        cmp     r15, 8
        je      .L26
.L23:
        mov     eax, DWORD PTR [rbx+r15*4]
        test    eax, eax
        jne     .L20
        mov     rax, QWORD PTR [rsp]
        mov     ecx, 1
        mov     edx, r15d
        mov     esi, ebp
        mov     rdi, r12
        mov     r13d, r15d
        mov     DWORD PTR [rax], r15d
        call    change_board
        cmp     r14d, 8
        je      .L27
        mov     rdi, QWORD PTR [rsp+8]
        mov     edx, r14d
        mov     rsi, r12
        call    set_queen.part.0
.L22:
        mov     ecx, -1
        mov     edx, r13d
        mov     esi, ebp
        mov     rdi, r12
        call    change_board
        add     r15, 1
        cmp     r15, 8
        jne     .L23
.L26:
        add     rsp, 24
        pop     rbx
        pop     rbp
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        ret
.L27:
        mov     rdi, QWORD PTR [rsp+8]
        add     DWORD PTR count[rip], 1
        call    print_board
        jmp     .L22
set_queen:
        cmp     edx, 8
        je      .L30
        jmp     set_queen.part.0
.L30:
        add     DWORD PTR count[rip], 1
        jmp     print_board
.LC5:
        .string "%d solutions were found.\n"
main:
        sub     rsp, 296
        xor     edx, edx
        mov     DWORD PTR count[rip], 0
        lea     rdi, [rsp+32]
        lea     rsi, [rsp+32]
        call    init_board
        mov     rdi, rsp
        call    set_queen.part.0
        mov     esi, DWORD PTR count[rip]
        mov     edi, OFFSET FLAT:.LC5
        xor     eax, eax
        call    printf
        xor     eax, eax
        add     rsp, 296
        ret
.LC1:
        .long   -1
        .long   -1
        .long   -1
        .long   0
.LC2:
        .long   -1
        .long   1
        .long   0
        .long   -1
.LC3:
        .long   0
        .long   1
        .long   1
        .long   -1
.LC4:
        .long   1
        .long   0
        .long   1
        .long   1