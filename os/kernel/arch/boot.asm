section .multiboot
    dd 0x1BADB002
    dd 0x00
    dd -(0x1BADB002)

section .text
    global start
    extern kernel_main

start:
    call kernel_main
    hlt
