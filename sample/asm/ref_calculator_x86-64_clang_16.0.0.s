main:                                   # @main
        push    rbp
        push    r15
        push    r14
        push    r13
        push    r12
        push    rbx
        push    rax
        mov     byte ptr [rip + input_str], 0
        lea     rbx, [rip + input_str]
        mov     dword ptr [rip + numSize], 0
        mov     dword ptr [rip + opSize], 0
        mov     dword ptr [rip + priorityBase], 0
        mov     dword ptr [rip + error_pos], 0
        lea     rdi, [rip + .Lstr]
        call    puts@PLT
        lea     rdi, [rip + .Lstr.10]
        call    puts@PLT
        lea     rdi, [rip + .L.str.2]
        xor     eax, eax
        call    printf@PLT
        mov     rdi, rbx
        call    receive_input@PLT
        test    rax, rax
        je      .LBB0_14
        lea     r14, [rip + .Lstr]
        lea     r15, [rip + .Lstr.10]
        lea     r12, [rip + .L.str.2]
        lea     rbp, [rip + .L.str.7]
        lea     r13, [rip + .Lstr.11]
        mov     rcx, rbx
        mov     rdx, rbx
        jmp     .LBB0_2
.LBB0_6:                                #   in Loop: Header=BB0_2 Depth=1
        inc     rcx
.LBB0_2:                                # =>This Inner Loop Header: Depth=1
        movzx   eax, byte ptr [rcx]
        test    eax, eax
        je      .LBB0_7
        cmp     eax, 10
        je      .LBB0_6
        cmp     eax, 32
        je      .LBB0_6
        mov     byte ptr [rdx], al
        inc     rdx
        jmp     .LBB0_6
.LBB0_7:                                #   in Loop: Header=BB0_2 Depth=1
        mov     byte ptr [rdx], 0
        cmp     byte ptr [rip + input_str], 0
        je      .LBB0_8
        call    parse_input
        test    eax, eax
        je      .LBB0_10
        call    perform_calculation
        test    eax, eax
        je      .LBB0_10
        mov     esi, dword ptr [rip + nums]
        lea     rdi, [rip + .L.str.4]
        xor     eax, eax
        call    printf@PLT
        jmp     .LBB0_13
.LBB0_10:                               #   in Loop: Header=BB0_2 Depth=1
        mov     rdi, rbx
        call    puts@PLT
        mov     esi, dword ptr [rip + error_pos]
        lea     rdi, [rip + .L.str.6]
        mov     rdx, rbp
        xor     eax, eax
        call    printf@PLT
        mov     edi, 94
        call    putchar@PLT
        mov     rdi, r13
        call    puts@PLT
        jmp     .LBB0_13
.LBB0_8:                                #   in Loop: Header=BB0_2 Depth=1
        mov     edi, 10
        call    putchar@PLT
.LBB0_13:                               #   in Loop: Header=BB0_2 Depth=1
        mov     byte ptr [rip + input_str], 0
        mov     dword ptr [rip + numSize], 0
        mov     dword ptr [rip + opSize], 0
        mov     dword ptr [rip + priorityBase], 0
        mov     dword ptr [rip + error_pos], 0
        mov     rdi, r14
        call    puts@PLT
        mov     rdi, r15
        call    puts@PLT
        mov     rdi, r12
        xor     eax, eax
        call    printf@PLT
        mov     rdi, rbx
        call    receive_input@PLT
        mov     rcx, rbx
        mov     rdx, rbx
        test    rax, rax
        jne     .LBB0_2
.LBB0_14:
        xor     eax, eax
        add     rsp, 8
        pop     rbx
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        pop     rbp
        ret
remove_space:                           # @remove_space
        mov     rax, rdi
        jmp     .LBB1_1
.LBB1_5:                                #   in Loop: Header=BB1_1 Depth=1
        inc     rdi
.LBB1_1:                                # =>This Inner Loop Header: Depth=1
        movzx   ecx, byte ptr [rdi]
        cmp     ecx, 10
        je      .LBB1_5
        cmp     ecx, 32
        je      .LBB1_5
        test    ecx, ecx
        je      .LBB1_6
        mov     byte ptr [rax], cl
        inc     rax
        jmp     .LBB1_5
.LBB1_6:
        mov     byte ptr [rax], 0
        ret
initialize:                             # @initialize
        mov     byte ptr [rip + input_str], 0
        mov     dword ptr [rip + numSize], 0
        mov     dword ptr [rip + opSize], 0
        mov     dword ptr [rip + priorityBase], 0
        mov     dword ptr [rip + error_pos], 0
        ret
parse_input:                            # @parse_input
        push    rbp
        push    r15
        push    r14
        push    r13
        push    r12
        push    rbx
        push    rax
        lea     r14, [rip + input_str]
        mov     rdi, r14
        call    strlen@PLT
        mov     rbx, rax
        mov     r13d, dword ptr [rip + priorityBase]
        test    ebx, ebx
        jle     .LBB3_38
        mov     r15d, dword ptr [rip + opSize]
        mov     r12d, dword ptr [rip + numSize]
        call    __ctype_b_loc@PLT
        lea     edx, [rbx - 1]
        mov     esi, ebx
        xor     ecx, ecx
        lea     rdi, [rip + .LJTI3_0]
        lea     r8, [rip + nums]
        lea     r9, [rip + ops]
        xor     r10d, r10d
        jmp     .LBB3_6
.LBB3_2:                                #   in Loop: Header=BB3_6 Depth=1
        movsxd  r11, r12d
        mov     dword ptr [r8 + 4*r11], r10d
        movsxd  r10, r15d
        lea     r10, [r10 + 2*r10]
        mov     dword ptr [r9 + 4*r10], 0
.LBB3_3:                                #   in Loop: Header=BB3_6 Depth=1
        lea     r11d, [r13 + 1]
.LBB3_4:                                #   in Loop: Header=BB3_6 Depth=1
        mov     dword ptr [r9 + 4*r10 + 4], r11d
        mov     dword ptr [r9 + 4*r10 + 8], ecx
        inc     r12d
        mov     dword ptr [rip + numSize], r12d
        inc     r15d
        mov     dword ptr [rip + opSize], r15d
        xor     r10d, r10d
.LBB3_5:                                #   in Loop: Header=BB3_6 Depth=1
        inc     rcx
        cmp     rsi, rcx
        je      .LBB3_38
.LBB3_6:                                # =>This Inner Loop Header: Depth=1
        movsx   rbp, byte ptr [rcx + r14]
        mov     r11, qword ptr [rax]
        test    byte ptr [r11 + 2*rbp + 1], 8
        jne     .LBB3_13
        movzx   ebp, bpl
        add     ebp, -40
        cmp     ebp, 7
        ja      .LBB3_41
        movsxd  rbp, dword ptr [rdi + 4*rbp]
        add     rbp, rdi
        jmp     rbp
.LBB3_9:                                #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_12
        movsx   rbp, byte ptr [rcx + r14 - 1]
        test    byte ptr [r11 + 2*rbp + 1], 8
        jne     .LBB3_41
        cmp     bpl, 41
        je      .LBB3_41
.LBB3_12:                               #   in Loop: Header=BB3_6 Depth=1
        add     r13d, 10
        mov     dword ptr [rip + priorityBase], r13d
        jmp     .LBB3_5
.LBB3_13:                               #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_15
        cmp     byte ptr [rcx + r14 - 1], 41
        je      .LBB3_41
.LBB3_15:                               #   in Loop: Header=BB3_6 Depth=1
        lea     r10d, [r10 + 4*r10]
        lea     r10d, [rbp + 2*r10 - 48]
        cmp     rdx, rcx
        jne     .LBB3_5
        movsxd  r12, r12d
        mov     dword ptr [r8 + 4*r12], r10d
        inc     r12d
        mov     dword ptr [rip + numSize], r12d
        jmp     .LBB3_5
.LBB3_17:                               #   in Loop: Header=BB3_6 Depth=1
        lea     ebp, [r13 - 10]
        mov     dword ptr [rip + priorityBase], ebp
        cmp     r13d, 9
        jle     .LBB3_41
        movsx   r13, byte ptr [rcx + r14 - 1]
        test    byte ptr [r11 + 2*r13 + 1], 8
        jne     .LBB3_20
        cmp     r13b, 41
        jne     .LBB3_41
