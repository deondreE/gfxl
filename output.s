.text
  .globl main
  .globl print_int
main:
  pushq %rbp
  movq %rsp, %rbp
  pushq $3
  pushq $2
  popq %rbx
  popq %rax
  imulq %rbx, %rax
  pushq %rax
  pushq $1
  popq %rbx
  popq %rax
  addq %rbx, %rax
  pushq %rax
  popq %rax
  movq %rax, -8(%rbp)
  pushq $2
  pushq $4
  pushq -8(%rbp)
  popq %rbx
  popq %rax
  addq %rbx, %rax
  pushq %rax
  popq %rbx
  popq %rax
  xorq %rdx, %rdx
  idivq %rbx
  pushq %rax
  popq %rax
  movq %rax, -16(%rbp)
  pushq $3
  pushq $2
  pushq $1
  popq %rbx
  popq %rax
  addq %rbx, %rax
  pushq %rax
  popq %rbx
  popq %rax
  addq %rbx, %rax
  pushq %rax
  popq %rdi
  call print_int
  pushq -8(%rbp)
  popq %rdi
  call print_int
  movq $0, %rax
  leave
  ret
