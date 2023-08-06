init_board:                             # @init_board
        xorps   xmm0, xmm0
        movups  xmmword ptr [rdi + 240], xmm0
        movups  xmmword ptr [rdi + 224], xmm0
        movups  xmmword ptr [rdi + 208], xmm0
        movups  xmmword ptr [rdi + 192], xmm0
        movups  xmmword ptr [rdi + 176], xmm0
        movups  xmmword ptr [rdi + 160], xmm0
        movups  xmmword ptr [rdi + 144], xmm0
        movups  xmmword ptr [rdi + 128], xmm0
        movups  xmmword ptr [rdi + 112], xmm0
        movups  xmmword ptr [rdi + 96], xmm0
        movups  xmmword ptr [rdi + 80], xmm0
        movups  xmmword ptr [rdi + 64], xmm0
        movups  xmmword ptr [rdi + 48], xmm0
        movups  xmmword ptr [rdi + 32], xmm0
        movups  xmmword ptr [rdi + 16], xmm0
        movups  xmmword ptr [rdi], xmm0
        ret
print_board:                            # @print_board
        push    rbp
        push    r14
        push    rbx
        mov     rbx, rdi
        mov     esi, dword ptr [rip + count]
        lea     rdi, [rip + .L.str]
        xor     r14d, r14d
        xor     eax, eax
        call    printf@PLT
        mov     ebp, 81
.LBB1_1:                                # =>This Inner Loop Header: Depth=1
        cmp     dword ptr [rbx + 4*r14], 0
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 1
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 2
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 3
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 4
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 5
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 6
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        cmp     dword ptr [rbx + 4*r14], 7
        mov     edi, 42
        cmove   edi, ebp
        call    putchar@PLT
        mov     edi, 10
        call    putchar@PLT
        inc     r14
        cmp     r14, 8
        jne     .LBB1_1
        mov     edi, 10
        pop     rbx
        pop     r14
        pop     rbp
        jmp     putchar@PLT                     # TAILCALL
change_board:                           # @change_board
        push    rbx
        xor     eax, eax
        lea     r8, [rip + .L__const.change_board.delta]
        jmp     .LBB2_1
.LBB2_29:                               #   in Loop: Header=BB2_1 Depth=1
        inc     rax
        cmp     rax, 8
        je      .LBB2_30
.LBB2_1:                                # =>This Inner Loop Header: Depth=1
        mov     r10d, dword ptr [r8 + 8*rax]
        mov     r9d, dword ptr [r8 + 8*rax + 4]
        lea     r11d, [r10 + rsi]
        cmp     r11d, 7
        ja      .LBB2_5
        mov     ebx, r9d
        add     ebx, edx
        js      .LBB2_5
        cmp     ebx, 7
        jg      .LBB2_5
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_5:                                #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [rsi + 2*r10]
        cmp     r11d, 7
        ja      .LBB2_9
        lea     ebx, [rdx + 2*r9]
        test    ebx, ebx
        js      .LBB2_9
        cmp     ebx, 7
        jg      .LBB2_9
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_9:                                #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [r10 + 2*r10]
        add     r11d, esi
        cmp     r11d, 7
        ja      .LBB2_13
        lea     ebx, [r9 + 2*r9]
        add     ebx, edx
        js      .LBB2_13
        cmp     ebx, 7
        jg      .LBB2_13
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_13:                               #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [rsi + 4*r10]
        cmp     r11d, 7
        ja      .LBB2_17
        lea     ebx, [rdx + 4*r9]
        test    ebx, ebx
        js      .LBB2_17
        cmp     ebx, 7
        jg      .LBB2_17
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_17:                               #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [r10 + 4*r10]
        add     r11d, esi
        cmp     r11d, 7
        ja      .LBB2_21
        lea     ebx, [r9 + 4*r9]
        add     ebx, edx
        js      .LBB2_21
        cmp     ebx, 7
        jg      .LBB2_21
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_21:                               #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [r10 + 2*r10]
        lea     r11d, [rsi + 2*r11]
        cmp     r11d, 7
        ja      .LBB2_25
        lea     ebx, [r9 + 2*r9]
        lea     ebx, [rdx + 2*rbx]
        test    ebx, ebx
        js      .LBB2_25
        cmp     ebx, 7
        jg      .LBB2_25
        mov     r11d, r11d
        mov     ebx, ebx
        shl     r11, 5
        add     r11, rdi
        add     dword ptr [r11 + 4*rbx], ecx
