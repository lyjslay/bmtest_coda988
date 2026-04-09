#include <arch.h>
#include "asm/csr.h"

	.section	.text
	.align	8

	.global		s_el1_entry
	.type		s_el_entry, %function
s_el1_entry:
	/* x0: previous stack */
	/* x1: callback function */
	/* Continue to use original stack */
	mov sp, x0

	/* Allow to access fp/simd(e.g. printf) */
	mrs x0, cpacr_el1
	orr x0, x0, #(3<<20)
	msr cpacr_el1, x0

	blr x1

	ldp	x29, x30, [sp],#32
	ret

	.global		switch_el3_to_s_el1
	.type		switch_el3_to_s_el1, %function
switch_el3_to_s_el1:
	/* x0: spsr */
	/* x1: callback function */

	/* Behave as usual function call */
	stp	x29, x30, [sp,#-32]!
	mov	x29, sp

	msr	spsr_el3, x0
	ldr x0, =s_el1_entry
	msr elr_el3, x0

	/* Set the next EL to be AArch64 */
	mrs x0, scr_el3
	orr x0, x0, #(1<<10)
	
	/* Switch to secure state. */
	bic x0, x0, #1
	
	msr scr_el3, x0

	/* Same exception vector */
	mrs x0, vbar_el3
	msr vbar_el1, x0

	/* Need to switch sp_el3 */
	mov x0, sp
	msr	spsel, #1
	mov sp, x0

	eret

	ret

	.global		switch_el3_to_el1
	.type		switch_el3_to_el1, %function
switch_el3_to_el1:
	/* x0: spsr */
	/* x1: callback function */

	/* Behave as usual function call */
	stp	x29, x30, [sp,#-32]!
	mov	x29, sp

	/*
	 * Setup EL3
	 */
	msr	spsr_el3, x0
	ldr x0, =s_el1_entry
	msr elr_el3, x0

	/* Set the next EL to be AArch64 */
	mrs x0, scr_el3
	orr x0, x0, #(1<<10)

	/* Switch to non-secure state. */
	orr x0, x0, #1

	/* Disable Hypervisor call */
	bic x0, x0, #8

	msr scr_el3, x0

	/* 
	 * Setup EL2
	 */
	mrs x0, hcr_el2

	/* Set the next EL(EL1) to be AArch64 */
	orr x0, x0, #(1<<31)

	msr hcr_el2, x0

	/*
	 * Initialise CPTR_EL2 setting all fields rather than
	 * relying on the hw. All fields have architecturally
	 * UNKNOWN reset values.
	 *
	 * CPTR_EL2.TCPAC: Set to zero so that Non-secure EL1
	 *  accesses to the CPACR_EL1 or CPACR from both
	 *  Execution states do not trap to EL2.
	 *
	 * CPTR_EL2.TTA: Set to zero so that Non-secure System
	 *  register accesses to the trace registers from both
	 *  Execution states do not trap to EL2.
	 *
	 * CPTR_EL2.TFP: Set to zero so that Non-secure accesses
	 *  to SIMD and floating-point functionality from both
	 *  Execution states do not trap to EL2.
	 */
	mov x0, (CPTR_EL2_RESET_VAL & \
			(CPTR_EL2_TCPAC_BIT | CPTR_EL2_TTA_BIT \
			| CPTR_EL2_TFP_BIT))

	msr cptr_el2, x0

	/*
	 * Initiliase CNTHCTL_EL2. All fields are
	 * architecturally UNKNOWN on reset and are set to zero
	 * except for field(s) listed below.
	 *
	 * CNTHCTL_EL2.EL1PCEN: Set to one to disable traps to
	 *  Hyp mode of Non-secure EL0 and EL1 accesses to the
	 *  physical timer registers.
	 *
	 * CNTHCTL_EL2.EL1PCTEN: Set to one to disable traps to
	 *  Hyp mode of  Non-secure EL0 and EL1 accesses to the
	 *  physical counter registers.
	 */
	mov x0, (CNTHCTL_RESET_VAL | EL1PCEN_BIT | EL1PCTEN_BIT)
	msr cnthctl_el2, x0

	/*
	 * Initialise CNTVOFF_EL2 to zero as it resets to an
	 * architecturally UNKNOWN value.
	 */
	//write_cntvoff_el2(0);
	mov x0, xzr
	msr cntvoff_el2, x0

	/*
	 * Set VPIDR_EL2 and VMPIDR_EL2 to match MIDR_EL1 and
	 * MPIDR_EL1 respectively.
	 */
	mrs x0, midr_el1
	msr vpidr_el2, x0

	mrs x0, mpidr_el1
	msr vmpidr_el2, x0

	/*
	 * Initialise VTTBR_EL2. All fields are architecturally
	 * UNKNOWN on reset.
	 *
	 * VTTBR_EL2.VMID: Set to zero. Even though EL1&0 stage
	 *  2 address translation is disabled, cache maintenance
	 *  operations depend on the VMID.
	 *
	 * VTTBR_EL2.BADDR: Set to zero as EL1&0 stage 2 address
	 *  translation is disabled.
	 */
	mov x0, #(VTTBR_RESET_VAL & \
		~((VTTBR_VMID_MASK << VTTBR_VMID_SHIFT) \
		| (VTTBR_BADDR_MASK << VTTBR_BADDR_SHIFT)))
	msr vttbr_el2, x0

	/*
	 * Initialise MDCR_EL2, setting all fields rather than
	 * relying on hw. Some fields are architecturally
	 * UNKNOWN on reset.
	 *
	 * MDCR_EL2.TPMS (ARM v8.2): Do not trap statistical
	 * profiling controls to EL2.
	 *
	 * MDCR_EL2.E2PB (ARM v8.2): SPE enabled in non-secure
	 * state. Accesses to profiling buffer controls at
	 * non-secure EL1 are not trapped to EL2.
	 *
	 * MDCR_EL2.TDRA: Set to zero so that Non-secure EL0 and
	 *  EL1 System register accesses to the Debug ROM
	 *  registers are not trapped to EL2.
	 *
	 * MDCR_EL2.TDOSA: Set to zero so that Non-secure EL1
	 *  System register accesses to the powerdown debug
	 *  registers are not trapped to EL2.
	 *
	 * MDCR_EL2.TDA: Set to zero so that System register
	 *  accesses to the debug registers do not trap to EL2.
	 *
	 * MDCR_EL2.TDE: Set to zero so that debug exceptions
	 *  are not routed to EL2.
	 *
	 * MDCR_EL2.HPME: Set to zero to disable EL2 Performance
	 *  Monitors.
	 *
	 * MDCR_EL2.TPM: Set to zero so that Non-secure EL0 and
	 *  EL1 accesses to all Performance Monitors registers
	 *  are not trapped to EL2.
	 *
	 * MDCR_EL2.TPMCR: Set to zero so that Non-secure EL0
	 *  and EL1 accesses to the PMCR_EL0 or PMCR are not
	 *  trapped to EL2.
	 *
	 * MDCR_EL2.HPMN: Set to value of PMCR_EL0.N which is the
	 *  architecturally-defined reset value.
	 */
	//mdcr_el2 = ((MDCR_EL2_RESET_VAL |
	//		((read_pmcr_el0() & PMCR_EL0_N_BITS)
	//		>> PMCR_EL0_N_SHIFT)) &
	//		~(MDCR_EL2_TDRA_BIT | MDCR_EL2_TDOSA_BIT
	//		| MDCR_EL2_TDA_BIT | MDCR_EL2_TDE_BIT
	//		| MDCR_EL2_HPME_BIT | MDCR_EL2_TPM_BIT
	//		| MDCR_EL2_TPMCR_BIT));

	// MDCR_EL2_RESET_VAL == 0
	mrs x0, pmcr_el0
	and x0, x0, PMCR_EL0_N_BITS
	lsr x0, x0, #PMCR_EL0_N_SHIFT
	and x0, x0, ~(MDCR_EL2_TDRA_BIT | MDCR_EL2_TDOSA_BIT \
			| MDCR_EL2_TDA_BIT | MDCR_EL2_TDE_BIT \
			| MDCR_EL2_HPME_BIT | MDCR_EL2_TPM_BIT \
			| MDCR_EL2_TPMCR_BIT)

#if 0 // ENABLE_SPE_FOR_LOWER_ELS
	uint64_t id_aa64dfr0_el1;

	/* Detect if SPE is implemented */
	id_aa64dfr0_el1 = read_id_aa64dfr0_el1() >>
		ID_AA64DFR0_PMS_SHIFT;
	if ((id_aa64dfr0_el1 & ID_AA64DFR0_PMS_MASK) == 1) {
		/*
		 * Make sure traps to EL2 are not generated if
		 * EL2 is implemented but not used.
		 */
		mdcr_el2 &= ~MDCR_EL2_TPMS;
		mdcr_el2 |= MDCR_EL2_E2PB(MDCR_EL2_E2PB_EL1);
	}
#endif

	//write_mdcr_el2(mdcr_el2);
	msr mdcr_el2, x0

	/*
	 * Initialise HSTR_EL2. All fields are architecturally
	 * UNKNOWN on reset.
	 *
	 * HSTR_EL2.T<n>: Set all these fields to zero so that
	 *  Non-secure EL0 or EL1 accesses to System registers
	 *  do not trap to EL2.
	 */
	//write_hstr_el2(HSTR_EL2_RESET_VAL & ~(HSTR_EL2_T_MASK));
	mov x0, #(HSTR_EL2_RESET_VAL & ~(HSTR_EL2_T_MASK))
	msr hstr_el2, x0

	/*
	 * Initialise CNTHP_CTL_EL2. All fields are
	 * architecturally UNKNOWN on reset.
	 *
	 * CNTHP_CTL_EL2:ENABLE: Set to zero to disable the EL2
	 *  physical timer and prevent timer interrupts.
	 */
	//write_cnthp_ctl_el2(CNTHP_CTL_RESET_VAL &
	//			~(CNTHP_CTL_ENABLE_BIT));
	mov x0, #(CNTHP_CTL_RESET_VAL & \
				~(CNTHP_CTL_ENABLE_BIT))
	msr cnthp_ctl_el2, x0

	/* 
	 * Setup EL1
	 */
	/* Same exception vector */
	mrs x0, vbar_el3
	msr vbar_el1, x0

	/* Need to switch sp_el3 */
	mov x0, sp
	msr	spsel, #1
	mov sp, x0

	eret

	ret

	.global		get_arm_current_el
	.type		get_arm_current_el, %function
get_arm_current_el:
	/* EL[3:2] */
	mrs	x0, currentel
	lsr x0, x0, #2
	and x0, x0, #3
	ret
