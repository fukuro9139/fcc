initialize:
        mov     BYTE PTR input_str[rip], 0
        mov     DWORD PTR numSize[rip], 0
        mov     DWORD PTR opSize[rip], 0
        mov     DWORD PTR priorityBase[rip], 0
        mov     DWORD PTR error_pos[rip], 0
        ret
remove_space:
        movzx   eax, BYTE PTR [rdi]
        test    al, al
        je      .L4
        mov     rdx, rdi
.L6:
        cmp     al, 32
        je      .L5
        cmp     al, 10
        je      .L5
        mov     BYTE PTR [rdi], al
        add     rdi, 1
.L5:
        movzx   eax, BYTE PTR [rdx+1]
        add     rdx, 1
        test    al, al
        jne     .L6
.L4:
        mov     BYTE PTR [rdi], 0
        ret
setOP:
        movsx   rax, DWORD PTR numSize[rip]
        mov     ecx, esi
        mov     r8d, edx
        xor     edx, edx
        mov     DWORD PTR nums[0+rax*4], ecx
        mov     rsi, rax
        movsx   rax, DWORD PTR opSize[rip]
        mov     rcx, rax
        lea     rax, [rax+rax*2]
        sal     rax, 2
        cmp     edi, 1
        seta    dl
        add     edx, DWORD PTR priorityBase[rip]
        add     esi, 1
        add     ecx, 1
        add     edx, 1
        mov     DWORD PTR ops[rax], edi
        mov     DWORD PTR ops[rax+4], edx
        mov     DWORD PTR ops[rax+8], r8d
        mov     DWORD PTR numSize[rip], esi
        mov     DWORD PTR opSize[rip], ecx
        ret
parse_input:
        push    rbp
        mov     edi, OFFSET FLAT:input_str
        push    rbx
        sub     rsp, 8
        call    strlen
        mov     rbp, rax
        test    eax, eax
        je      .L50
        mov     ebx, eax
        call    __ctype_b_loc
        xor     r9d, r9d
        xor     esi, esi
        mov     r10, QWORD PTR [rax]
.L49:
        movsx   rdx, BYTE PTR input_str[r9]
        mov     ecx, r9d
        mov     rax, rdx
        test    BYTE PTR [r10+1+rdx*2], 8
        je      .L24
        mov     edx, r9d
        test    r9, r9
        je      .L25
        cmp     BYTE PTR input_str[r9-1], 41
        je      .L46
.L25:
        lea     ecx, [rsi+rsi*4]
        lea     esi, [rax-48+rcx*2]
        lea     eax, [rbx-1]
        cmp     eax, edx
        je      .L88
.L28:
        add     r9, 1
        cmp     rbp, r9
        jne     .L49
.L50:
        mov     eax, DWORD PTR priorityBase[rip]
        test    eax, eax
        jne     .L86
        mov     eax, DWORD PTR opSize[rip]
        lea     edx, [rax+1]
        cmp     edx, DWORD PTR numSize[rip]
        mov     eax, 1
        je      .L20
.L86:
        lea     ecx, [rbp-1]
        jmp     .L46
.L24:
        sub     eax, 40
        cmp     al, 7
        ja      .L46
        movzx   eax, al
        jmp     [QWORD PTR .L30[0+rax*8]]
.L30:
        .quad   .L35
        .quad   .L34
        .quad   .L33
        .quad   .L32
        .quad   .L46
        .quad   .L31
        .quad   .L46
        .quad   .L29
.L40:
        lea     eax, [rbx-1]
        cmp     eax, r9d
        jne     .L28
.L88:
        movsx   rdx, DWORD PTR numSize[rip]
        mov     rax, rdx
        mov     DWORD PTR nums[0+rdx*4], esi
        add     eax, 1
        mov     DWORD PTR numSize[rip], eax
        jmp     .L28
.L35:
        test    r9, r9
        je      .L36
        movsx   rdx, BYTE PTR input_str[r9-1]
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L46
        cmp     dl, 41
        je      .L46
.L36:
        add     DWORD PTR priorityBase[rip], 10
        jmp     .L28
.L34:
        sub     DWORD PTR priorityBase[rip], 10
        js      .L46
        movsx   rdx, BYTE PTR input_str[r9-1]
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L40
        cmp     dl, 41
        je      .L40
.L46:
        mov     DWORD PTR error_pos[rip], ecx
        xor     eax, eax
