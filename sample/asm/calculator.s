.intel_syntax noprefix
.file 1 ".\sample\calculator.c"
  .data
  .globl .L..29
.L..29:
  .byte 32
  .byte 73
  .byte 110
  .byte 118
  .byte 97
  .byte 108
  .byte 105
  .byte 100
  .byte 32
  .byte 101
  .byte 120
  .byte 112
  .byte 114
  .byte 101
  .byte 115
  .byte 115
  .byte 105
  .byte 111
  .byte 110
  .byte 46
  .byte 32
  .byte 80
  .byte 108
  .byte 101
  .byte 97
  .byte 115
  .byte 101
  .byte 32
  .byte 116
  .byte 114
  .byte 121
  .byte 32
  .byte 97
  .byte 103
  .byte 97
  .byte 105
  .byte 110
  .byte 46
  .byte 10
  .byte 10
  .byte 0
  .data
  .globl .L..28
.L..28:
  .byte 94
  .byte 0
  .data
  .globl .L..27
.L..27:
  .byte 0
  .data
  .globl .L..26
.L..26:
  .byte 37
  .byte 42
  .byte 115
  .byte 0
  .data
  .globl .L..25
.L..25:
  .byte 37
  .byte 115
  .byte 10
  .byte 0
  .data
  .globl .L..6
.L..6:
  .byte 82
  .byte 101
  .byte 115
  .byte 117
  .byte 108
  .byte 116
  .byte 58
  .byte 32
  .byte 37
  .byte 100
  .byte 10
  .byte 10
  .byte 0
  .data
  .globl .L..5
.L..5:
  .byte 10
  .byte 0
  .data
  .globl .L..4
.L..4:
  .byte 73
  .byte 110
  .byte 112
  .byte 117
  .byte 116
  .byte 58
  .byte 32
  .byte 0
  .data
  .globl .L..3
.L..3:
  .byte 80
  .byte 114
  .byte 101
  .byte 115
  .byte 115
  .byte 32
  .byte 67
  .byte 116
  .byte 114
  .byte 108
  .byte 43
  .byte 67
  .byte 32
  .byte 116
  .byte 111
  .byte 32
  .byte 101
  .byte 120
  .byte 105
  .byte 116
  .byte 46
  .byte 10
  .byte 0
  .data
  .globl .L..2
.L..2:
  .byte 69
  .byte 110
  .byte 116
  .byte 101
  .byte 114
  .byte 32
  .byte 116
  .byte 104
  .byte 101
  .byte 32
  .byte 102
  .byte 111
  .byte 114
  .byte 109
  .byte 117
  .byte 108
  .byte 97
  .byte 32
  .byte 116
  .byte 111
  .byte 32
  .byte 98
  .byte 101
  .byte 32
  .byte 99
  .byte 97
  .byte 108
  .byte 99
  .byte 117
  .byte 108
  .byte 97
  .byte 116
  .byte 101
  .byte 100
  .byte 46
  .byte 10
  .byte 0
  .data
  .globl error_pos
error_pos:
  .zero 4
  .data
  .globl opSize
opSize:
  .zero 4
  .data
  .globl numSize
numSize:
  .zero 4
  .data
  .globl priorityBase
priorityBase:
  .zero 4
  .data
  .globl ops
ops:
  .zero 1536
  .data
  .globl nums
nums:
  .zero 512
  .data
  .globl input_str
input_str:
  .zero 128
  .globl check_mul
  .text
check_mul:
  push rbp
  mov rbp, rsp
  sub rsp, 32
  mov [rbp - 4], edi
  mov [rbp - 8], esi
  .loc 1 577
  .loc 1 577
  .loc 1 577
  .loc 1 577
  lea rax, [rbp - 32]
  push rax
  .loc 1 577
  .loc 1 577
  mov rax, 2147483647
  pop rdi
  mov [rdi], eax
  .loc 1 578
  .loc 1 578
  .loc 1 578
  lea rax, [rbp - 28]
  push rax
  .loc 1 578
  .loc 1 578
  .loc 1 578
  .loc 1 578
  mov rax, 2147483648
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 581
  .loc 1 581
  .loc 1 581
  .loc 1 581
  .loc 1 581
  mov rax, 0
  push rax
  .loc 1 581
  .loc 1 581
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.2
  .loc 1 581
  .loc 1 581
  .loc 1 581
  mov rax, 0
  push rax
  .loc 1 581
  .loc 1 581
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.2
  mov rax, 0
  jmp .L.end.2
.L.true.2:
  mov rax, 1
.L.end.2:
  cmp rax, 0
  je .L.else.1
  .loc 1 583
  .loc 1 583
  .loc 1 583
  .loc 1 583
  mov rax, 1
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.1
.L.else.1:
.L.end.1:
  .loc 1 587
  .loc 1 587
  .loc 1 587
  .loc 1 587
  .loc 1 587
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 587
  .loc 1 587
  mov rax, 0
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.false.4
  .loc 1 587
  .loc 1 587
  .loc 1 587
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 587
  .loc 1 587
  mov rax, 0
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.false.4
  mov rax, 1
  jmp .L.end.4
.L.false.4:
  mov rax, 0
.L.end.4:
  cmp rax, 0
  je .L.else.3
  .loc 1 589
  .loc 1 589
  .loc 1 589
  .loc 1 589
  .loc 1 589
  .loc 1 589
  .loc 1 589
  .loc 1 589
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 589
  .loc 1 589
  lea rax, [rbp - 32]
  mov eax, [rax]
  pop rdi
  cdq
  idiv edi
  push rax
  .loc 1 589
  .loc 1 589
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.3
.L.else.3:
.L.end.3:
  .loc 1 593
  .loc 1 593
  .loc 1 593
  .loc 1 593
  .loc 1 593
  mov rax, 0
  push rax
  .loc 1 593
  .loc 1 593
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.false.6
  .loc 1 593
  .loc 1 593
  .loc 1 593
  mov rax, 0
  push rax
  .loc 1 593
  .loc 1 593
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.false.6
  mov rax, 1
  jmp .L.end.6
.L.false.6:
  mov rax, 0
.L.end.6:
  cmp rax, 0
  je .L.else.5
  .loc 1 596
  .loc 1 596
  .loc 1 596
  .loc 1 596
  .loc 1 596
  .loc 1 596
  lea rax, [rbp - 28]
  mov eax, [rax]
  push rax
  .loc 1 596
  .loc 1 596
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.8
  .loc 1 596
  .loc 1 596
  .loc 1 596
  lea rax, [rbp - 28]
  mov eax, [rax]
  push rax
  .loc 1 596
  .loc 1 596
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.8
  mov rax, 0
  jmp .L.end.8
.L.true.8:
  mov rax, 1
.L.end.8:
  cmp rax, 0
  je .L.else.7
  .loc 1 598
  .loc 1 598
  .loc 1 598
  .loc 1 598
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.7
.L.else.7:
.L.end.7:
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  lea rax, [rbp - 4]
  mov eax, [rax]
  neg rax
  push rax
  .loc 1 600
  .loc 1 600
  lea rax, [rbp - 32]
  mov eax, [rax]
  pop rdi
  cdq
  idiv edi
  push rax
  .loc 1 600
  .loc 1 600
  .loc 1 600
  .loc 1 600
  lea rax, [rbp - 8]
  mov eax, [rax]
  neg rax
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.5
.L.else.5:
.L.end.5:
  .loc 1 604
  .loc 1 604
  .loc 1 604
  .loc 1 604
  lea rax, [rbp - 28]
  mov eax, [rax]
  push rax
  .loc 1 604
  .loc 1 604
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.9
  .loc 1 606
  .loc 1 606
  .loc 1 606
  .loc 1 606
  .loc 1 606
  .loc 1 606
  mov rax, 1
  push rax
  .loc 1 606
  .loc 1 606
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.9
.L.else.9:
.L.end.9:
  .loc 1 608
  .loc 1 608
  .loc 1 608
  .loc 1 608
  lea rax, [rbp - 28]
  mov eax, [rax]
  push rax
  .loc 1 608
  .loc 1 608
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.10
  .loc 1 610
  .loc 1 610
  .loc 1 610
  .loc 1 610
  .loc 1 610
  .loc 1 610
  mov rax, 1
  push rax
  .loc 1 610
  .loc 1 610
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_mul
  jmp .L.end.10
