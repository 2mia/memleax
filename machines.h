#ifndef MLX_MACHINES_H
#define MLX_MACHINES_H

#include "ptrace_utils.h"

/*

X86_64 ---------------
Registers
In the assembly syntax accepted by gcc, register names are always prefixed with %.  All of these registers are 64 bits wide.
The register file is as follows:

Register		Purpose												Saved across calls
%rax			temp register; return value							No
%rbx			callee-saved										Yes
%rcx			used to pass 4th argument to functions				No
%rdx			used to pass 3rd argument to functions				No
%rsp			stack pointer										Yes
%rbp			callee-saved; base pointer							Yes
%rsi			used to pass 2nd argument to functions				No
%rdi			used to pass 1st argument to functions				No
%r8				used to pass 5th argument to functions				No
%r9				used to pass 6th argument to functions				No
%r10-r11		temporary											No
%r12-r15		callee-saved registers								Yes

ARM 32b --------------
There are 16 data/core registers r0-r15
Of these 16, three are special purpose registers
Register r13 acts as the stack pointer (sp)
Register r14 acts as the link register (lr)
Register r15 acts as the program counter (pc)

ARM 64b --------------
For the purposes of function calls, the general-purpose registers are divided into four groups:
Argument registers (X0-X7)
	These are used to pass parameters to a function and to return a result. They can be used as scratch registers or as caller-saved register variables 
	that can hold intermediate values within a function, between calls to other functions. The fact that 8 registers are available for passing parameters 
	reduces the need to spill parameters to the stack when compared with AArch32.
Caller-saved temporary registers (X9-X15)
	If the caller requires the values in any of these registers to be preserved across a call to another function, the caller must save the affected 
	registers in its own stack frame. They can be modified by the called subroutine without the need to save and restore them before returning to the caller.
Callee-saved registers (X19-X29)
	These registers are saved in the callee frame. They can be modified by the called subroutine as long as they are saved and restored before returning.
Registers with a special purpose (X8, X16-X18, X29, X30)
	X8 is the indirect result register. This is used to pass the address location of an indirect result, for example, where a function returns a large structure.
	X16 and X17 are IP0 and IP1, intra-procedure-call temporary registers. These can be used by call veneers and similar code, or as temporary registers f
	or intermediate values between subroutine calls. They are corruptible by a function. Veneers are small pieces of code which are automatically inserted by the linker,
	for example when the branch target is out of range of the branch instruction.
	X18 is the platform register and is reserved for the use of platform ABIs. This is an additional temporary register on platforms that don't assign a 
	special meaning to it.
	X29 is the frame pointer register (FP).
	X30 is the link register (LR).
*/
/* We use the names in Linux, and define the aliases for FreeBSD. */
#ifdef MLX_FREEBSD
#define esp r_esp
#define eax r_eax
#define esi r_esi
#define edi r_edi
#define eip r_eip

#define rsp r_rsp
#define rax r_rax
#define rsi r_rsi
#define rdi r_rdi
#define rip r_rip
#define rdx r_rdx
#define rcx r_rcx
#define r8 r_r8
#define r9 r_r9
#endif


#if defined(MLX_X86)
static inline uintptr_t call_return_address(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp);
}
static inline uintptr_t call_return_value(registers_info_t *regs)
{
	return regs->eax;
}
static inline uintptr_t call_arg1(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 4);
}
static inline uintptr_t call_arg2(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 8);
}
static inline uintptr_t call_arg3(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 12);
}
static inline uintptr_t call_arg4(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 16);
}
static inline uintptr_t call_arg5(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 20);
}
static inline uintptr_t call_arg6(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->esp + 24);
}
static inline uintptr_t pc_unwind(pid_t pid, registers_info_t *regs)
{
	regs->eip--;
	ptrace_set_regs(pid, regs);
	return regs->eip;
}
static inline void set_breakpoint(pid_t pid, uintptr_t address, uintptr_t code)
{
	ptrace_set_data(pid, address, (code & 0xFFFFFF00U) | 0xCC);
}
static inline int is_breakpoint(pid_t pid, uintptr_t address)
{
	return (ptrace_get_data(pid, address) & 0xFF) == 0xCC;
}