.L20:
        add     rsp, 8
        pop     rbx
        pop     rbp
        ret
.L33:
        test    r9, r9
        jne     .L89
.L52:
        xor     ecx, ecx
        jmp     .L46
.L32:
        test    r9, r9
        je      .L41
        movsx   rdx, BYTE PTR input_str[r9-1]
        mov     rax, rdx
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L41
        sub     eax, 40
        cmp     al, 1
        ja      .L46
.L41:
        movsx   rax, DWORD PTR numSize[rip]
        mov     DWORD PTR nums[0+rax*4], esi
        movsx   rsi, DWORD PTR opSize[rip]
        mov     rdx, rax
        mov     rax, rsi
        lea     rsi, [rsi+rsi*2]
        mov     DWORD PTR ops[0+rsi*4], 0
        lea     rdi, ops[0+rsi*4]
.L87:
        mov     esi, DWORD PTR priorityBase[rip]
        add     edx, 1
        add     eax, 1
        mov     DWORD PTR [rdi+8], ecx
        add     esi, 1
        mov     DWORD PTR [rdi+4], esi
        xor     esi, esi
        mov     DWORD PTR numSize[rip], edx
        mov     DWORD PTR opSize[rip], eax
        jmp     .L28
.L31:
        test    r9, r9
        je      .L44
        movsx   rdx, BYTE PTR input_str[r9-1]
        mov     rax, rdx
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L44
        sub     eax, 40
        cmp     al, 1
        ja      .L46
.L44:
        movsx   rax, DWORD PTR numSize[rip]
        mov     DWORD PTR nums[0+rax*4], esi
        movsx   rsi, DWORD PTR opSize[rip]
        mov     rdx, rax
        mov     rax, rsi
        lea     rsi, [rsi+rsi*2]
        mov     DWORD PTR ops[0+rsi*4], 1
        lea     rdi, ops[0+rsi*4]
        jmp     .L87
.L29:
        test    r9, r9
        je      .L52
        movsx   rdx, BYTE PTR input_str[r9-1]
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L54
        cmp     dl, 41
        jne     .L46
.L54:
        mov     edx, ecx
        mov     edi, 2
        call    setOP
        xor     esi, esi
        jmp     .L28
.L89:
        movsx   rdx, BYTE PTR input_str[r9-1]
        test    BYTE PTR [r10+1+rdx*2], 8
        jne     .L55
        cmp     dl, 41
        jne     .L46
.L55:
        mov     edx, ecx
        mov     edi, 3
        call    setOP
        xor     esi, esi
        jmp     .L28
find_highest_priority_op:
        mov     edi, DWORD PTR opSize[rip]
        cmp     edi, 1
        jle     .L94
        mov     ecx, OFFSET FLAT:ops+16
        mov     eax, 1
        xor     edx, edx
.L93:
        movsx   rsi, edx
        lea     rsi, [rsi+rsi*2]
        mov     esi, DWORD PTR ops[4+rsi*4]
        cmp     DWORD PTR [rcx], esi
        cmovg   edx, eax
        add     eax, 1
        add     rcx, 12
        cmp     edi, eax
        jne     .L93
        mov     eax, edx
        ret
.L94:
        xor     edx, edx
        mov     eax, edx
        ret
.LC0:
        .string ""
.LC1:
        .string "%*s"
.LC2:
        .string " Invalid expression. Please try again.\n"
report_error:
        sub     rsp, 8
        mov     edi, OFFSET FLAT:input_str
        call    puts
        mov     esi, DWORD PTR error_pos[rip]
        mov     edx, OFFSET FLAT:.LC0
        xor     eax, eax
        mov     edi, OFFSET FLAT:.LC1
        call    printf
        mov     edi, 94
        call    putchar
        mov     edi, OFFSET FLAT:.LC2
        add     rsp, 8
        jmp     puts
check_add:
        test    edi, edi
        jns     .L101
        mov     eax, -2147483648
        sub     eax, edi
        cmp     eax, esi
        setle   al
        movzx   eax, al
        ret
.L101:
        mov     eax, 2147483647
        sub     eax, edi
        cmp     eax, esi
        setge   al
        movzx   eax, al
        ret
check_sub:
        cmp     esi, -2147483648
        je      .L106
        neg     esi
        test    edi, edi
        jns     .L107
        mov     eax, -2147483648
        sub     eax, edi
        cmp     esi, eax
        setge   al
        movzx   eax, al
        ret
