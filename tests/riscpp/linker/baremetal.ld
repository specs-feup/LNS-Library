ENTRY(_start)
MEMORY
{
  CODE_RAM (rx)  : ORIGIN = 0x00000000, LENGTH = 0x4000 /* 16 KB */
  DATA_RAM (rwx) : ORIGIN = 0x00004000, LENGTH = 0x10000 /* 64 KB */
}
SECTIONS
{
  .text :
  {
    KEEP(*(.vectors))
    *(.text*)
  } > CODE_RAM
  
  .rodata : 
  {
    *(.rodata*)
  } > CODE_RAM
  
  .data : AT (ADDR(.rodata) + SIZEOF(.rodata))
  {
    __data_start = .;
    *(.data*)
    __data_end = .;
  } > DATA_RAM
  
  .bss (NOLOAD) :
  {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  } > DATA_RAM
  
  .stack (NOLOAD) :
  {
    . = ORIGIN(DATA_RAM) + LENGTH(DATA_RAM);
    __stack_top = .;
  } > DATA_RAM
  
  PROVIDE(_etext = ADDR(.text) + SIZEOF(.text));
  PROVIDE(_end = .);
  PROVIDE(__heap_start = _end);
  PROVIDE(__heap_end = __stack_top);
}