.LBB3_20:                               #   in Loop: Header=BB3_6 Depth=1
        cmp     rdx, rcx
        jne     .LBB3_22
        movsxd  r11, r12d
        mov     dword ptr [r8 + 4*r11], r10d
        inc     r12d
        mov     dword ptr [rip + numSize], r12d
.LBB3_22:                               #   in Loop: Header=BB3_6 Depth=1
        mov     r13d, ebp
        jmp     .LBB3_5
.LBB3_23:                               #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_44
        movsx   rbp, byte ptr [rcx + r14 - 1]
        test    byte ptr [r11 + 2*rbp + 1], 8
        jne     .LBB3_26
        cmp     bpl, 41
        jne     .LBB3_41
.LBB3_26:                               #   in Loop: Header=BB3_6 Depth=1
        movsxd  r11, r12d
        mov     dword ptr [r8 + 4*r11], r10d
        movsxd  r15, r15d
        lea     r10, [r15 + 2*r15]
        mov     dword ptr [r9 + 4*r10], 3
        lea     r11d, [r13 + 2]
        jmp     .LBB3_4
.LBB3_27:                               #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_2
        movsx   rbp, byte ptr [rcx + r14 - 1]
        movzx   r11d, word ptr [r11 + 2*rbp]
        test    r11d, 2048
        jne     .LBB3_2
        and     bpl, -2
        cmp     bpl, 40
        je      .LBB3_2
        jmp     .LBB3_41
.LBB3_30:                               #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_33
        movsx   rbp, byte ptr [rcx + r14 - 1]
        movzx   r11d, word ptr [r11 + 2*rbp]
        test    r11d, 2048
        jne     .LBB3_33
        and     bpl, -2
        cmp     bpl, 40
        jne     .LBB3_41
.LBB3_33:                               #   in Loop: Header=BB3_6 Depth=1
        movsxd  r11, r12d
        mov     dword ptr [r8 + 4*r11], r10d
        movsxd  r10, r15d
        lea     r10, [r10 + 2*r10]
        mov     dword ptr [r9 + 4*r10], 1
        jmp     .LBB3_3
.LBB3_34:                               #   in Loop: Header=BB3_6 Depth=1
        test    rcx, rcx
        je      .LBB3_44
        movsx   rbp, byte ptr [rcx + r14 - 1]
        test    byte ptr [r11 + 2*rbp + 1], 8
        jne     .LBB3_37
        cmp     bpl, 41
        jne     .LBB3_41
.LBB3_37:                               #   in Loop: Header=BB3_6 Depth=1
        movsxd  r11, r12d
        mov     dword ptr [r8 + 4*r11], r10d
        movsxd  r10, r15d
        lea     r10, [r10 + 2*r10]
        mov     dword ptr [r9 + 4*r10], 2
        lea     r11d, [r13 + 2]
        jmp     .LBB3_4
.LBB3_38:
        test    r13d, r13d
        jne     .LBB3_40
        mov     ecx, dword ptr [rip + opSize]
        inc     ecx
        mov     eax, 1
        cmp     dword ptr [rip + numSize], ecx
        je      .LBB3_42
.LBB3_40:
        lea     ecx, [rbx - 1]
.LBB3_41:
        mov     dword ptr [rip + error_pos], ecx
        xor     eax, eax
.LBB3_42:
        add     rsp, 8
        pop     rbx
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        pop     rbp
        ret
.LBB3_44:
        xor     ecx, ecx
        jmp     .LBB3_41
.LJTI3_0:
        .long   .LBB3_9-.LJTI3_0
        .long   .LBB3_17-.LJTI3_0
        .long   .LBB3_23-.LJTI3_0
        .long   .LBB3_27-.LJTI3_0
        .long   .LBB3_41-.LJTI3_0
        .long   .LBB3_30-.LJTI3_0
        .long   .LBB3_41-.LJTI3_0
        .long   .LBB3_34-.LJTI3_0
