OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64") OUTPUT_ARCH(aarch64)
ENTRY(start)

MEMORY
{
	sdram (xrw)      : ORIGIN = 0x80080000, LENGTH = 0x10000000
}

STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 0x8000000;

SECTIONS
{
  .text :
  {
    . = ALIGN(8);
    __text_start = .;
    KEEP(*(.text.start))
    . = ALIGN(8);
    *(.text .text*)
    *(.rodata .rodata*)
    . = ALIGN(8);
  } > sdram

  . = ALIGN(8);
  __module_testcase_start = .;
  .module0.testcase :
  {
      KEEP(*(.module0.testcase))
  } > sdram
  .module1.testcase :
  {
      KEEP(*(.module1.testcase))
  } > sdram
  .module2.testcase :
  {
      KEEP(*(.module2.testcase))
  } > sdram
  __module_testcase_end = .;

  __rw_start = LOADADDR(.data);

  .data :
  {
    . = ALIGN(8);
    __data_start = .;
    *(.data .data.*)
    . = ALIGN(8);
    __data_end = .;
  } > sdram

  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start = . ;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    __bss_end = . ;
  } > sdram

  .stack (NOLOAD) :
  {
    . = ALIGN(8);
    __stack_start = . ;
    . = . + STACK_SIZE;
    . = ALIGN(8);
    __stack_end = . ;
  } > sdram

  /DISCARD/ :
  {
	*(.init*)
	*(.fini*)
  }

}

