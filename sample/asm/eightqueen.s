.intel_syntax noprefix
.file 1 ".\sample\EightQueen.c"
  .data
  .globl .L..19
.L..19:
  .byte 37
  .byte 100
  .byte 32
  .byte 115
  .byte 111
  .byte 108
  .byte 117
  .byte 116
  .byte 105
  .byte 111
  .byte 110
  .byte 115
  .byte 32
  .byte 119
  .byte 101
  .byte 114
  .byte 101
  .byte 32
  .byte 102
  .byte 111
  .byte 117
  .byte 110
  .byte 100
  .byte 46
  .byte 10
  .byte 0
  .data
  .globl .L..12
.L..12:
  .byte 10
  .byte 0
  .data
  .globl .L..11
.L..11:
  .byte 10
  .byte 0
  .data
  .globl .L..10
.L..10:
  .byte 42
  .byte 0
  .data
  .globl .L..9
.L..9:
  .byte 81
  .byte 0
  .data
  .globl .L..4
.L..4:
  .byte 78
  .byte 111
  .byte 46
  .byte 37
  .byte 100
  .byte 10
  .byte 0
  .data
  .globl count
count:
  .zero 4
  .globl main
  .text
main:
  push rbp
  mov rbp, rsp
  sub rsp, 288
  .loc 1 107
  .loc 1 107
  .loc 1 107
  lea rax, [rip + count]
  push rax
  .loc 1 107
  .loc 1 107
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 115
  .loc 1 118
  .loc 1 121
  .loc 1 121
  .loc 1 121
  .loc 1 121
  lea rax, [rbp - 256]
  push rax
  pop rdi
  mov rax, 0
  call init_board
  .loc 1 124
  .loc 1 124
  .loc 1 124
  .loc 1 124
  lea rax, [rbp - 288]
  push rax
  .loc 1 124
  lea rax, [rbp - 256]
  push rax
  .loc 1 124
  mov rax, 0
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call set_queen
  .loc 1 126
  .loc 1 126
  .loc 1 126
  lea rax, [rip + .L..19]
  push rax
  .loc 1 126
  lea rax, [rip + count]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call printf
  .loc 1 128
  .loc 1 128
  .loc 1 128
  mov rax, 0
  jmp .L.return.main
.L.return.main:
  mov rsp, rbp
  pop rbp
  ret
  .globl set_queen
  .text
set_queen:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  mov [rbp - 8], rdi
  mov [rbp - 16], rsi
  mov [rbp - 20], edx
  .loc 1 79
  .loc 1 79
  .loc 1 79
  .loc 1 79
  .loc 1 79
  mov rax, 8
  push rax
  .loc 1 79
  .loc 1 79
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.1
  .loc 1 81
  .loc 1 81
  .loc 1 81
  .loc 1 81
  .loc 1 81
  .loc 1 81
  mov rax, -1
  push rax
  .loc 1 81
  .loc 1 81
  .loc 1 81
  lea rax, [rbp - 48]
  push rax
  .loc 1 81
  .loc 1 81
  lea rax, [rip + count]
  pop rdi
  mov [rdi], rax
  .loc 1 81
  .loc 1 81
  lea rax, [rbp - 48]
  mov rax, [rax]
  push rax
  .loc 1 81
  .loc 1 81
  .loc 1 81
  .loc 1 81
  mov rax, 1
  push rax
  .loc 1 81
  .loc 1 81
  .loc 1 81
  lea rax, [rbp - 48]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  .loc 1 82
  .loc 1 82
  .loc 1 82
  .loc 1 82
  lea rax, [rbp - 8]
  mov rax, [rax]
  push rax
  pop rdi
  mov rax, 0
  call print_board
  .loc 1 83
  .loc 1 83
  .loc 1 83
  mov rax, 0
  jmp .L.return.set_queen
  jmp .L.end.1
