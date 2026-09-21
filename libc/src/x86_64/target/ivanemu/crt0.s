  # _start: call main, then halt the machine with its return value.
  .text
  .globl _start
_start:
  call main
  mov $0x10000008, %rdi
  mov %al, (%rdi)

  # The halt register stops the machine, so this is only reached if it does not.
.Lhang:
  jmp .Lhang