#elif defined(MLX_X86_64)
static inline uintptr_t call_return_address(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->rsp);
}
static inline uintptr_t call_return_value(registers_info_t *regs)
{
	return regs->rax;
}
static inline uintptr_t call_arg1(pid_t pid, registers_info_t *regs)
{
	return regs->rdi;
}
static inline uintptr_t call_arg2(pid_t pid, registers_info_t *regs)
{
	return regs->rsi;
}
static inline uintptr_t call_arg3(pid_t pid, registers_info_t *regs)
{
	return regs->rdx;
}
static inline uintptr_t call_arg4(pid_t pid, registers_info_t *regs)
{
	return regs->rcx;
}
static inline uintptr_t call_arg5(pid_t pid, registers_info_t *regs)
{
	return regs->r8;
}
static inline uintptr_t call_arg6(pid_t pid, registers_info_t *regs)
{
	return regs->r9;
}
static inline uintptr_t pc_unwind(pid_t pid, registers_info_t *regs)
{
	regs->rip--;
	ptrace_set_regs(pid, regs);
	return regs->rip;
}
static inline void set_breakpoint(pid_t pid, uintptr_t address, uintptr_t code)
{
  #ifdef MLX_LINUX
	ptrace_set_data(pid, address, (code & 0xFFFFFFFFFFFFFF00UL) | 0xCC);
  #else // MLX_FREEBSD
	ptrace_set_data(pid, address, (code & 0xFFFFFF00U) | 0xCC);
  #endif
}
static inline int is_breakpoint(pid_t pid, uintptr_t address)
{
	return (ptrace_get_data(pid, address) & 0xFF) == 0xCC;
}

#elif defined(MLX_ARMv7)
static inline uintptr_t call_return_address(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[14];
}
static inline uintptr_t call_return_value(registers_info_t *regs)
{
	return regs->uregs[0];
}
static inline uintptr_t call_arg1(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[0];
}
static inline uintptr_t call_arg2(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[1];
}
static inline uintptr_t call_arg3(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[2];
}
static inline uintptr_t call_arg4(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[3];
}
static inline uintptr_t call_arg5(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->uregs[13]);
}
static inline uintptr_t call_arg6(pid_t pid, registers_info_t *regs)
{
	return ptrace_get_data(pid, regs->uregs[13] + 4);
}
static inline uintptr_t pc_unwind(pid_t pid, registers_info_t *regs)
{
	return regs->uregs[15];
}
static inline void set_breakpoint(pid_t pid, uintptr_t address, uintptr_t code)
{
	ptrace_set_data(pid, address, 0xE7F001F0);
}
static inline int is_breakpoint(pid_t pid, uintptr_t address)
{
	return ptrace_get_data(pid, address) == 0xE7F001F0;
}

#elif defined(MLX_AARCH64)
static inline uintptr_t call_return_address(pid_t pid, registers_info_t *regs)
{
	return regs->regs[30];
}
static inline uintptr_t call_return_value(registers_info_t *regs)
{
	return regs->regs[0];
}
static inline uintptr_t call_arg1(pid_t pid, registers_info_t *regs)
{
	return regs->regs[0];
}
static inline uintptr_t call_arg2(pid_t pid, registers_info_t *regs)
{
	return regs->regs[1];
}
static inline uintptr_t call_arg3(pid_t pid, registers_info_t *regs)
{
	return regs->regs[2];
}
static inline uintptr_t call_arg4(pid_t pid, registers_info_t *regs)
{
	return regs->regs[3];
}
static inline uintptr_t call_arg5(pid_t pid, registers_info_t *regs)
{
	return regs->regs[4];
}
static inline uintptr_t call_arg6(pid_t pid, registers_info_t *regs)
{
	return regs->regs[5];
}
static inline uintptr_t pc_unwind(pid_t pid, registers_info_t *regs)
{
	return regs->pc;
}
static inline void set_breakpoint(pid_t pid, uintptr_t address, uintptr_t code)
{
	ptrace_set_data(pid, address, 0xd4200000);
}
static inline int is_breakpoint(pid_t pid, uintptr_t address)
{
	return ptrace_get_data(pid, address) == 0xd4200000;
}
#endif

#endif