.L.else.1:
.L.end.1:
  .loc 1 87
  .loc 1 87
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 36]
  push rax
  .loc 1 87
  .loc 1 87
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.2:
  .loc 1 87
  .loc 1 87
  .loc 1 87
  mov rax, 8
  push rax
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 36]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..17
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  mov rax, 0
  push rax
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  mov rax, 4
  push rax
  .loc 1 90
  .loc 1 90
  lea rax, [rbp - 36]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  .loc 1 90
  mov rax, 32
  push rax
  .loc 1 90
  .loc 1 90
  lea rax, [rbp - 20]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 90
  .loc 1 90
  lea rax, [rbp - 16]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.3
  .loc 1 93
  .loc 1 93
  .loc 1 93
  .loc 1 93
  .loc 1 93
  .loc 1 93
  .loc 1 93
  .loc 1 93
  mov rax, 4
  push rax
  .loc 1 93
  .loc 1 93
  lea rax, [rbp - 20]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 93
  .loc 1 93
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  push rax
  .loc 1 93
  .loc 1 93
  lea rax, [rbp - 36]
  mov eax, [rax]
  pop rdi
  mov [rdi], eax
  .loc 1 95
  .loc 1 95
  .loc 1 95
  .loc 1 95
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 95
  lea rax, [rbp - 20]
  mov eax, [rax]
  push rax
  .loc 1 95
  lea rax, [rbp - 36]
  mov eax, [rax]
  push rax
  .loc 1 95
  mov rax, 1
  push rax
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call change_board
  .loc 1 97
  .loc 1 97
  .loc 1 97
  .loc 1 97
  lea rax, [rbp - 8]
  mov rax, [rax]
  push rax
  .loc 1 97
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 97
  .loc 1 97
  .loc 1 97
  mov rax, 1
  push rax
  .loc 1 97
  .loc 1 97
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  add eax, edi
  push rax
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call set_queen
  .loc 1 99
  .loc 1 99
  .loc 1 99
  .loc 1 99
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 99
  lea rax, [rbp - 20]
  mov eax, [rax]
  push rax
  .loc 1 99
  lea rax, [rbp - 36]
  mov eax, [rax]
  push rax
  .loc 1 99
  .loc 1 99
  .loc 1 99
  mov rax, 1
  neg rax
  push rax
  pop rcx
  pop rdx
  pop rsi
  pop rdi
  mov rax, 0
  call change_board
  jmp .L.end.3
.L.else.3:
.L.end.3:
.L..18:
  .loc 1 87
  .loc 1 87
  .loc 1 87
  .loc 1 87
  mov rax, -1
  push rax
  .loc 1 87
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 32]
  push rax
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 36]
  pop rdi
  mov [rdi], rax
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 87
  .loc 1 87
  .loc 1 87
  .loc 1 87
  mov rax, 1
  push rax
  .loc 1 87
  .loc 1 87
  .loc 1 87
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.2
.L..17:
.L.return.set_queen:
  mov rsp, rbp
  pop rbp
  ret
  .globl change_board
  .text
change_board:
  push rbp
  mov rbp, rsp
  sub rsp, 144
  mov [rbp - 8], rdi
  mov [rbp - 12], esi
  mov [rbp - 16], edx
  mov [rbp - 20], ecx
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 2
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 2
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 3
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 3
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 4
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 4
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 5
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 5
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 1
  neg rax
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 6
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 6
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 7
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 4
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  .loc 1 59
  mov rax, 8
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 7
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 59
  .loc 1 59
  mov rax, 1
  pop rdi
  mov [rdi], eax
  .loc 1 61
  .loc 1 61
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 68]
  push rax
  .loc 1 61
  .loc 1 61
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.4:
  .loc 1 61
  .loc 1 61
  .loc 1 61
  mov rax, 8
  push rax
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 68]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..13
  .loc 1 63
  .loc 1 63
  .loc 1 63
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 52]
  push rax
  .loc 1 63
  .loc 1 63
  mov rax, 1
  pop rdi
  mov [rdi], eax
.L.begin.5:
  .loc 1 63
  .loc 1 63
  .loc 1 63
  mov rax, 8
  push rax
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..15
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  lea rax, [rbp - 40]
  push rax
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  mov rax, 4
  push rax
  .loc 1 65
  .loc 1 65
  mov rax, 0
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  .loc 1 65
  mov rax, 8
  push rax
  .loc 1 65
  .loc 1 65
  lea rax, [rbp - 68]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 65
  .loc 1 65
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  mov eax, [rax]
  push rax
  .loc 1 65
  .loc 1 65
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  imul eax, edi
  push rax
  .loc 1 65
  .loc 1 65
  lea rax, [rbp - 12]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 66
  .loc 1 66
  .loc 1 66
  lea rax, [rbp - 36]
  push rax
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  mov rax, 4
  push rax
  .loc 1 66
  .loc 1 66
  mov rax, 1
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  .loc 1 66
  mov rax, 8
  push rax
  .loc 1 66
  .loc 1 66
  lea rax, [rbp - 68]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 66
  .loc 1 66
  lea rax, [rbp - 132]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  mov eax, [rax]
  push rax
  .loc 1 66
  .loc 1 66
  lea rax, [rbp - 52]
  mov eax, [rax]
  pop rdi
  imul eax, edi
  push rax
  .loc 1 66
  .loc 1 66
  lea rax, [rbp - 16]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  .loc 1 68
  .loc 1 68
  .loc 1 68
  .loc 1 68
  .loc 1 68
  .loc 1 68
  .loc 1 68
  mov rax, 0
  push rax
  .loc 1 68
  .loc 1 68
  lea rax, [rbp - 40]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  jne .L.true.9
  .loc 1 68
  .loc 1 68
  .loc 1 68
  lea rax, [rbp - 40]
  mov eax, [rax]
  push rax
  .loc 1 68
  .loc 1 68
  mov rax, 8
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  cmp rax, 0
  jne .L.true.9
  mov rax, 0
  jmp .L.end.9
