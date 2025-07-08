.intel_syntax noprefix
.global _start
.text
main:
  push rbp
  mov rbp, rsp
  push 3
  push 2
  pop rax
  pop rbx
  imul rax, rbx
  push rax
  push 1
  pop rax
  pop rbx
  add rax, rbx
  push rax
  pop rax
  mov QWORD PTR [rbp - 8], rax
  push 2
  push 4
  push QWORD PTR [rbp - 8]
  pop rax
  pop rbx
  add rax, rbx
  push rax
  pop rax
  pop rbx
  xor rdx, rdx
  idiv rbx
  push rax
  pop rax
  mov QWORD PTR [rbp - 16], rax
  push 3
  push 2
  push 1
  pop rax
  pop rbx
  add rax, rbx
  push rax
  pop rax
  pop rbx
  add rax, rbx
  push rax
  pop rdi
  call print_int
  push QWORD PTR [rbp - 8]
  pop rdi
  call print_int
  mov rax, 60
  xor rdi, rdi
  syscall
_start:
  call main
  mov rax, 60
  xor rdi, rdi
  syscall