.L107:
        mov     eax, 2147483647
        sub     eax, edi
        cmp     esi, eax
        setle   al
        movzx   eax, al
        ret
.L106:
        mov     eax, edi
        shr     eax, 31
        ret
check_div:
        xor     eax, eax
        test    esi, esi
        setne   al
        ret
check_mul:
        test    edi, edi
        je      .L115
        test    esi, esi
        je      .L115
        test    edi, edi
        jle     .L111
        test    esi, esi
        jle     .L111
        xor     edx, edx
        mov     eax, 2147483647
        idiv    edi
        cmp     eax, esi
        setge   al
        movzx   eax, al
        ret
.L111:
        test    edi, esi
        jns     .L112
        cmp     edi, -2147483648
        je      .L116
        cmp     esi, -2147483648
        je      .L116
        neg     edi
        xor     edx, edx
        mov     eax, 2147483647
        neg     esi
        idiv    edi
        cmp     esi, eax
        setle   al
        movzx   eax, al
        ret
.L115:
        mov     eax, 1
        ret
.L112:
        cmp     edi, -2147483648
        je      .L123
        cmp     esi, -2147483648
        je      .L124
        mov     eax, esi
        mov     ecx, edi
        neg     eax
        cmovns  esi, eax
        neg     ecx
        mov     eax, 2147483647
        cmovs   ecx, edi
        xor     edx, edx
        idiv    ecx
        cmp     esi, eax
        setle   al
        movzx   eax, al
        ret
.L116:
        xor     eax, eax
        ret
.L123:
        xor     eax, eax
        cmp     esi, 1
        sete    al
        ret
.L124:
        xor     eax, eax
        cmp     edi, 1
        sete    al
        ret
calc:
        movsx   r8, edi
        add     edi, 1
        lea     rax, [r8+r8*2]
        movsx   rdi, edi
        mov     r10d, DWORD PTR nums[0+r8*4]
        mov     eax, DWORD PTR ops[0+rax*4]
        mov     r9d, DWORD PTR nums[0+rdi*4]
        cmp     eax, 2
        je      .L126
        ja      .L127
        test    eax, eax
        je      .L149
        mov     eax, r10d
        shr     eax, 31
        cmp     r9d, -2147483648
        je      .L136
        mov     edx, r9d
        neg     edx
        test    r10d, r10d
        jns     .L150
        mov     eax, -2147483648
        sub     eax, r10d
        cmp     edx, eax
        jge     .L140
.L138:
        lea     rax, [r8+r8*2]
        mov     eax, DWORD PTR ops[8+rax*4]
        mov     DWORD PTR error_pos[rip], eax
.L134:
        xor     eax, eax
        ret
.L127:
        cmp     eax, 3
        jne     .L134
        mov     esi, r9d
        mov     edi, r10d
        call    check_mul
        test    eax, eax
        je      .L138
        imul    r9d, r10d
        mov     DWORD PTR nums[0+r8*4], r9d
.L139:
        mov     eax, 1
        ret
.L150:
        mov     eax, 2147483647
        sub     eax, r10d
        cmp     edx, eax
        setle   al
        movzx   eax, al
.L136:
        test    eax, eax
        je      .L138
.L140:
        sub     r10d, r9d
        mov     eax, 1
        mov     DWORD PTR nums[0+r8*4], r10d
        ret
.L126:
        test    r9d, r9d
        je      .L138
        mov     eax, r10d
        cdq
        idiv    r9d
        mov     DWORD PTR nums[0+r8*4], eax
        jmp     .L139
.L149:
        test    r10d, r10d
        jns     .L151
        mov     eax, -2147483648
        sub     eax, r10d
        cmp     r9d, eax
        setge   al
        movzx   eax, al
.L133:
        test    eax, eax
        je      .L138
        add     r9d, r10d
        mov     DWORD PTR nums[0+r8*4], r9d
        jmp     .L139
.L151:
        mov     eax, 2147483647
        sub     eax, r10d
        cmp     r9d, eax
        setle   al
        movzx   eax, al
        jmp     .L133
perform_calculation:
        cmp     DWORD PTR numSize[rip], 1
        jle     .L172
        push    r12
        push    rbp
        mov     ebp, DWORD PTR opSize[rip]
        push    rbx
.L154:
        mov     edx, OFFSET FLAT:ops+16
        xor     ebx, ebx
        mov     eax, 1
        cmp     ebp, 1
        jle     .L163