.L.else.10:
.L.end.10:
  .loc 1 613
  .loc 1 613
  .loc 1 613
  .loc 1 613
  mov rax, 0
  push rax
  .loc 1 613
  .loc 1 613
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.else.11
  .loc 1 615
  .loc 1 615
  .loc 1 615
  .loc 1 615
  lea rax, [rbp - 24]
  push rax
  .loc 1 615
  .loc 1 615
  lea rax, [rbp - 4]
  pop rdi
  mov [rdi], rax
  .loc 1 615
  .loc 1 615
  lea rax, [rbp - 24]
  mov rax, [rax]
  push rax
  .loc 1 615
  .loc 1 615
  .loc 1 615
  .loc 1 615
  .loc 1 615
  .loc 1 615
  mov rax, 1
  neg rax
  push rax
  .loc 1 615
  .loc 1 615
  .loc 1 615
  lea rax, [rbp - 24]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  imul eax, edi
  pop rdi
  mov [rdi], eax
  jmp .L.end.11
.L.else.11:
.L.end.11:
  .loc 1 618
  .loc 1 618
  .loc 1 618
  .loc 1 618
  mov rax, 0
  push rax
  .loc 1 618
  .loc 1 618
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.else.12
  .loc 1 620
  .loc 1 620
  .loc 1 620
  .loc 1 620
  lea rax, [rbp - 16]
  push rax
  .loc 1 620
  .loc 1 620
  lea rax, [rbp - 8]
  pop rdi
  mov [rdi], rax
  .loc 1 620
  .loc 1 620
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 620
  .loc 1 620
  .loc 1 620
  .loc 1 620
  .loc 1 620
  .loc 1 620
  mov rax, 1
  neg rax
  push rax
  .loc 1 620
  .loc 1 620
  .loc 1 620
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  imul eax, edi
  pop rdi
  mov [rdi], eax
  jmp .L.end.12
.L.else.12:
.L.end.12:
  .loc 1 623
  .loc 1 623
  .loc 1 623
  .loc 1 623
  .loc 1 623
  .loc 1 623
  .loc 1 623
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 623
  .loc 1 623
  lea rax, [rbp - 32]
  mov eax, [rax]
  pop rdi
  cdq
  idiv edi
  push rax
  .loc 1 623
  .loc 1 623
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_mul
.L.return.check_mul:
  mov rsp, rbp
  pop rbp
  ret
  .globl check_div
  .text
check_div:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp - 4], edi
  mov [rbp - 8], esi
  .loc 1 565
  .loc 1 565
  .loc 1 565
  .loc 1 565
  .loc 1 565
  .loc 1 565
  mov rax, 0
  push rax
  .loc 1 565
  .loc 1 565
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_div
.L.return.check_div:
  mov rsp, rbp
  pop rbp
  ret
  .globl check_sub
  .text
check_sub:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp - 4], edi
  mov [rbp - 8], esi
  .loc 1 545
  .loc 1 545
  .loc 1 545
  .loc 1 545
  lea rax, [rbp - 12]
  push rax
  .loc 1 545
  .loc 1 545
  .loc 1 545
  .loc 1 545
  mov rax, 2147483648
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 547
  .loc 1 547
  .loc 1 547
  .loc 1 547
  lea rax, [rbp - 12]
  mov eax, [rax]
  push rax
  .loc 1 547
  .loc 1 547
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.13
  .loc 1 549
  .loc 1 549
  .loc 1 549
  .loc 1 549
  .loc 1 549
  .loc 1 549
  mov rax, 0
  push rax
  .loc 1 549
  .loc 1 549
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_sub
  jmp .L.end.13
.L.else.13:
.L.end.13:
  .loc 1 551
  .loc 1 551
  .loc 1 551
  .loc 1 551
  .loc 1 551
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 551
  .loc 1 551
  .loc 1 551
  lea rax, [rbp - 8]
  mov eax, [rax]
  neg rax
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call check_add
  jmp .L.return.check_sub
.L.return.check_sub:
  mov rsp, rbp
  pop rbp
  ret
  .globl check_add
  .text
check_add:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp - 4], edi
  mov [rbp - 8], esi
  .loc 1 522
  .loc 1 522
  .loc 1 522
  .loc 1 522
  lea rax, [rbp - 16]
  push rax
  .loc 1 522
  .loc 1 522
  mov rax, 2147483647
  pop rdi
  mov [rdi], eax
  .loc 1 523
  .loc 1 523
  .loc 1 523
  lea rax, [rbp - 12]
  push rax
  .loc 1 523
  .loc 1 523
  .loc 1 523
  .loc 1 523
  mov rax, 2147483648
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 526
  .loc 1 526
  .loc 1 526
  .loc 1 526
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 526
  .loc 1 526
  mov rax, 0
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  cmp rax, 0
  je .L.else.14
  .loc 1 528
  .loc 1 528
  .loc 1 528
  .loc 1 528
  .loc 1 528
  .loc 1 528
  .loc 1 528
  .loc 1 528
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 528
  .loc 1 528
  lea rax, [rbp - 16]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  push rax
  .loc 1 528
  .loc 1 528
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_add
  jmp .L.end.14
.L.else.14:
.L.end.14:
  .loc 1 532
  .loc 1 532
  .loc 1 532
  .loc 1 532
  .loc 1 532
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 532
  .loc 1 532
  .loc 1 532
  .loc 1 532
  lea rax, [rbp - 4]
  mov eax, [rax]
  push rax
  .loc 1 532
  .loc 1 532
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  movsxd rax, eax
  jmp .L.return.check_add
.L.return.check_add:
  mov rsp, rbp
  pop rbp
  ret
  .globl report_error
  .text
report_error:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  .loc 1 506
  .loc 1 506
  .loc 1 506
  .loc 1 506
  .loc 1 506
  lea rax, [rip + .L..25]
  push rax
  .loc 1 506
  lea rax, [rip + input_str]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call printf
  .loc 1 507
  .loc 1 507
  .loc 1 507
  .loc 1 507
  lea rax, [rip + .L..26]
  push rax
  .loc 1 507
  lea rax, [rip + error_pos]
  mov eax, [rax]
  push rax
  .loc 1 507
  lea rax, [rip + .L..27]
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call printf
  .loc 1 508
  .loc 1 508
  .loc 1 508
  .loc 1 508
  lea rax, [rip + .L..28]
  push rax
  pop rdi
  mov rax, 0
  call printf
  .loc 1 509
  .loc 1 509
  .loc 1 509
  .loc 1 509
  lea rax, [rip + .L..29]
  push rax
  pop rdi
  mov rax, 0
  call printf
.L.return.report_error:
  mov rsp, rbp
  pop rbp
  ret
  .globl calc
  .text
calc:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  mov [rbp - 4], edi
  .loc 1 436
  .loc 1 436
  .loc 1 436
  .loc 1 436
  lea rax, [rbp - 12]
  push rax
  .loc 1 436
  .loc 1 436
  .loc 1 436
  .loc 1 436
  .loc 1 436
  .loc 1 436
  .loc 1 436
  mov rax, 4
  push rax
  .loc 1 436
  .loc 1 436
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 436
  .loc 1 436
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 437
  .loc 1 437
  .loc 1 437
  lea rax, [rbp - 8]
  push rax
  .loc 1 437
  .loc 1 437
  .loc 1 437
  .loc 1 437
  .loc 1 437
  .loc 1 437
  .loc 1 437
  mov rax, 4
  push rax
  .loc 1 437
  .loc 1 437
  .loc 1 437
  .loc 1 437
  mov rax, 1
  push rax
  .loc 1 437
  .loc 1 437
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  add eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 437
  .loc 1 437
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 438
  .loc 1 438
  .loc 1 438
  .loc 1 438
  .loc 1 438
  .loc 1 438
  .loc 1 438
  mov rax, 12
  push rax
  .loc 1 438
  .loc 1 438
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 438
  .loc 1 438
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 0
  mov eax, [rax]
  cmp eax, 3
  je .L..23
  cmp eax, 2
  je .L..22
  cmp eax, 1
  je .L..21
  cmp eax, 0
  je .L..20
  jmp .L..24
  jmp .L..19
  .loc 1 441
  .loc 1 441
