//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013-2016 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2016-04-29 14:14:55 +0100 (Fri, 29 Apr 2016) $
//
//      Revision            : $Revision: 340883 $
//
//      Release Information : CORTEXA53-r0p4-00rel1
//
//-------------------------------------------------------------------------------
// Description:
//
//   Set up translation tables for the execution_tb tests.  Short descriptor
//   format sections are used, each  defining 1MB of the memory space.
//
//   The 1MB region containing the validation peripherals is marked as
//   strongly ordered.  The rest of the memory .space is marked as normal
//   shareable memory.
//-------------------------------------------------------------------------------

        .section pagetables, "a", %progbits
        .align 3
        .global  ttb0_base
        .global  mair_value

.equ XN,     (1<<22)
.equ PXN,    (1<<21)
.equ CONTIG, (1<<20)

.equ NGLOBAL,  (1<<11)
.equ AF,       (1<<10)
.equ NON_SH,   (0<<8)
.equ OUTER_SH, (2<<8)
.equ INNER_SH, (3<<8)
.equ AP2,      (1<<7)
.equ AP1,      (1<<6)
.equ NS,       (1<<5)
.equ IDX0,     (0<<2)
.equ IDX1,     (1<<2)
.equ IDX2,     (2<<2)
.equ IDX3,     (3<<2)
.equ IDX4,     (4<<2)
.equ IDX5,     (5<<2)
.equ IDX6,     (6<<2)
.equ IDX7,     (7<<2)
.equ BLOCK,    (1<<0)
.equ TABLE,    (3<<0)
.equ UNMAPPED, (0<<0)

.equ OUTER_WT,       (0<<6)
.equ OUTER_WB,       (1<<6)
.equ OUTER_NC,       (1<<6)
.equ OUTER_NONTRANS, (1<<7)
.equ OUTER_WRALLOC,  (1<<4)
.equ OUTER_RDALLOC,  (1<<5)

.equ nGnRnE, 0x00
.equ nGnRE,  0x04
.equ nGRE,   0x08
.equ GRE,    0x0C

.equ INNER_WT,       (0<<2)
.equ INNER_WB,       (1<<2)
.equ INNER_NC,       (1<<2)
.equ INNER_NONTRANS, (1<<3)
.equ INNER_WRALLOC,  (1<<0)
.equ INNER_RDALLOC,  (1<<1)

.equ IO_WBRWA, (OUTER_WB | OUTER_NONTRANS | OUTER_WRALLOC | OUTER_RDALLOC | INNER_WB | INNER_NONTRANS | INNER_WRALLOC | INNER_RDALLOC)

// Memory attributes for MAIR
.equ ATTR0, nGnRnE
.equ ATTR1, nGnRE
.equ ATTR2, IO_WBRWA
.equ ATTR3, 0
.equ ATTR4, 0
.equ ATTR5, 0
.equ ATTR6, 0
.equ ATTR7, 0

.ifndef ARCH
.equ ARCH, 64
.endif
.ifndef BIGEND
.equ BIGEND, 0
.endif

        // Put 64-bit value with correct endianness
        .macro PUT_64B high, low
        .if (ARCH == 64)
        .quad (\high << 32) + \low
        .else
        .if (BIGEND == 0)
        .word \low
        .word \high
        .else
        .word \high
        .word \low
        .endif
        .endif
        .endm

mair_value:
        PUT_64B ((ATTR7<<24) | (ATTR6<<16) | (ATTR5<<8) | (ATTR4<<0)), ((ATTR3<<24) | (ATTR2<<16) | (ATTR1<<8) | (ATTR0<<0))

// Memory is marked as inner and outer cacheable + write allocate
.equ MEMORY,    (IDX2 | AP1 | AF | NS)
.equ DEVICE,    (IDX1 | AP1 | AF)