.L156:
        movsx   rcx, ebx
        lea     rcx, [rcx+rcx*2]
        mov     esi, DWORD PTR ops[4+rcx*4]
        cmp     DWORD PTR [rdx], esi
        cmovg   ebx, eax
        add     eax, 1
        add     rdx, 12
        cmp     ebp, eax
        jne     .L156
.L163:
        mov     edi, ebx
        call    calc
        test    eax, eax
        je      .L152
        mov     edx, DWORD PTR numSize[rip]
        lea     eax, [rbx+1]
        lea     r12d, [rdx-1]
        cmp     eax, r12d
        jge     .L161
        cdqe
        sub     edx, 3
        sal     rax, 2
        sub     edx, ebx
        lea     rdx, [4+rdx*4]
        lea     rsi, nums[rax+4]
        lea     rdi, nums[rax]
        call    memmove
.L161:
        mov     eax, DWORD PTR opSize[rip]
        mov     DWORD PTR numSize[rip], r12d
        lea     ebp, [rax-1]
        cmp     ebp, ebx
        jle     .L160
        movsx   rdx, ebx
        sub     eax, 2
        lea     rdi, [rdx+rdx*2]
        sub     eax, ebx
        sal     rdi, 2
        lea     rdx, [rax+3+rax*2]
        lea     rsi, ops[rdi+12]
        sal     rdx, 2
        add     rdi, OFFSET FLAT:ops
        call    memmove
.L160:
        mov     DWORD PTR opSize[rip], ebp
        cmp     r12d, 1
        jg      .L154
        mov     eax, 1
.L152:
        pop     rbx
        pop     rbp
        pop     r12
        ret
.L172:
        mov     eax, 1
        ret
.LC3:
        .string "Enter the formula to be calculated."
.LC4:
        .string "Press Ctrl+C to exit."
.LC5:
        .string "Input: "
.LC6:
        .string "Result: %d\n\n"
main:
        sub     rsp, 8
.L177:
        mov     edi, OFFSET FLAT:.LC3
        mov     BYTE PTR input_str[rip], 0
        mov     DWORD PTR numSize[rip], 0
        mov     DWORD PTR opSize[rip], 0
        mov     DWORD PTR priorityBase[rip], 0
        mov     DWORD PTR error_pos[rip], 0
        call    puts
        mov     edi, OFFSET FLAT:.LC4
        call    puts
        mov     edi, OFFSET FLAT:.LC5
        xor     eax, eax
        call    printf
        mov     edi, OFFSET FLAT:input_str
        call    receive_input
        test    rax, rax
        je      .L178
        movzx   eax, BYTE PTR input_str[rip]
        test    al, al
        je      .L186
        mov     edx, OFFSET FLAT:input_str
        mov     rcx, rdx
.L181:
        cmp     al, 32
        je      .L180
        cmp     al, 10
        je      .L180
        mov     BYTE PTR [rcx], al
        add     rcx, 1
.L180:
        movzx   eax, BYTE PTR [rdx+1]
        add     rdx, 1
        test    al, al
        jne     .L181
.L179:
        mov     BYTE PTR [rcx], 0
        cmp     BYTE PTR input_str[rip], 0
        je      .L195
        xor     eax, eax
        call    parse_input
        test    eax, eax
        je      .L196
        xor     eax, eax
        call    perform_calculation
        test    eax, eax
        je      .L197
        mov     esi, DWORD PTR nums[rip]
        mov     edi, OFFSET FLAT:.LC6
        xor     eax, eax
        call    printf
        jmp     .L177
.L195:
        mov     edi, 10
        call    putchar
        jmp     .L177
.L196:
        mov     edi, OFFSET FLAT:input_str
        call    puts
        mov     esi, DWORD PTR error_pos[rip]
        mov     edx, OFFSET FLAT:.LC0
        xor     eax, eax
        mov     edi, OFFSET FLAT:.LC1
        call    printf
        mov     edi, 94
        call    putchar
        mov     edi, OFFSET FLAT:.LC2
        call    puts
        jmp     .L177
.L197:
        call    report_error
        jmp     .L177
.L178:
        xor     eax, eax
        add     rsp, 8
        ret
.L186:
        mov     ecx, OFFSET FLAT:input_str
        jmp     .L179
error_pos:
        .zero   4
opSize:
        .zero   4
numSize:
        .zero   4
priorityBase:
        .zero   4
ops:
        .zero   1536
nums:
        .zero   512
input_str:
        .zero   128