.L.true.9:
  mov rax, 1
.L.end.9:
  cmp rax, 0
  jne .L.true.8
  .loc 1 68
  .loc 1 68
  .loc 1 68
  mov rax, 0
  push rax
  .loc 1 68
  .loc 1 68
  lea rax, [rbp - 36]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  jne .L.true.8
  mov rax, 0
  jmp .L.end.8
.L.true.8:
  mov rax, 1
.L.end.8:
  cmp rax, 0
  jne .L.true.7
  .loc 1 68
  .loc 1 68
  .loc 1 68
  lea rax, [rbp - 36]
  mov eax, [rax]
  push rax
  .loc 1 68
  .loc 1 68
  mov rax, 8
  pop rdi
  cmp eax, edi
  setle al
  movzb rax, al
  cmp rax, 0
  jne .L.true.7
  mov rax, 0
  jmp .L.end.7
.L.true.7:
  mov rax, 1
.L.end.7:
  cmp rax, 0
  je .L.else.6
  .loc 1 69
  jmp .L..16
  jmp .L.end.6
.L.else.6:
.L.end.6:
  .loc 1 71
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 32]
  push rax
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  mov rax, 4
  push rax
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 36]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  mov rax, 32
  push rax
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 40]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  pop rdi
  mov [rdi], rax
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 71
  .loc 1 71
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 20]
  mov eax, [rax]
  push rax
  .loc 1 71
  .loc 1 71
  .loc 1 71
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
.L..16:
  .loc 1 63
  .loc 1 63
  .loc 1 63
  .loc 1 63
  mov rax, -1
  push rax
  .loc 1 63
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 48]
  push rax
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 52]
  pop rdi
  mov [rdi], rax
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 48]
  mov rax, [rax]
  push rax
  .loc 1 63
  .loc 1 63
  .loc 1 63
  .loc 1 63
  mov rax, 1
  push rax
  .loc 1 63
  .loc 1 63
  .loc 1 63
  lea rax, [rbp - 48]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.5
.L..15:
.L..14:
  .loc 1 61
  .loc 1 61
  .loc 1 61
  .loc 1 61
  mov rax, -1
  push rax
  .loc 1 61
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 64]
  push rax
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 68]
  pop rdi
  mov [rdi], rax
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 64]
  mov rax, [rax]
  push rax
  .loc 1 61
  .loc 1 61
  .loc 1 61
  .loc 1 61
  mov rax, 1
  push rax
  .loc 1 61
  .loc 1 61
  .loc 1 61
  lea rax, [rbp - 64]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.4
.L..13:
.L.return.change_board:
  mov rsp, rbp
  pop rbp
  ret
  .globl print_board
  .text
print_board:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  mov [rbp - 8], rdi
  .loc 1 32
  .loc 1 32
  .loc 1 32
  .loc 1 32
  lea rax, [rip + .L..4]
  push rax
  .loc 1 32
  lea rax, [rip + count]
  mov eax, [rax]
  push rax
  pop rsi
  pop rdi
  mov rax, 0
  call printf
  .loc 1 33
  .loc 1 33
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 36]
  push rax
  .loc 1 33
  .loc 1 33
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.10:
  .loc 1 33
  .loc 1 33
  .loc 1 33
  mov rax, 8
  push rax
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 36]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..5
  .loc 1 35
  .loc 1 35
  .loc 1 35
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 20]
  push rax
  .loc 1 35
  .loc 1 35
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.11:
  .loc 1 35
  .loc 1 35
  .loc 1 35
  mov rax, 8
  push rax
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..7
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  .loc 1 38
  mov rax, 4
  push rax
  .loc 1 38
  .loc 1 38
  lea rax, [rbp - 36]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 38
  .loc 1 38
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  mov eax, [rax]
  push rax
  .loc 1 38
  .loc 1 38
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  sete al
  movzb rax, al
  cmp rax, 0
  je .L.else.12
  .loc 1 40
  .loc 1 40
  .loc 1 40
  .loc 1 40
  lea rax, [rip + .L..9]
  push rax
  pop rdi
  mov rax, 0
  call printf
  jmp .L.end.12