setOP:                                  # @setOP
        movsxd  rax, dword ptr [rip + numSize]
        lea     rcx, [rip + nums]
        mov     dword ptr [rcx + 4*rax], esi
        movsxd  rcx, dword ptr [rip + opSize]
        lea     rsi, [rcx + 2*rcx]
        lea     r8, [rip + ops]
        cmp     edi, 2
        mov     r9d, dword ptr [rip + priorityBase]
        sbb     r9d, 0
        mov     dword ptr [r8 + 4*rsi], edi
        add     r9d, 2
        mov     dword ptr [r8 + 4*rsi + 4], r9d
        mov     dword ptr [r8 + 4*rsi + 8], edx
        inc     eax
        mov     dword ptr [rip + numSize], eax
        lea     eax, [rcx + 1]
        mov     dword ptr [rip + opSize], eax
        ret
find_highest_priority_op:               # @find_highest_priority_op
        push    rbx
        mov     ecx, dword ptr [rip + opSize]
        xor     eax, eax
        cmp     ecx, 2
        jl      .LBB5_5
        mov     eax, ecx
        dec     rcx
        cmp     eax, 2
        jne     .LBB5_6
        xor     eax, eax
        mov     edx, 1
.LBB5_3:
        test    cl, 1
        je      .LBB5_5
        lea     rcx, [rdx + 2*rdx]
        lea     rsi, [rip + ops]
        mov     ecx, dword ptr [rsi + 4*rcx + 4]
        cdqe
        lea     rdi, [rax + 2*rax]
        cmp     ecx, dword ptr [rsi + 4*rdi + 4]
        cmovg   eax, edx
.LBB5_5:
        pop     rbx
        ret
.LBB5_6:
        mov     rsi, rcx
        and     rsi, -2
        xor     eax, eax
        mov     edx, 1
        lea     rdi, [rip + ops+28]
        lea     r8, [rip + ops]
        jmp     .LBB5_7
.LBB5_11:                               #   in Loop: Header=BB5_7 Depth=1
        add     rdx, 2
        add     rdi, 24
        cmp     r10, rsi
        je      .LBB5_3
.LBB5_7:                                # =>This Inner Loop Header: Depth=1
        movsxd  r9, eax
        lea     r10, [r9 + 2*r9]
        mov     r11d, dword ptr [rdi - 12]
        mov     r9d, edx
        cmp     r11d, dword ptr [r8 + 4*r10 + 4]
        jg      .LBB5_9
        mov     r9d, eax
.LBB5_9:                                #   in Loop: Header=BB5_7 Depth=1
        mov     r11d, dword ptr [rdi]
        movsxd  rax, r9d
        lea     rbx, [rax + 2*rax]
        lea     r10, [rdx + 1]
        mov     eax, r10d
        cmp     r11d, dword ptr [r8 + 4*rbx + 4]
        jg      .LBB5_11
        mov     eax, r9d
        jmp     .LBB5_11
perform_calculation:                    # @perform_calculation
        push    r15
        push    r14
        push    r13
        push    r12
        push    rbx
        mov     ebx, 1
        cmp     dword ptr [rip + numSize], 2
        jl      .LBB6_27
        lea     r15, [rip + ops]
        lea     r12, [rip + ops+28]
        lea     r13, [rip + nums]
        jmp     .LBB6_2
.LBB6_26:                               #   in Loop: Header=BB6_2 Depth=1
        mov     dword ptr [rip + opSize], ecx
        cmp     eax, 2
        jle     .LBB6_27
.LBB6_2:                                # =>This Loop Header: Depth=1
        mov     eax, dword ptr [rip + opSize]
        mov     r14d, 0
        cmp     eax, 2
        jl      .LBB6_13
        mov     ecx, eax
        dec     rax
        cmp     ecx, 2
        jne     .LBB6_5
        xor     r14d, r14d
        mov     ecx, 1
.LBB6_11:                               #   in Loop: Header=BB6_2 Depth=1
        test    al, 1
        je      .LBB6_13
        lea     rax, [rcx + 2*rcx]
        mov     eax, dword ptr [r15 + 4*rax + 4]
        movsxd  r14, r14d
        lea     rdx, [r14 + 2*r14]
        cmp     eax, dword ptr [r15 + 4*rdx + 4]
        cmovg   r14d, ecx