.L..20:
  .loc 1 442
  .loc 1 442
  .loc 1 442
  .loc 1 442
  lea rax, [rbp - 12]
  mov eax, [rax]
  push rax
  .loc 1 442
  .loc 1 442
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call check_add
  cmp rax, 0
  je .L.else.15
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  mov rax, 4
  push rax
  .loc 1 444
  .loc 1 444
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 444
  .loc 1 444
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 444
  .loc 1 444
  .loc 1 444
  .loc 1 444
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 444
  .loc 1 444
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 445
  jmp .L..19
  jmp .L.end.15
.L.else.15:
  .loc 1 449
  .loc 1 449
  .loc 1 449
  lea rax, [rip + error_pos]
  push rax
  .loc 1 449
  .loc 1 449
  .loc 1 449
  .loc 1 449
  .loc 1 449
  .loc 1 449
  .loc 1 449
  mov rax, 12
  push rax
  .loc 1 449
  .loc 1 449
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 449
  .loc 1 449
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 8
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 450
  .loc 1 450
  .loc 1 450
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.calc
.L.end.15:
  .loc 1 454
.L..21:
  .loc 1 455
  .loc 1 455
  .loc 1 455
  .loc 1 455
  lea rax, [rbp - 12]
  mov eax, [rax]
  push rax
  .loc 1 455
  .loc 1 455
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call check_sub
  cmp rax, 0
  je .L.else.16
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  mov rax, 4
  push rax
  .loc 1 457
  .loc 1 457
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 457
  .loc 1 457
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 457
  .loc 1 457
  .loc 1 457
  .loc 1 457
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 457
  .loc 1 457
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 458
  jmp .L..19
  jmp .L.end.16
.L.else.16:
  .loc 1 462
  .loc 1 462
  .loc 1 462
  lea rax, [rip + error_pos]
  push rax
  .loc 1 462
  .loc 1 462
  .loc 1 462
  .loc 1 462
  .loc 1 462
  .loc 1 462
  .loc 1 462
  mov rax, 12
  push rax
  .loc 1 462
  .loc 1 462
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 462
  .loc 1 462
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 8
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 463
  .loc 1 463
  .loc 1 463
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.calc
.L.end.16:
  .loc 1 467
.L..22:
  .loc 1 468
  .loc 1 468
  .loc 1 468
  .loc 1 468
  lea rax, [rbp - 12]
  mov eax, [rax]
  push rax
  .loc 1 468
  .loc 1 468
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call check_div
  cmp rax, 0
  je .L.else.17
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  mov rax, 4
  push rax
  .loc 1 470
  .loc 1 470
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 470
  .loc 1 470
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 470
  .loc 1 470
  .loc 1 470
  .loc 1 470
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 470
  .loc 1 470
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  cdq
  idiv edi
  pop rdi
  mov [rdi], eax
  .loc 1 471
  jmp .L..19
  jmp .L.end.17
.L.else.17:
  .loc 1 475
  .loc 1 475
  .loc 1 475
  lea rax, [rip + error_pos]
  push rax
  .loc 1 475
  .loc 1 475
  .loc 1 475
  .loc 1 475
  .loc 1 475
  .loc 1 475
  .loc 1 475
  mov rax, 12
  push rax
  .loc 1 475
  .loc 1 475
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 475
  .loc 1 475
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 8
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 476
  .loc 1 476
  .loc 1 476
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.calc
.L.end.17:
  .loc 1 480
.L..23:
  .loc 1 481
  .loc 1 481
  .loc 1 481
  .loc 1 481
  lea rax, [rbp - 12]
  mov eax, [rax]
  push rax
  .loc 1 481
  .loc 1 481
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call check_mul
  cmp rax, 0
  je .L.else.18
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  mov rax, 4
  push rax
  .loc 1 483
  .loc 1 483
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 483
  .loc 1 483
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 483
  .loc 1 483
  .loc 1 483
  .loc 1 483
  lea rax, [rbp - 8]
  mov eax, [rax]
  push rax
  .loc 1 483
  .loc 1 483
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  imul eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 484
  jmp .L..19
  jmp .L.end.18
.L.else.18:
  .loc 1 488
  .loc 1 488
  .loc 1 488
  lea rax, [rip + error_pos]
  push rax
  .loc 1 488
  .loc 1 488
  .loc 1 488
  .loc 1 488
  .loc 1 488
  .loc 1 488
  .loc 1 488
  mov rax, 12
  push rax
  .loc 1 488
  .loc 1 488
  lea rax, [rbp - 4]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 488
  .loc 1 488
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 8
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 489
  .loc 1 489
  .loc 1 489
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.calc
.L.end.18:
  .loc 1 491
.L..24:
  .loc 1 493
  .loc 1 493
  .loc 1 493
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.calc
.L..19:
  .loc 1 495
  .loc 1 495
  .loc 1 495
  mov rax, 1
  movsxd rax, eax
  jmp .L.return.calc
.L.return.calc:
  mov rsp, rbp
  pop rbp
  ret
  .globl perform_calculation
  .text
perform_calculation:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  .loc 1 401
  .loc 1 401
.L.begin.19:
  .loc 1 401
  .loc 1 401
  .loc 1 401
  lea rax, [rip + numSize]
  mov eax, [rax]
  push rax
  .loc 1 401
  .loc 1 401
  mov rax, 1
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..13
  .loc 1 404
  .loc 1 404
  .loc 1 404
  .loc 1 404
  lea rax, [rbp - 48]
  push rax
  .loc 1 404
  .loc 1 404
  mov rax, 0
  call find_highest_priority_op
  pop rdi
  mov [rdi], eax
  .loc 1 406
  .loc 1 406
  .loc 1 406
  .loc 1 406
  mov rax, 0
  push rax
  .loc 1 406
  .loc 1 406
  .loc 1 406
  .loc 1 406
  lea rax, [rbp - 48]
  mov eax, [rax]
  push rax
  pop rdi
  mov rax, 0
  call calc
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.20
  .loc 1 408
  .loc 1 408
  .loc 1 408
  .loc 1 408
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.perform_calculation
  jmp .L.end.20
.L.else.20:
.L.end.20:
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 44]
  push rax
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  mov rax, 1
  push rax
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 48]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
.L.begin.21:
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  mov rax, 1
  push rax
  .loc 1 411
  .loc 1 411
  lea rax, [rip + numSize]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  push rax
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 44]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..15
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  mov rax, 4
  push rax
  .loc 1 413
  .loc 1 413
  lea rax, [rbp - 44]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 413
  .loc 1 413
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  mov rax, 4
  push rax
  .loc 1 413
  .loc 1 413
  .loc 1 413
  .loc 1 413
  mov rax, 1
  push rax
  .loc 1 413
  .loc 1 413
  lea rax, [rbp - 44]
  mov eax, [rax]
  pop rdi
  add eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 413
  .loc 1 413
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
.L..16:
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  mov rax, -1
  push rax
  .loc 1 411
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 40]
  push rax
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 44]
  pop rdi
  mov [rdi], rax
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 40]
  mov rax, [rax]
  push rax
  .loc 1 411
  .loc 1 411
  .loc 1 411
  .loc 1 411
  mov rax, 1
  push rax
  .loc 1 411
  .loc 1 411
  .loc 1 411
  lea rax, [rbp - 40]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.21
