    .section .text
    .globl _start
_start:
    la sp, __stack_top
    la a0, __data_start
    la a1, __data_end
    la a2, _etext
copy_data:
    beq a0, a1, call_my_main
    lw t0, 0(a2)
    sw t0, 0(a0)
    addi a0, a0, 4
    addi a2, a2, 4
    j copy_data

call_my_main:
    #la t4, 0x44A00000
    #sw x0, 0(t4) 
    call my_main
    la t0, 0xdeadbeef
    la a0, 0
    sw t0, 0(a0)
    la ra, 0
    ret