.LBB6_13:                               #   in Loop: Header=BB6_2 Depth=1
        mov     edi, r14d
        call    calc
        test    eax, eax
        je      .LBB6_14
        lea     edx, [r14 + 1]
        movsxd  rax, dword ptr [rip + numSize]
        lea     ecx, [rax - 1]
        cmp     edx, ecx
        jge     .LBB6_23
        movsxd  r8, edx
        mov     esi, eax
        sub     esi, r14d
        add     esi, -3
        cmp     esi, 7
        jae     .LBB6_18
        mov     rdx, r8
        jmp     .LBB6_21
.LBB6_5:                                #   in Loop: Header=BB6_2 Depth=1
        mov     rdx, rax
        and     rdx, -2
        xor     r14d, r14d
        mov     ecx, 1
        mov     rsi, r12
        jmp     .LBB6_6
.LBB6_10:                               #   in Loop: Header=BB6_6 Depth=2
        add     rcx, 2
        add     rsi, 24
        cmp     r8, rdx
        je      .LBB6_11
.LBB6_6:                                #   Parent Loop BB6_2 Depth=1
        movsxd  rdi, r14d
        lea     r8, [rdi + 2*rdi]
        mov     r9d, dword ptr [rsi - 12]
        mov     edi, ecx
        cmp     r9d, dword ptr [r15 + 4*r8 + 4]
        jg      .LBB6_8
        mov     edi, r14d
.LBB6_8:                                #   in Loop: Header=BB6_6 Depth=2
        mov     r9d, dword ptr [rsi]
        movsxd  r8, edi
        lea     r10, [r8 + 2*r8]
        lea     r8, [rcx + 1]
        mov     r14d, r8d
        cmp     r9d, dword ptr [r15 + 4*r10 + 4]
        jg      .LBB6_10
        mov     r14d, edi
        jmp     .LBB6_10
.LBB6_18:                               #   in Loop: Header=BB6_2 Depth=1
        inc     rsi
        mov     rdi, rsi
        and     rdi, -8
        lea     rdx, [rdi + r8]
        lea     r8, [4*r8 + 20]
        add     r8, r13
        xor     r9d, r9d
.LBB6_19:                               #   Parent Loop BB6_2 Depth=1
        movups  xmm0, xmmword ptr [r8 + 4*r9 - 16]
        movups  xmm1, xmmword ptr [r8 + 4*r9]
        movups  xmmword ptr [r8 + 4*r9 - 20], xmm0
        movups  xmmword ptr [r8 + 4*r9 - 4], xmm1
        add     r9, 8
        cmp     rdi, r9
        jne     .LBB6_19
        cmp     rsi, rdi
        je      .LBB6_23
.LBB6_21:                               #   in Loop: Header=BB6_2 Depth=1
        lea     rsi, [rax - 1]
.LBB6_22:                               #   Parent Loop BB6_2 Depth=1
        mov     edi, dword ptr [r13 + 4*rdx + 4]
        mov     dword ptr [r13 + 4*rdx], edi
        lea     rdi, [rdx + 1]
        mov     rdx, rdi
        cmp     esi, edi
        jne     .LBB6_22
.LBB6_23:                               #   in Loop: Header=BB6_2 Depth=1
        mov     dword ptr [rip + numSize], ecx
        movsxd  rdi, dword ptr [rip + opSize]
        lea     ecx, [rdi - 1]
        cmp     r14d, ecx
        jge     .LBB6_26
        movsxd  rdx, r14d
        lea     rsi, [rdx + 2*rdx]
        lea     rsi, [r15 + 4*rsi]
        not     rdx
        add     rdx, rdi
.LBB6_25:                               #   Parent Loop BB6_2 Depth=1
        mov     edi, dword ptr [rsi + 20]
        mov     dword ptr [rsi + 8], edi
        mov     rdi, qword ptr [rsi + 12]
        mov     qword ptr [rsi], rdi
        add     rsi, 12
        dec     rdx
        jne     .LBB6_25
        jmp     .LBB6_26
.LBB6_14:
        xor     ebx, ebx