.L..15:
  .loc 1 415
  .loc 1 415
  .loc 1 415
  .loc 1 415
  .loc 1 415
  mov rax, 1
  push rax
  .loc 1 415
  .loc 1 415
  .loc 1 415
  lea rax, [rbp - 32]
  push rax
  .loc 1 415
  .loc 1 415
  lea rax, [rip + numSize]
  pop rdi
  mov [rdi], rax
  .loc 1 415
  .loc 1 415
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 415
  .loc 1 415
  .loc 1 415
  .loc 1 415
  mov rax, -1
  push rax
  .loc 1 415
  .loc 1 415
  .loc 1 415
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  .loc 1 417
  .loc 1 417
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 20]
  push rax
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 48]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
.L.begin.22:
  .loc 1 417
  .loc 1 417
  .loc 1 417
  .loc 1 417
  .loc 1 417
  mov rax, 1
  push rax
  .loc 1 417
  .loc 1 417
  lea rax, [rip + opSize]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  push rax
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..17
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  mov rax, 12
  push rax
  .loc 1 419
  .loc 1 419
  lea rax, [rbp - 20]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 419
  .loc 1 419
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  mov rax, 12
  push rax
  .loc 1 419
  .loc 1 419
  .loc 1 419
  .loc 1 419
  mov rax, 1
  push rax
  .loc 1 419
  .loc 1 419
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  add eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 419
  .loc 1 419
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  pop rdi
  mov r8b, [rax + 0 ]
  mov [rdi + 0 ], r8b
  mov r8b, [rax + 1 ]
  mov [rdi + 1 ], r8b
  mov r8b, [rax + 2 ]
  mov [rdi + 2 ], r8b
  mov r8b, [rax + 3 ]
  mov [rdi + 3 ], r8b
  mov r8b, [rax + 4 ]
  mov [rdi + 4 ], r8b
  mov r8b, [rax + 5 ]
  mov [rdi + 5 ], r8b
  mov r8b, [rax + 6 ]
  mov [rdi + 6 ], r8b
  mov r8b, [rax + 7 ]
  mov [rdi + 7 ], r8b
  mov r8b, [rax + 8 ]
  mov [rdi + 8 ], r8b
  mov r8b, [rax + 9 ]
  mov [rdi + 9 ], r8b
  mov r8b, [rax + 10 ]
  mov [rdi + 10 ], r8b
  mov r8b, [rax + 11 ]
  mov [rdi + 11 ], r8b
.L..18:
  .loc 1 417
  .loc 1 417
  .loc 1 417
  .loc 1 417
  mov rax, -1
  push rax
  .loc 1 417
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 16]
  push rax
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 20]
  pop rdi
  mov [rdi], rax
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 417
  .loc 1 417
  .loc 1 417
  .loc 1 417
  mov rax, 1
  push rax
  .loc 1 417
  .loc 1 417
  .loc 1 417
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.22
.L..17:
  .loc 1 421
  .loc 1 421
  .loc 1 421
  .loc 1 421
  .loc 1 421
  mov rax, 1
  push rax
  .loc 1 421
  .loc 1 421
  .loc 1 421
  lea rax, [rbp - 8]
  push rax
  .loc 1 421
  .loc 1 421
  lea rax, [rip + opSize]
  pop rdi
  mov [rdi], rax
  .loc 1 421
  .loc 1 421
  lea rax, [rbp - 8]
  mov rax, [rax]
  push rax
  .loc 1 421
  .loc 1 421
  .loc 1 421
  .loc 1 421
  mov rax, -1
  push rax
  .loc 1 421
  .loc 1 421
  .loc 1 421
  lea rax, [rbp - 8]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
.L..14:
  jmp .L.begin.19
.L..13:
  .loc 1 424
  .loc 1 424
  .loc 1 424
  mov rax, 1
  movsxd rax, eax
  jmp .L.return.perform_calculation
.L.return.perform_calculation:
  mov rsp, rbp
  pop rbp
  ret
  .globl find_highest_priority_op
  .text
find_highest_priority_op:
  push rbp
  mov rbp, rsp
  sub rsp, 16
  .loc 1 382
  .loc 1 382
  .loc 1 382
  .loc 1 382
  lea rax, [rbp - 16]
  push rax
  .loc 1 382
  .loc 1 382
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 383
  .loc 1 383
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 12]
  push rax
  .loc 1 383
  .loc 1 383
  mov rax, 1
  pop rdi
  mov [rdi], eax
.L.begin.23:
  .loc 1 383
  .loc 1 383
  .loc 1 383
  lea rax, [rip + opSize]
  mov eax, [rax]
  push rax
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..11
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  mov rax, 12
  push rax
  .loc 1 385
  .loc 1 385
  lea rax, [rbp - 12]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 385
  .loc 1 385
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 4
  mov eax, [rax]
  push rax
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  .loc 1 385
  mov rax, 12
  push rax
  .loc 1 385
  .loc 1 385
  lea rax, [rbp - 16]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 385
  .loc 1 385
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 4
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.else.24
  .loc 1 387
  .loc 1 387
  .loc 1 387
  lea rax, [rbp - 16]
  push rax
  .loc 1 387
  .loc 1 387
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  jmp .L.end.24
.L.else.24:
.L.end.24:
.L..12:
  .loc 1 383
  .loc 1 383
  .loc 1 383
  .loc 1 383
  mov rax, -1
  push rax
  .loc 1 383
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 8]
  push rax
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 12]
  pop rdi
  mov [rdi], rax
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 8]
  mov rax, [rax]
  push rax
  .loc 1 383
  .loc 1 383
  .loc 1 383
  .loc 1 383
  mov rax, 1
  push rax
  .loc 1 383
  .loc 1 383
  .loc 1 383
  lea rax, [rbp - 8]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.23
.L..11:
  .loc 1 390
  .loc 1 390
  .loc 1 390
  lea rax, [rbp - 16]
  mov eax, [rax]
  jmp .L.return.find_highest_priority_op
.L.return.find_highest_priority_op:
  mov rsp, rbp
  pop rbp
  ret
  .globl setOP
  .text
setOP:
  push rbp
  mov rbp, rsp
  sub rsp, 32
  mov [rbp - 4], edi
  mov [rbp - 8], esi
  mov [rbp - 12], edx
  .loc 1 354
  .loc 1 354
  .loc 1 354
  .loc 1 354
  .loc 1 354
  .loc 1 354
  .loc 1 354
  .loc 1 354
  mov rax, 4
  push rax
  .loc 1 354
  .loc 1 354
  lea rax, [rip + numSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 354
  .loc 1 354
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 354
  .loc 1 354
  lea rax, [rbp - 8]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 357
  .loc 1 357
  .loc 1 357
  .loc 1 357
  .loc 1 357
  .loc 1 357
  .loc 1 357
  mov rax, 12
  push rax
  .loc 1 357
  .loc 1 357
  lea rax, [rip + opSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 357
  .loc 1 357
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 0
  push rax
  .loc 1 357
  .loc 1 357
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 360
  .loc 1 360
  .loc 1 360
  .loc 1 360
  .loc 1 360
  mov rax, 0
  push rax
  .loc 1 360
  .loc 1 360
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.26
  .loc 1 360
  .loc 1 360
  .loc 1 360
  mov rax, 1
  push rax
  .loc 1 360
  .loc 1 360
  lea rax, [rbp - 4]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.26
  mov rax, 0
  jmp .L.end.26
.L.true.26:
  mov rax, 1
.L.end.26:
  cmp rax, 0
  je .L.else.25
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  mov rax, 12
  push rax
  .loc 1 362
  .loc 1 362
  lea rax, [rip + opSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 362
  .loc 1 362
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 4
  push rax
  .loc 1 362
  .loc 1 362
  .loc 1 362
  .loc 1 362
  mov rax, 1
  push rax
  .loc 1 362
  .loc 1 362
  lea rax, [rip + priorityBase]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  jmp .L.end.25
.L.else.25:
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  mov rax, 12
  push rax
  .loc 1 366
  .loc 1 366
  lea rax, [rip + opSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 366
  .loc 1 366
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 4
  push rax
  .loc 1 366
  .loc 1 366
  .loc 1 366
  .loc 1 366
  mov rax, 2
  push rax
  .loc 1 366
  .loc 1 366
  lea rax, [rip + priorityBase]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
.L.end.25:
  .loc 1 370
  .loc 1 370
  .loc 1 370
  .loc 1 370
  .loc 1 370
  .loc 1 370
  .loc 1 370
  mov rax, 12
  push rax
  .loc 1 370
  .loc 1 370
  lea rax, [rip + opSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 370
  .loc 1 370
  lea rax, [rip + ops]
  pop rdi
  add rax, rdi
  add rax, 8
  push rax
  .loc 1 370
  .loc 1 370
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 371
  .loc 1 371
  .loc 1 371
  .loc 1 371
  .loc 1 371
  mov rax, -1
  push rax
  .loc 1 371
  .loc 1 371
  .loc 1 371
  lea rax, [rbp - 32]
  push rax
  .loc 1 371
  .loc 1 371
  lea rax, [rip + numSize]
  pop rdi
  mov [rdi], rax
  .loc 1 371
  .loc 1 371
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 371
  .loc 1 371
  .loc 1 371
  .loc 1 371
  mov rax, 1
  push rax
  .loc 1 371
  .loc 1 371
  .loc 1 371
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  .loc 1 372
  .loc 1 372
  .loc 1 372
  .loc 1 372
  .loc 1 372
  mov rax, -1
  push rax
  .loc 1 372
  .loc 1 372
  .loc 1 372
  lea rax, [rbp - 24]
  push rax
  .loc 1 372
  .loc 1 372
  lea rax, [rip + opSize]
  pop rdi
  mov [rdi], rax
  .loc 1 372
  .loc 1 372
  lea rax, [rbp - 24]
  mov rax, [rax]
  push rax
  .loc 1 372
  .loc 1 372
  .loc 1 372
  .loc 1 372
  mov rax, 1
  push rax
  .loc 1 372
  .loc 1 372
  .loc 1 372
  lea rax, [rbp - 24]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
.L.return.setOP:
  mov rsp, rbp
  pop rbp
  ret
  .globl parse_input
  .text
parse_input:
  push rbp
  mov rbp, rsp
  sub rsp, 64
  .loc 1 171
  .loc 1 171
  .loc 1 171
  .loc 1 171
  lea rax, [rbp - 60]
  push rax
  .loc 1 171
  .loc 1 171
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 172
  .loc 1 172
  .loc 1 172
  lea rax, [rbp - 56]
  push rax
  .loc 1 172
  .loc 1 172
  .loc 1 172
  .loc 1 172
  lea rax, [rip + input_str]
  push rax
  pop rdi
  mov rax, 0
  call strlen
  pop rdi
  mov [rdi], eax
  .loc 1 174
  .loc 1 174
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 52]
  push rax
  .loc 1 174
  .loc 1 174
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.27:
  .loc 1 174
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 56]
  mov eax, [rax]
  push rax
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..9
  .loc 1 176
  .loc 1 176
  .loc 1 176
  .loc 1 176
  lea rax, [rbp - 37]
  push rax
  .loc 1 176
  .loc 1 176
  .loc 1 176
  .loc 1 176
  .loc 1 176
  .loc 1 176
  .loc 1 176
  mov rax, 1
  push rax
  .loc 1 176
  .loc 1 176
  lea rax, [rbp - 52]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 176
  .loc 1 176
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  mov [rdi], al
  .loc 1 179
  .loc 1 179
  .loc 1 179
  .loc 1 179
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  je .L.else.28
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  mov rax, 0
  push rax
  .loc 1 182
  .loc 1 182
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.30
  .loc 1 182
  .loc 1 182
  .loc 1 182
  mov rax, 41
  push rax
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  mov rax, 1
  push rax
  .loc 1 182
  .loc 1 182
  .loc 1 182
  .loc 1 182
  mov rax, 1
  push rax
  .loc 1 182
  .loc 1 182
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 182
  .loc 1 182
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.false.30
  mov rax, 1
  jmp .L.end.30
.L.false.30:
  mov rax, 0
.L.end.30:
  cmp rax, 0
  je .L.else.29
  .loc 1 184
  .loc 1 184
  .loc 1 184
  lea rax, [rip + error_pos]
  push rax
  .loc 1 184
  .loc 1 184
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 185
  .loc 1 185
  .loc 1 185
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.29
.L.else.29:
.L.end.29:
  .loc 1 189
  .loc 1 189
  .loc 1 189
  lea rax, [rbp - 36]
  push rax
  .loc 1 189
  .loc 1 189
  .loc 1 189
  .loc 1 189
  mov rax, 48
  push rax
  .loc 1 189
  .loc 1 189
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  sub eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 190
  .loc 1 190
  lea rax, [rbp - 60]
  push rax
  .loc 1 190
  .loc 1 190
  .loc 1 190
  .loc 1 190
  lea rax, [rbp - 36]
  mov eax, [rax]
  push rax
  .loc 1 190
  .loc 1 190
  .loc 1 190
  .loc 1 190
  lea rax, [rbp - 60]
  mov eax, [rax]
  push rax
  .loc 1 190
  .loc 1 190
  mov rax, 10
  pop rdi
  imul eax, edi
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 193
  .loc 1 193
  .loc 1 193
  .loc 1 193
  .loc 1 193
  .loc 1 193
  mov rax, 1
  push rax
  .loc 1 193
  .loc 1 193
  lea rax, [rbp - 56]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  push rax
  .loc 1 193
  .loc 1 193
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.31
  .loc 1 195
  .loc 1 195
  .loc 1 195
  .loc 1 195
  .loc 1 195
  .loc 1 195
  .loc 1 195
  .loc 1 195
  mov rax, 4
  push rax
  .loc 1 195
  .loc 1 195
  lea rax, [rip + numSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 195
  .loc 1 195
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 195
  .loc 1 195
  lea rax, [rbp - 60]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 196
  .loc 1 196
  .loc 1 196
  .loc 1 196
  .loc 1 196
  mov rax, -1
  push rax
  .loc 1 196
  .loc 1 196
  .loc 1 196
  lea rax, [rbp - 32]
  push rax
  .loc 1 196
  .loc 1 196
  lea rax, [rip + numSize]
  pop rdi
  mov [rdi], rax
  .loc 1 196
  .loc 1 196
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 196
  .loc 1 196
  .loc 1 196
  .loc 1 196
  mov rax, 1
  push rax
  .loc 1 196
  .loc 1 196
  .loc 1 196
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.end.31
.L.else.31:
.L.end.31:
  .loc 1 198
  jmp .L..10
  jmp .L.end.28
.L.else.28:
.L.end.28:
  .loc 1 202
  .loc 1 202
  .loc 1 202
  .loc 1 202
  mov rax, 40
  push rax
  .loc 1 202
  .loc 1 202
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.32
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 0
  push rax
  .loc 1 205
  .loc 1 205
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.34
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 1
  push rax
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 1
  push rax
  .loc 1 205
  .loc 1 205
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 205
  .loc 1 205
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  jne .L.true.35
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 41
  push rax
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 1
  push rax
  .loc 1 205
  .loc 1 205
  .loc 1 205
  .loc 1 205
  mov rax, 1
  push rax
  .loc 1 205
  .loc 1 205
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 205
  .loc 1 205
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  jne .L.true.35
  mov rax, 0
  jmp .L.end.35
.L.true.35:
  mov rax, 1
.L.end.35:
  cmp rax, 0
  je .L.false.34
  mov rax, 1
  jmp .L.end.34
.L.false.34:
  mov rax, 0
.L.end.34:
  cmp rax, 0
  je .L.else.33
  .loc 1 207
  .loc 1 207
  .loc 1 207
  lea rax, [rip + error_pos]
  push rax
  .loc 1 207
  .loc 1 207
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 208
  .loc 1 208
  .loc 1 208
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.33
.L.else.33:
.L.end.33:
  .loc 1 211
  .loc 1 211
  .loc 1 211
  lea rax, [rbp - 24]
  push rax
  .loc 1 211
  .loc 1 211
  lea rax, [rip + priorityBase]
  pop rdi
  mov [rdi], rax
  .loc 1 211
  .loc 1 211
  lea rax, [rbp - 24]
  mov rax, [rax]
  push rax
  .loc 1 211
  .loc 1 211
  .loc 1 211
  .loc 1 211
  mov rax, 10
  push rax
  .loc 1 211
  .loc 1 211
  .loc 1 211
  lea rax, [rbp - 24]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 212
  jmp .L..10
  jmp .L.end.32
.L.else.32:
.L.end.32:
  .loc 1 215
  .loc 1 215
  .loc 1 215
  .loc 1 215
  mov rax, 41
  push rax
  .loc 1 215
  .loc 1 215
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.36
  .loc 1 217
  .loc 1 217
  .loc 1 217
  .loc 1 217
  lea rax, [rbp - 16]
  push rax
  .loc 1 217
  .loc 1 217
  lea rax, [rip + priorityBase]
  pop rdi
  mov [rdi], rax
  .loc 1 217
  .loc 1 217
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 217
  .loc 1 217
  .loc 1 217
  .loc 1 217
  mov rax, 10
  push rax
  .loc 1 217
  .loc 1 217
  .loc 1 217
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 220
  .loc 1 220
  .loc 1 220
  .loc 1 220
  mov rax, 0
  push rax
  .loc 1 220
  .loc 1 220
  lea rax, [rip + priorityBase]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L.else.37
  .loc 1 222
  .loc 1 222
  .loc 1 222
  lea rax, [rip + error_pos]
  push rax
  .loc 1 222
  .loc 1 222
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 223
  .loc 1 223
  .loc 1 223
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.37
.L.else.37:
.L.end.37:
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  mov rax, 1
  push rax
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  mov rax, 1
  push rax
  .loc 1 227
  .loc 1 227
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 227
  .loc 1 227
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  sete al
  movzx rax, al
  cmp rax, 0
  je .L.false.39
  .loc 1 227
  .loc 1 227
  .loc 1 227
  mov rax, 41
  push rax
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  mov rax, 1
  push rax
  .loc 1 227
  .loc 1 227
  .loc 1 227
  .loc 1 227
  mov rax, 1
  push rax
  .loc 1 227
  .loc 1 227
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 227
  .loc 1 227
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.39
  mov rax, 1
  jmp .L.end.39
.L.false.39:
  mov rax, 0
.L.end.39:
  cmp rax, 0
  je .L.else.38
  .loc 1 229
  .loc 1 229
  .loc 1 229
  lea rax, [rip + error_pos]
  push rax
  .loc 1 229
  .loc 1 229
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 230
  .loc 1 230
  .loc 1 230
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.38
.L.else.38:
.L.end.38:
  .loc 1 234
  .loc 1 234
  .loc 1 234
  .loc 1 234
  .loc 1 234
  .loc 1 234
  mov rax, 1
  push rax
  .loc 1 234
  .loc 1 234
  lea rax, [rbp - 56]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  push rax
  .loc 1 234
  .loc 1 234
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.40
  .loc 1 236
  .loc 1 236
  .loc 1 236
  .loc 1 236
  .loc 1 236
  .loc 1 236
  .loc 1 236
  .loc 1 236
  mov rax, 4
  push rax
  .loc 1 236
  .loc 1 236
  lea rax, [rip + numSize]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 236
  .loc 1 236
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 236
  .loc 1 236
  lea rax, [rbp - 60]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 237
  .loc 1 237
  .loc 1 237
  .loc 1 237
  .loc 1 237
  mov rax, -1
  push rax
  .loc 1 237
  .loc 1 237
  .loc 1 237
  lea rax, [rbp - 8]
  push rax
  .loc 1 237
  .loc 1 237
  lea rax, [rip + numSize]
  pop rdi
  mov [rdi], rax
  .loc 1 237
  .loc 1 237
  lea rax, [rbp - 8]
  mov rax, [rax]
  push rax
  .loc 1 237
  .loc 1 237
  .loc 1 237
  .loc 1 237
  mov rax, 1
  push rax
  .loc 1 237
  .loc 1 237
  .loc 1 237
  lea rax, [rbp - 8]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.end.40
.L.else.40:
.L.end.40:
  .loc 1 239
  jmp .L..10
  jmp .L.end.36
.L.else.36:
.L.end.36:
  .loc 1 249
  .loc 1 249
  .loc 1 249
  .loc 1 249
  mov rax, 43
  push rax
  .loc 1 249
  .loc 1 249
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.41
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 0
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.45
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  sete al
  movzx rax, al
  cmp rax, 0
  je .L.false.45
  mov rax, 1
  jmp .L.end.45
.L.false.45:
  mov rax, 0
.L.end.45:
  cmp rax, 0
  je .L.false.44
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 40
  push rax
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.44
  mov rax, 1
  jmp .L.end.44
.L.false.44:
  mov rax, 0
.L.end.44:
  cmp rax, 0
  je .L.false.43
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 41
  push rax
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  .loc 1 252
  .loc 1 252
  mov rax, 1
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 252
  .loc 1 252
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.43
  mov rax, 1
  jmp .L.end.43
.L.false.43:
  mov rax, 0
.L.end.43:
  cmp rax, 0
  je .L.else.42
  .loc 1 254
  .loc 1 254
  .loc 1 254
  lea rax, [rip + error_pos]
  push rax
  .loc 1 254
  .loc 1 254
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 255
  .loc 1 255
  .loc 1 255
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.42
.L.else.42:
.L.end.42:
  .loc 1 257
  .loc 1 257
  .loc 1 257
  .loc 1 257
  mov rax, 0
  movsxd rax, eax
  push rax
  .loc 1 257
  .loc 1 257
  lea rax, [rbp - 60]
  mov eax, [rax]
  push rax
  .loc 1 257
  .loc 1 257
  lea rax, [rbp - 52]
  mov eax, [rax]
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call setOP
  .loc 1 258
  .loc 1 258
  lea rax, [rbp - 60]
  push rax
  .loc 1 258
  .loc 1 258
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 259
  jmp .L..10
  jmp .L.end.41
.L.else.41:
.L.end.41:
  .loc 1 263
  .loc 1 263
  .loc 1 263
  .loc 1 263
  mov rax, 45
  push rax
  .loc 1 263
  .loc 1 263
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.46
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 0
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.50
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  sete al
  movzx rax, al
  cmp rax, 0
  je .L.false.50
  mov rax, 1
  jmp .L.end.50
.L.false.50:
  mov rax, 0
.L.end.50:
  cmp rax, 0
  je .L.false.49
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 40
  push rax
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.49
  mov rax, 1
  jmp .L.end.49
.L.false.49:
  mov rax, 0
.L.end.49:
  cmp rax, 0
  je .L.false.48
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 41
  push rax
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  .loc 1 266
  .loc 1 266
  mov rax, 1
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 266
  .loc 1 266
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.48
  mov rax, 1
  jmp .L.end.48
.L.false.48:
  mov rax, 0
.L.end.48:
  cmp rax, 0
  je .L.else.47
  .loc 1 268
  .loc 1 268
  .loc 1 268
  lea rax, [rip + error_pos]
  push rax
  .loc 1 268
  .loc 1 268
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 269
  .loc 1 269
  .loc 1 269
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.47
.L.else.47:
.L.end.47:
  .loc 1 271
  .loc 1 271
  .loc 1 271
  .loc 1 271
  mov rax, 1
  movsxd rax, eax
  push rax
  .loc 1 271
  .loc 1 271
  lea rax, [rbp - 60]
  mov eax, [rax]
  push rax
  .loc 1 271
  .loc 1 271
  lea rax, [rbp - 52]
  mov eax, [rax]
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call setOP
  .loc 1 272
  .loc 1 272
  lea rax, [rbp - 60]
  push rax
  .loc 1 272
  .loc 1 272
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 273
  jmp .L..10
  jmp .L.end.46
.L.else.46:
.L.end.46:
  .loc 1 277
  .loc 1 277
  .loc 1 277
  .loc 1 277
  mov rax, 47
  push rax
  .loc 1 277
  .loc 1 277
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.51
  .loc 1 280
  .loc 1 280
  .loc 1 280
  .loc 1 280
  .loc 1 280
  mov rax, 0
  push rax
  .loc 1 280
  .loc 1 280
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.52
  .loc 1 282
  .loc 1 282
  .loc 1 282
  lea rax, [rip + error_pos]
  push rax
  .loc 1 282
  .loc 1 282
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 283
  .loc 1 283
  .loc 1 283
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.52
.L.else.52:
.L.end.52:
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  mov rax, 1
  push rax
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  mov rax, 1
  push rax
  .loc 1 286
  .loc 1 286
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 286
  .loc 1 286
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  sete al
  movzx rax, al
  cmp rax, 0
  je .L.false.54
  .loc 1 286
  .loc 1 286
  .loc 1 286
  mov rax, 41
  push rax
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  mov rax, 1
  push rax
  .loc 1 286
  .loc 1 286
  .loc 1 286
  .loc 1 286
  mov rax, 1
  push rax
  .loc 1 286
  .loc 1 286
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 286
  .loc 1 286
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.54
  mov rax, 1
  jmp .L.end.54
.L.false.54:
  mov rax, 0
.L.end.54:
  cmp rax, 0
  je .L.else.53
  .loc 1 288
  .loc 1 288
  .loc 1 288
  lea rax, [rip + error_pos]
  push rax
  .loc 1 288
  .loc 1 288
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 289
  .loc 1 289
  .loc 1 289
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.53
.L.else.53:
.L.end.53:
  .loc 1 293
  .loc 1 293
  .loc 1 293
  .loc 1 293
  mov rax, 2
  movsxd rax, eax
  push rax
  .loc 1 293
  .loc 1 293
  lea rax, [rbp - 60]
  mov eax, [rax]
  push rax
  .loc 1 293
  .loc 1 293
  lea rax, [rbp - 52]
  mov eax, [rax]
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call setOP
  .loc 1 296
  .loc 1 296
  lea rax, [rbp - 60]
  push rax
  .loc 1 296
  .loc 1 296
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 297
  jmp .L..10
  jmp .L.end.51
.L.else.51:
.L.end.51:
  .loc 1 301
  .loc 1 301
  .loc 1 301
  .loc 1 301
  mov rax, 42
  push rax
  .loc 1 301
  .loc 1 301
  lea rax, [rbp - 37]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.55
  .loc 1 304
  .loc 1 304
  .loc 1 304
  .loc 1 304
  .loc 1 304
  mov rax, 0
  push rax
  .loc 1 304
  .loc 1 304
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.56
  .loc 1 306
  .loc 1 306
  .loc 1 306
  lea rax, [rip + error_pos]
  push rax
  .loc 1 306
  .loc 1 306
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 307
  .loc 1 307
  .loc 1 307
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.56
.L.else.56:
.L.end.56:
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  mov rax, 1
  push rax
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  mov rax, 1
  push rax
  .loc 1 310
  .loc 1 310
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 310
  .loc 1 310
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  push rax
  pop rdi
  mov rax, 0
  call isdigit
  cmp rax, 0
  sete al
  movzx rax, al
  cmp rax, 0
  je .L.false.58
  .loc 1 310
  .loc 1 310
  .loc 1 310
  mov rax, 41
  push rax
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  mov rax, 1
  push rax
  .loc 1 310
  .loc 1 310
  .loc 1 310
  .loc 1 310
  mov rax, 1
  push rax
  .loc 1 310
  .loc 1 310
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 310
  .loc 1 310
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.58
  mov rax, 1
  jmp .L.end.58
.L.false.58:
  mov rax, 0
.L.end.58:
  cmp rax, 0
  je .L.else.57
  .loc 1 312
  .loc 1 312
  .loc 1 312
  lea rax, [rip + error_pos]
  push rax
  .loc 1 312
  .loc 1 312
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 313
  .loc 1 313
  .loc 1 313
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.57
.L.else.57:
.L.end.57:
  .loc 1 317
  .loc 1 317
  .loc 1 317
  .loc 1 317
  mov rax, 3
  movsxd rax, eax
  push rax
  .loc 1 317
  .loc 1 317
  lea rax, [rbp - 60]
  mov eax, [rax]
  push rax
  .loc 1 317
  .loc 1 317
  lea rax, [rbp - 52]
  mov eax, [rax]
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call setOP
  .loc 1 320
  .loc 1 320
  lea rax, [rbp - 60]
  push rax
  .loc 1 320
  .loc 1 320
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 321
  jmp .L..10
  jmp .L.end.55
.L.else.55:
.L.end.55:
  .loc 1 325
  .loc 1 325
  lea rax, [rip + error_pos]
  push rax
  .loc 1 325
  .loc 1 325
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 326
  .loc 1 326
  .loc 1 326
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
.L..10:
  .loc 1 174
  .loc 1 174
  .loc 1 174
  .loc 1 174
  mov rax, -1
  push rax
  .loc 1 174
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 48]
  push rax
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 52]
  pop rdi
  mov [rdi], rax
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 48]
  mov rax, [rax]
  push rax
  .loc 1 174
  .loc 1 174
  .loc 1 174
  .loc 1 174
  mov rax, 1
  push rax
  .loc 1 174
  .loc 1 174
  .loc 1 174
  lea rax, [rbp - 48]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.27
.L..9:
  .loc 1 331
  .loc 1 331
  .loc 1 331
  .loc 1 331
  mov rax, 0
  push rax
  .loc 1 331
  .loc 1 331
  lea rax, [rip + priorityBase]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.else.59
  .loc 1 333
  .loc 1 333
  .loc 1 333
  lea rax, [rip + error_pos]
  push rax
  .loc 1 333
  .loc 1 333
  .loc 1 333
  .loc 1 333
  mov rax, 1
  push rax
  .loc 1 333
  .loc 1 333
  lea rax, [rbp - 56]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 334
  .loc 1 334
  .loc 1 334
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.59
.L.else.59:
.L.end.59:
  .loc 1 337
  .loc 1 337
  .loc 1 337
  .loc 1 337
  .loc 1 337
  .loc 1 337
  mov rax, 1
  push rax
  .loc 1 337
  .loc 1 337
  lea rax, [rip + opSize]
  mov eax, [rax]
  pop rdi
  add eax, edi
  push rax
  .loc 1 337
  .loc 1 337
  lea rax, [rip + numSize]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.else.60
  .loc 1 339
  .loc 1 339
  .loc 1 339
  lea rax, [rip + error_pos]
  push rax
  .loc 1 339
  .loc 1 339
  .loc 1 339
  .loc 1 339
  mov rax, 1
  push rax
  .loc 1 339
  .loc 1 339
  lea rax, [rbp - 56]
  mov eax, [rax]
  pop rdi
  sub eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 340
  .loc 1 340
  .loc 1 340
  mov rax, 0
  movsxd rax, eax
  jmp .L.return.parse_input
  jmp .L.end.60
.L.else.60:
.L.end.60:
  .loc 1 342
  .loc 1 342
  .loc 1 342
  mov rax, 1
  movsxd rax, eax
  jmp .L.return.parse_input
.L.return.parse_input:
  mov rsp, rbp
  pop rbp
  ret
  .globl remove_space
  .text
remove_space:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  mov [rbp - 8], rdi
  .loc 1 142
  .loc 1 142
  .loc 1 142
  .loc 1 142
  lea rax, [rbp - 40]
  push rax
  .loc 1 142
  .loc 1 142
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  mov [rdi], rax
  .loc 1 144
  .loc 1 144
  .loc 1 144
  lea rax, [rbp - 32]
  push rax
  .loc 1 144
  .loc 1 144
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  mov [rdi], rax
  .loc 1 147
.L.begin.61:
  .loc 1 147
  .loc 1 147
  lea rax, [rbp - 40]
  mov rax, [rax]
  movsx eax, BYTE PTR [rax]
  cmp rax, 0
  je .L..7
  .loc 1 150
  .loc 1 150
  .loc 1 150
  .loc 1 150
  .loc 1 150
  .loc 1 150
  mov rax, 32
  push rax
  .loc 1 150
  .loc 1 150
  .loc 1 150
  lea rax, [rbp - 40]
  mov rax, [rax]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.63
  .loc 1 150
  .loc 1 150
  .loc 1 150
  mov rax, 10
  push rax
  .loc 1 150
  .loc 1 150
  .loc 1 150
  lea rax, [rbp - 40]
  mov rax, [rax]
  movsx eax, BYTE PTR [rax]
  pop rdi
  cmp eax, edi
  setne al
  movzb rax, al
  cmp rax, 0
  je .L.false.63
  mov rax, 1
  jmp .L.end.63
.L.false.63:
  mov rax, 0
.L.end.63:
  cmp rax, 0
  je .L.else.62
  .loc 1 152
  .loc 1 152
  .loc 1 152
  .loc 1 152
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 152
  .loc 1 152
  .loc 1 152
  lea rax, [rbp - 40]
  mov rax, [rax]
  movsx eax, BYTE PTR [rax]
  pop rdi
  mov [rdi], al
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  mov rax, 1
  push rax
  .loc 1 153
  .loc 1 153
  mov rax, -1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 153
  .loc 1 153
  .loc 1 153
  lea rax, [rbp - 24]
  push rax
  .loc 1 153
  .loc 1 153
  lea rax, [rbp - 32]
  pop rdi
  mov [rdi], rax
  .loc 1 153
  .loc 1 153
  lea rax, [rbp - 24]
  mov rax, [rax]
  push rax
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  .loc 1 153
  mov rax, 1
  push rax
  .loc 1 153
  .loc 1 153
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 153
  .loc 1 153
  .loc 1 153
  lea rax, [rbp - 24]
  mov rax, [rax]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  pop rdi
  mov [rdi], rax
  pop rdi
  add rax, rdi
  jmp .L.end.62
.L.else.62:
.L.end.62:
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  mov rax, 1
  push rax
  .loc 1 156
  .loc 1 156
  mov rax, -1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 156
  .loc 1 156
  .loc 1 156
  lea rax, [rbp - 16]
  push rax
  .loc 1 156
  .loc 1 156
  lea rax, [rbp - 40]
  pop rdi
  mov [rdi], rax
  .loc 1 156
  .loc 1 156
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  .loc 1 156
  mov rax, 1
  push rax
  .loc 1 156
  .loc 1 156
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 156
  .loc 1 156
  .loc 1 156
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  pop rdi
  mov [rdi], rax
  pop rdi
  add rax, rdi
.L..8:
  jmp .L.begin.61
.L..7:
  .loc 1 160
  .loc 1 160
  .loc 1 160
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 160
  .loc 1 160
  mov rax, 0
  movsx eax, al
  pop rdi
  mov [rdi], al
.L.return.remove_space:
  mov rsp, rbp
  pop rbp
  ret
  .globl initialize
  .text
initialize:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  .loc 1 132
  .loc 1 132
  .loc 1 132
  .loc 1 132
  .loc 1 132
  .loc 1 132
  .loc 1 132
  .loc 1 132
  mov rax, 1
  push rax
  .loc 1 132
  .loc 1 132
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 132
  .loc 1 132
  lea rax, [rip + input_str]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 132
  .loc 1 132
  mov rax, 0
  movsx eax, al
  pop rdi
  mov [rdi], al
  .loc 1 133
  .loc 1 133
  lea rax, [rip + numSize]
  push rax
  .loc 1 133
  .loc 1 133
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 134
  .loc 1 134
  lea rax, [rip + opSize]
  push rax
  .loc 1 134
  .loc 1 134
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 135
  .loc 1 135
  lea rax, [rip + priorityBase]
  push rax
  .loc 1 135
  .loc 1 135
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 136
  .loc 1 136
  lea rax, [rip + error_pos]
  push rax
  .loc 1 136
  .loc 1 136
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.return.initialize:
  mov rsp, rbp
  pop rbp
  ret
  .globl main
  .text
main:
  push rbp
  mov rbp, rsp
  sub rsp, 0
  .loc 1 81
  .loc 1 81
.L.begin.64:
  .loc 1 81
  mov rax, 1
  cmp rax, 0
  je .L..0
  .loc 1 84
  .loc 1 84
  .loc 1 84
  mov rax, 0
  call initialize
  .loc 1 87
  .loc 1 87
  .loc 1 87
  .loc 1 87
  lea rax, [rip + .L..2]
  push rax
  pop rdi
  mov rax, 0
  call printf
  .loc 1 88
  .loc 1 88
  .loc 1 88
  .loc 1 88
  lea rax, [rip + .L..3]
  push rax
  pop rdi
  mov rax, 0
  call printf
  .loc 1 89
  .loc 1 89
  .loc 1 89
  .loc 1 89
  lea rax, [rip + .L..4]
  push rax
  pop rdi
  mov rax, 0
  call printf
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  mov rax, 0
  movsxd rax, eax
  push rax
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  lea rax, [rip + input_str]
  push rax
  pop rdi
  mov rax, 0
  call receive_input
  pop rdi
  cmp rax, rdi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.65
  .loc 1 92
  .loc 1 92
  jmp .L..0
  jmp .L.end.65
.L.else.65:
.L.end.65:
  .loc 1 96
  .loc 1 96
  .loc 1 96
  .loc 1 96
  lea rax, [rip + input_str]
  push rax
  pop rdi
  mov rax, 0
  call remove_space
  .loc 1 99
  .loc 1 99
  .loc 1 99
  .loc 1 99
  mov rax, 0
  push rax
  .loc 1 99
  .loc 1 99
  .loc 1 99
  .loc 1 99
  lea rax, [rip + input_str]
  push rax
  pop rdi
  mov rax, 0
  call strlen
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.66
  .loc 1 101
  .loc 1 101
  .loc 1 101
  .loc 1 101
  .loc 1 101
  lea rax, [rip + .L..5]
  push rax
  pop rdi
  mov rax, 0
  call printf
  .loc 1 102
  jmp .L..1
  jmp .L.end.66
.L.else.66:
.L.end.66:
  .loc 1 106
  .loc 1 106
  .loc 1 106
  .loc 1 106
  mov rax, 0
  push rax
  .loc 1 106
  .loc 1 106
  mov rax, 0
  call parse_input
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.67
  .loc 1 108
  .loc 1 108
  .loc 1 108
  mov rax, 0
  call report_error
  .loc 1 109
  jmp .L..1
  jmp .L.end.67
.L.else.67:
.L.end.67:
  .loc 1 113
  .loc 1 113
  .loc 1 113
  .loc 1 113
  mov rax, 0
  push rax
  .loc 1 113
  .loc 1 113
  mov rax, 0
  call perform_calculation
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.68
  .loc 1 115
  .loc 1 115
  .loc 1 115
  mov rax, 0
  call report_error
  .loc 1 116
  jmp .L..1
  jmp .L.end.68
.L.else.68:
.L.end.68:
  .loc 1 120
  .loc 1 120
  .loc 1 120
  .loc 1 120
  lea rax, [rip + .L..6]
  push rax
  .loc 1 120
  .loc 1 120
  .loc 1 120
  .loc 1 120
  .loc 1 120
  .loc 1 120
  mov rax, 4
  push rax
  .loc 1 120
  .loc 1 120
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 120
  .loc 1 120
  lea rax, [rip + nums]
  pop rdi
  add rax, rdi
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call printf
.L..1:
  jmp .L.begin.64
.L..0:
  .loc 1 123
  .loc 1 123
  .loc 1 123
  mov rax, 0
  jmp .L.return.main
.L.return.main:
  mov rsp, rbp
  pop rbp
  ret
