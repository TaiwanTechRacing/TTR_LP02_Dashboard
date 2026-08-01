/*
 * Linker script for the STM32CubeProgrammer external loader.
 *
 * The loader is downloaded into the target's RAM and executed there, so
 * everything - code, data and bss - is linked at a RAM address. There is no
 * flash region and no vector table: CubeProgrammer calls the exported
 * functions directly rather than resetting the part.
 *
 * 0x24000000 is AXI SRAM. The firmware normally uses it, but nothing of the
 * firmware is running while the loader is.
 */

ENTRY(Init)

MEMORY
{
    RAM (xrw) : ORIGIN = 0x24000000, LENGTH = 512K
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > RAM

    /*
     * CubeProgrammer finds the device description by section name, so this must
     * survive with its name intact. KEEP stops --gc-sections discarding it.
     */
    .Dev_Info :
    {
        . = ALIGN(4);
        KEEP(*(.Dev_Info))
        . = ALIGN(4);
    } > RAM

    .data :
    {
        . = ALIGN(4);
        *(.data)
        *(.data*)
        . = ALIGN(4);
    } > RAM

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Stack space for the loader's own calls. CubeProgrammer sets SP before
     * entering, but reserving the room keeps it out of .bss. */
    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        . = . + 0x1000;
        . = ALIGN(8);
    } > RAM

    /DISCARD/ :
    {
        *(.ARM.exidx*)
        *(.ARM.extab*)
    }
}
