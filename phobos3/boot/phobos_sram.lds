OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64") OUTPUT_ARCH(aarch64)
ENTRY(start)

MEMORY
{
	sram (xrw)      : ORIGIN = 0x0C000000, LENGTH = 0x0040000
}

STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 0x8000;

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
  } > sram

  . = ALIGN(8);
  __module_testcase_start = .;
  .module0.testcase :
  {
      KEEP(*(.module0.testcase))
  } > sram
  .module1.testcase :
  {
      KEEP(*(.module1.testcase))
  } > sram
  .module2.testcase :
  {
      KEEP(*(.module2.testcase))
  } > sram
  __module_testcase_end = .;

  __rw_start = LOADADDR(.data);

  .data :
  {
    . = ALIGN(8);
    __data_start = .;
    *(.data .data.*)
    . = ALIGN(8);
    __data_end = .;
  } > sram

  .bss (NOLOAD) :
  {
    . = ALIGN(8);
    __bss_start = . ;
    *(.bss .bss.*)
    *(COMMON)
    . = ALIGN(8);
    __bss_end = . ;
  } > sram

  .stack (NOLOAD) :
  {
    . = ALIGN(8);
    __stack_start = . ;
    . = . + STACK_SIZE;
    . = ALIGN(8);
    __stack_end = . ;
  } > sram

  /DISCARD/ :
  {
	*(.init*)
	*(.fini*)
  }

}

