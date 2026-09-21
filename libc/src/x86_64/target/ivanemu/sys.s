  # putchar(c): store one byte to the UART, return the byte written.
  .text
  .globl putchar
putchar:
  mov $0x10000000, %rsi
  mov %dil, (%rsi)
  mov %rdi, %rax
  ret