.LBB6_27:
        mov     eax, ebx
        pop     rbx
        pop     r12
        pop     r13
        pop     r14
        pop     r15
        ret
calc:                                   # @calc
        push    rbp
        push    rbx
        movsxd  rdi, edi
        lea     r9, [rdi + 2*rdi]
        lea     r10, [rip + ops]
        mov     eax, dword ptr [r10 + 4*r9]
        xor     ecx, ecx
        cmp     rax, 3
        ja      .LBB7_28
        lea     r8, [rip + nums]
        mov     esi, dword ptr [r8 + 4*rdi]
        mov     r11d, dword ptr [r8 + 4*rdi + 4]
        lea     rdx, [rip + .LJTI7_0]
        movsxd  rax, dword ptr [rdx + 4*rax]
        add     rax, rdx
        jmp     rax
.LBB7_2:
        mov     eax, esi
        xor     eax, 2147483647
        xor     edx, edx
        cmp     eax, r11d
        setl    dl
        mov     eax, -2147483648
        sub     eax, esi
        xor     ebx, ebx
        cmp     eax, r11d
        setg    bl
        test    esi, esi
        cmovs   edx, ebx
        test    dl, dl
        jne     .LBB7_23
        add     r11d, esi
        jmp     .LBB7_26
.LBB7_4:
        cmp     r11d, -2147483648
        jne     .LBB7_13
        mov     eax, esi
        shr     eax, 31
        test    eax, eax
        jne     .LBB7_14
        jmp     .LBB7_23
.LBB7_6:
        test    r11d, r11d
        je      .LBB7_23
        mov     eax, esi
        cdq
        idiv    r11d
        mov     dword ptr [r8 + 4*rdi], eax
        jmp     .LBB7_27
.LBB7_8:
        test    esi, esi
        je      .LBB7_25
        test    r11d, r11d
        je      .LBB7_25
        test    esi, esi
        jle     .LBB7_15
        test    r11d, r11d
        jle     .LBB7_15
        mov     eax, 2147483647
        xor     edx, edx
        div     esi
        cmp     eax, r11d
        jb      .LBB7_23
        jmp     .LBB7_25
.LBB7_13:
        mov     eax, r11d
        neg     eax
        mov     edx, esi
        xor     edx, 2147483647
        xor     ebx, ebx
        cmp     edx, eax
        setge   bl
        mov     edx, -2147483648
        sub     edx, esi
        xor     ebp, ebp
        cmp     edx, eax
        setle   bpl
        test    esi, esi
        cmovs   ebx, ebp
        movzx   eax, bl
        test    eax, eax
        je      .LBB7_23
.LBB7_14:
        sub     esi, r11d
        mov     dword ptr [r8 + 4*rdi], esi
        jmp     .LBB7_27
.LBB7_15:
        test    r11d, esi
        js      .LBB7_18
        cmp     esi, -2147483648
        jne     .LBB7_21
        cmp     r11d, 1
        jne     .LBB7_23
        jmp     .LBB7_25
.LBB7_18:
        cmp     esi, -2147483648
        je      .LBB7_23
        cmp     r11d, -2147483648
        je      .LBB7_23
        mov     ebx, r11d
        neg     ebx
        mov     ebp, esi
        neg     ebp
        mov     eax, 2147483647
        xor     edx, edx
        div     ebp
        cmp     eax, ebx
        jl      .LBB7_23
        jmp     .LBB7_25
.LBB7_21:
        cmp     r11d, -2147483648
        jne     .LBB7_24
        cmp     esi, 1
        je      .LBB7_25
.LBB7_23:
        mov     eax, dword ptr [r10 + 4*r9 + 8]
        mov     dword ptr [rip + error_pos], eax
        jmp     .LBB7_28
.LBB7_24:
        mov     ebx, esi
        neg     ebx
        cmovs   ebx, esi
        mov     ebp, r11d
        neg     ebp
        cmovs   ebp, r11d
        mov     eax, 2147483647
        xor     edx, edx
        div     ebx
        cmp     ebp, eax
        ja      .LBB7_23
.LBB7_25:
        imul    r11d, esi
.LBB7_26:
        mov     dword ptr [r8 + 4*rdi], r11d
.LBB7_27:
        mov     ecx, 1
.LBB7_28:
        mov     eax, ecx
        pop     rbx
        pop     rbp
        ret
.LJTI7_0:
        .long   .LBB7_2-.LJTI7_0
        .long   .LBB7_4-.LJTI7_0
        .long   .LBB7_6-.LJTI7_0
        .long   .LBB7_8-.LJTI7_0
check_add:                              # @check_add
        mov     eax, edi
        xor     eax, 2147483647
        xor     ecx, ecx
        cmp     eax, esi
        setge   cl
        mov     eax, -2147483648
        sub     eax, edi
        xor     edx, edx
        cmp     eax, esi
        setle   dl
        test    edi, edi
        cmovs   ecx, edx
        movzx   eax, cl
        ret
check_sub:                              # @check_sub
        mov     eax, edi
        cmp     esi, -2147483648
        jne     .LBB9_2
        shr     eax, 31
        ret
.LBB9_2:
        neg     esi
        mov     ecx, eax
        xor     ecx, 2147483647
        xor     edx, edx
        cmp     ecx, esi
        setge   dl
        mov     ecx, -2147483648
        sub     ecx, eax
        xor     edi, edi
        cmp     ecx, esi
        setle   dil
        test    eax, eax
        cmovs   edx, edi
        movzx   eax, dl
        ret
check_div:                              # @check_div
        xor     eax, eax
        test    esi, esi
        setne   al
        ret
check_mul:                              # @check_mul
        mov     al, 1
        test    edi, edi
        je      .LBB11_15
        test    esi, esi
        je      .LBB11_15
        test    edi, edi
        jle     .LBB11_5
        test    esi, esi
        jle     .LBB11_5
        mov     eax, 2147483647
        xor     edx, edx
        div     edi
        cmp     eax, esi
        setae   al
        movzx   eax, al
        ret
.LBB11_5:
        test    esi, edi
        js      .LBB11_9
        cmp     edi, -2147483648
        jne     .LBB11_12
        cmp     esi, 1
        jmp     .LBB11_8
.LBB11_9:
        xor     eax, eax
        cmp     edi, -2147483648
        je      .LBB11_15
        cmp     esi, -2147483648
        je      .LBB11_15
        neg     esi
        neg     edi
        mov     eax, 2147483647
        xor     edx, edx
        div     edi
        cmp     eax, esi
        setge   al
        movzx   eax, al
        ret
.LBB11_12:
        cmp     esi, -2147483648
        jne     .LBB11_14
        cmp     edi, 1
.LBB11_8:
        sete    al
        movzx   eax, al
        ret
.LBB11_14:
        mov     ecx, edi
        neg     ecx
        cmovs   ecx, edi
        mov     edi, esi
        neg     edi
        cmovs   edi, esi
        mov     eax, 2147483647
        xor     edx, edx
        div     ecx
        cmp     edi, eax
        setbe   al
.LBB11_15:
        movzx   eax, al
        ret
report_error:                           # @report_error
        push    rax
        lea     rdi, [rip + input_str]
        call    puts@PLT
        mov     esi, dword ptr [rip + error_pos]
        lea     rdi, [rip + .L.str.6]
        lea     rdx, [rip + .L.str.7]
        xor     eax, eax
        call    printf@PLT
        mov     edi, 94
        call    putchar@PLT
        lea     rdi, [rip + .Lstr.11]
        pop     rax
        jmp     puts@PLT                        # TAILCALL
.L.str.2:
        .asciz  "Input: "

input_str:
        .zero   128

.L.str.4:
        .asciz  "Result: %d\n\n"

nums:
        .zero   512

numSize:
        .long   0                               # 0x0

opSize:
        .long   0                               # 0x0

priorityBase:
        .long   0                               # 0x0

error_pos:
        .long   0                               # 0x0

ops:
        .zero   1536

.L.str.6:
        .asciz  "%*s"

.L.str.7:
        .zero   1

.Lstr:
        .asciz  "Enter the formula to be calculated."

.Lstr.10:
        .asciz  "Press Ctrl+C to exit."

.Lstr.11:
        .asciz  " Invalid expression. Please try again.\n"