.L.else.12:
  .loc 1 45
  .loc 1 45
  .loc 1 45
  .loc 1 45
  lea rax, [rip + .L..10]
  push rax
  pop rdi
  mov rax, 0
  call printf
.L.end.12:
.L..8:
  .loc 1 35
  .loc 1 35
  .loc 1 35
  .loc 1 35
  mov rax, -1
  push rax
  .loc 1 35
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 16]
  push rax
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 20]
  pop rdi
  mov [rdi], rax
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 35
  .loc 1 35
  .loc 1 35
  .loc 1 35
  mov rax, 1
  push rax
  .loc 1 35
  .loc 1 35
  .loc 1 35
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.11
.L..7:
  .loc 1 48
  .loc 1 48
  .loc 1 48
  lea rax, [rip + .L..11]
  push rax
  pop rdi
  mov rax, 0
  call printf
.L..6:
  .loc 1 33
  .loc 1 33
  .loc 1 33
  .loc 1 33
  mov rax, -1
  push rax
  .loc 1 33
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 32]
  push rax
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 36]
  pop rdi
  mov [rdi], rax
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 33
  .loc 1 33
  .loc 1 33
  .loc 1 33
  mov rax, 1
  push rax
  .loc 1 33
  .loc 1 33
  .loc 1 33
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.10
.L..5:
  .loc 1 50
  .loc 1 50
  .loc 1 50
  lea rax, [rip + .L..12]
  push rax
  pop rdi
  mov rax, 0
  call printf
.L.return.print_board:
  mov rsp, rbp
  pop rbp
  ret
  .globl init_board
  .text
init_board:
  push rbp
  mov rbp, rsp
  sub rsp, 48
  mov [rbp - 8], rdi
  .loc 1 20
  .loc 1 20
  .loc 1 20
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 36]
  push rax
  .loc 1 20
  .loc 1 20
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.13:
  .loc 1 20
  .loc 1 20
  .loc 1 20
  mov rax, 8
  push rax
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 36]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..0
  .loc 1 22
  .loc 1 22
  .loc 1 22
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 20]
  push rax
  .loc 1 22
  .loc 1 22
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L.begin.14:
  .loc 1 22
  .loc 1 22
  .loc 1 22
  mov rax, 8
  push rax
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 20]
  mov eax, [rax]
  pop rdi
  cmp eax, edi
  setl al
  movzb rax, al
  cmp rax, 0
  je .L..2
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  mov rax, 4
  push rax
  .loc 1 24
  .loc 1 24
  lea rax, [rbp - 20]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  .loc 1 24
  mov rax, 32
  push rax
  .loc 1 24
  .loc 1 24
  lea rax, [rbp - 36]
  mov eax, [rax]
  movsxd rax, eax
  pop rdi
  imul rax, rdi
  push rax
  .loc 1 24
  .loc 1 24
  lea rax, [rbp - 8]
  mov rax, [rax]
  pop rdi
  add rax, rdi
  pop rdi
  add rax, rdi
  push rax
  .loc 1 24
  .loc 1 24
  mov rax, 0
  pop rdi
  mov [rdi], eax
.L..3:
  .loc 1 22
  .loc 1 22
  .loc 1 22
  .loc 1 22
  mov rax, -1
  push rax
  .loc 1 22
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 16]
  push rax
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 20]
  pop rdi
  mov [rdi], rax
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 16]
  mov rax, [rax]
  push rax
  .loc 1 22
  .loc 1 22
  .loc 1 22
  .loc 1 22
  mov rax, 1
  push rax
  .loc 1 22
  .loc 1 22
  .loc 1 22
  lea rax, [rbp - 16]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.14
.L..2:
.L..1:
  .loc 1 20
  .loc 1 20
  .loc 1 20
  .loc 1 20
  mov rax, -1
  push rax
  .loc 1 20
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 32]
  push rax
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 36]
  pop rdi
  mov [rdi], rax
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 32]
  mov rax, [rax]
  push rax
  .loc 1 20
  .loc 1 20
  .loc 1 20
  .loc 1 20
  mov rax, 1
  push rax
  .loc 1 20
  .loc 1 20
  .loc 1 20
  lea rax, [rbp - 32]
  mov rax, [rax]
  mov eax, [rax]
  pop rdi
  add eax, edi
  pop rdi
  mov [rdi], eax
  pop rdi
  add eax, edi
  jmp .L.begin.13
.L..0:
.L.return.init_board:
  mov rsp, rbp
  pop rbp
  ret