.equ SHARED_MEMORY,    (MEMORY    | INNER_SH)

        // Create an entry pointing to a next-level table
        .macro TABLE_ENTRY PA, ATTR
        PUT_64B   \ATTR, ((\PA) + TABLE)
        .endm

        // Create an entry for a 1GB block
        .macro BLOCK_1GB PA, ATTR_HI, ATTR_LO
        PUT_64B   \ATTR_HI, ((\PA)  &  0xC0000000)  |  \ATTR_LO  |  BLOCK
        .endm

        // Create an entry for a 2MB block
        .macro BLOCK_2MB PA, ATTR_HI, ATTR_LO
        PUT_64B   \ATTR_HI, ((\PA)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        .endm

        // Create an entry for a 2MB block
        .macro BLOCK_4KB PA, ATTR_HI, ATTR_LO
        PUT_64B   \ATTR_HI, ((\PA)  &  0xFFFFF000)  |  \ATTR_LO  |  BLOCK
        .endm

        // Create an entry for a 32MB block (using multiple 2MB blocks)
        .macro BLOCK_32MB PA, ATTR_HI, ATTR_LO
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA)                &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0200000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0400000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0600000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0800000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0A00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0C00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x0E00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1000000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1200000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1400000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1600000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1800000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1A00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1C00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        PUT_64B   \ATTR_HI  |  CONTIG, ((\PA  |  0x1E00000)  &  0xFFE00000)  |  \ATTR_LO  |  BLOCK
        .endm

//------------------------------------------------------------------------------
// Second-level table
//
//   0x00000000 - 0x12FFFFFF : Memory
//   0x13000000 - 0x13FFFFFF : Device (trickbox area)
//   0x14000000 - 0x3FFFFFFF : Memory
//------------------------------------------------------------------------------

        // Align to page
        .balign (1<<12)

ttb_second_level_0:
        .set ADDR, 0x000  // Address counter for blocks

        // Range 0x00000000 -> 0x12ffffff: memory
        .rept (0x120 / 32)  // Address range (divide by page size for this block)
        BLOCK_32MB (ADDR << 20), 0, SHARED_MEMORY
        .set ADDR, ADDR+32
        .endr
        .rept ((0x130 - 0x120) / 2)
        BLOCK_2MB (ADDR << 20), 0, SHARED_MEMORY
        .set ADDR, ADDR+2
        .endr

        // Range 0x13000000 -> 0x13ffffff: registers
        .rept ((0x140 - 0x130) / 2)
        BLOCK_2MB (ADDR << 20), (PXN|XN), DEVICE
        .set ADDR, ADDR+2
        .endr

        // Range 0x14000000 -> 0x3FFFFFFF: memory
        .rept ((0x400 - 0x140) / 32)
        BLOCK_32MB (ADDR << 20), 0, SHARED_MEMORY
        .set ADDR, ADDR+32
        .endr

//------------------------------------------------------------------------------
// First-level table
//
//   Entry 0: -> Second-level table
//   Entry 1: Block 0x40000000 - 0x7FFFFFFF : Memory
//   Entry 2: Block 0x80000000 - 0xBFFFFFFF : Memory
//   Entry 3: Block 0xC0000000 - 0xFFFFFFFF : Memory
//------------------------------------------------------------------------------

        // Align to page
        .balign (1<<12)

ttb0_base:
.ifdef ORIG_PAGETABLE
        TABLE_ENTRY ttb_second_level_0, 0
        BLOCK_1GB   0x40000000, 0, SHARED_MEMORY
        BLOCK_1GB   0x80000000, 0, SHARED_MEMORY
        BLOCK_1GB   0xC0000000, 0, SHARED_MEMORY
.else
        BLOCK_1GB   0x00000000, 0, SHARED_MEMORY
        BLOCK_1GB   0x40000000, 0, SHARED_MEMORY
        BLOCK_1GB   0x80000000, 0, SHARED_MEMORY
        BLOCK_1GB   0xC0000000, 0, DEVICE
.endif

        .end

