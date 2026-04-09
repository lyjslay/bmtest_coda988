//-----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2013-2014 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : $Date: 2013-03-11 17:54:35 +0000 (Mon, 11 Mar 2013) $
//
//      Revision            : $Revision: 240933 $
//
//      Release Information : CORTEXA53-r0p4-00rel1
//-----------------------------------------------------------------------------
// Purpose:
//   This file contains definitions that are used by multiple files.  It is
//   loaded by files that require these symbols using the ".include" directive.
//-----------------------------------------------------------------------------

// Stack definitions.  Each CPU gets a chunk of the global stack space.
.equ CPU_STACK_SIZE, 1024
.equ STACK_SIZE,     CPU_STACK_SIZE*4
.equ HEAP_SIZE,      1024*4