.LBB2_25:                               #   in Loop: Header=BB2_1 Depth=1
        lea     r11d, [8*r10]
        sub     r11d, r10d
        add     r11d, esi
        cmp     r11d, 7
        ja      .LBB2_29
        lea     r10d, [8*r9]
        sub     r10d, r9d
        add     r10d, edx
        js      .LBB2_29
        cmp     r10d, 7
        jg      .LBB2_29
        mov     r9d, r11d
        mov     r10d, r10d
        shl     r9, 5
        add     r9, rdi
        add     dword ptr [r9 + 4*r10], ecx
        jmp     .LBB2_29
.LBB2_30:
        pop     rbx
        ret
set_queen:                              # @set_queen
        push    rbp
        push    r15
        push    r14
        push    r13
        push    r12
        push    rbx
        push    rax
        mov     rbx, rdi
        cmp     edx, 8
        jne     .LBB3_1
        inc     dword ptr [rip + count]
        mov     rdi, rbx
        add     rsp, 8
        pop     rbx
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        pop     rbp
        jmp     print_board                     # TAILCALL
.LBB3_1:
        mov     r14d, edx
        mov     r15, rsi
        movsxd  rbp, edx
        lea     eax, [r14 + 1]
        mov     dword ptr [rsp + 4], eax        # 4-byte Spill
        mov     r13, rbp
        shl     r13, 5
        add     r13, rsi
        xor     r12d, r12d
        jmp     .LBB3_2
.LBB3_4:                                #   in Loop: Header=BB3_2 Depth=1
        inc     r12
        cmp     r12, 8
        je      .LBB3_5
.LBB3_2:                                # =>This Inner Loop Header: Depth=1
        cmp     dword ptr [r13 + 4*r12], 0
        jne     .LBB3_4
        mov     dword ptr [rbx + 4*rbp], r12d
        mov     rdi, r15
        mov     esi, r14d
        mov     edx, r12d
        mov     ecx, 1
        call    change_board
        mov     rdi, rbx
        mov     rsi, r15
        mov     edx, dword ptr [rsp + 4]        # 4-byte Reload
        call    set_queen
        mov     rdi, r15
        mov     esi, r14d
        mov     edx, r12d
        mov     ecx, -1
        call    change_board
        jmp     .LBB3_4
.LBB3_5:
        add     rsp, 8
        pop     rbx
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        pop     rbp
        ret
main:                                   # @main
        sub     rsp, 296
        mov     dword ptr [rip + count], 0
        xorps   xmm0, xmm0
        movaps  xmmword ptr [rsp + 240], xmm0
        movaps  xmmword ptr [rsp + 224], xmm0
        movaps  xmmword ptr [rsp + 208], xmm0
        movaps  xmmword ptr [rsp + 192], xmm0
        movaps  xmmword ptr [rsp + 176], xmm0
        movaps  xmmword ptr [rsp + 160], xmm0
        movaps  xmmword ptr [rsp + 144], xmm0
        movaps  xmmword ptr [rsp + 128], xmm0
        movaps  xmmword ptr [rsp + 112], xmm0
        movaps  xmmword ptr [rsp + 96], xmm0
        movaps  xmmword ptr [rsp + 80], xmm0
        movaps  xmmword ptr [rsp + 64], xmm0
        movaps  xmmword ptr [rsp + 48], xmm0
        movaps  xmmword ptr [rsp + 32], xmm0
        movaps  xmmword ptr [rsp + 16], xmm0
        movaps  xmmword ptr [rsp], xmm0
        lea     rdi, [rsp + 256]
        mov     rsi, rsp
        xor     edx, edx
        call    set_queen
        mov     esi, dword ptr [rip + count]
        lea     rdi, [rip + .L.str.4]
        xor     eax, eax
        call    printf@PLT
        xor     eax, eax
        add     rsp, 296
        ret
.L.str:
        .asciz  "No.%d\n"

.L__const.change_board.delta:
        .zero   8,255
        .long   4294967295                      # 0xffffffff
        .long   0                               # 0x0
        .long   4294967295                      # 0xffffffff
        .long   1                               # 0x1
        .long   0                               # 0x0
        .long   4294967295                      # 0xffffffff
        .long   0                               # 0x0
        .long   1                               # 0x1
        .long   1                               # 0x1
        .long   4294967295                      # 0xffffffff
        .long   1                               # 0x1
        .long   0                               # 0x0
        .long   1                               # 0x1
        .long   1                               # 0x1

.L.str.4:
        .asciz  "%d solutions were found.\n"