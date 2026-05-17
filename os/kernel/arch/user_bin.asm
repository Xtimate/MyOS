section .rodata
global user_hello_start
global user_hello_end

user_hello_start:
    incbin "user/hello"
user_hello_end:
