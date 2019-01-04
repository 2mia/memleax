/*
 * memory allocation/free API breakpoints
 *
 * Author: Wu Bingzheng
 *   Date: 2016-5
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "callstack.h"
#include "memblock.h"
#include "machines.h"
#include "breakpoint.h"
#include "ptr_backtrace.h"
#include "ptrace_utils.h"
#include "symtab.h"
#include "memleax.h"

#define NUMBER_OF_BPH 7
struct breakpoint_s g_breakpoints[NUMBER_OF_BPH];

static int bph_malloc(uintptr_t pointer, uintptr_t size, uintptr_t none1,
	uintptr_t none2, uintptr_t none3, uintptr_t none4, uintptr_t none5)
{
	log_debug("-- malloc size:%ld ret:%lx\n", size, pointer);

	return memblock_new(pointer, size);
}

static int bph_free(uintptr_t none1, uintptr_t pointer, uintptr_t none2,
	uintptr_t none3, uintptr_t none4, uintptr_t none5, uintptr_t none6)
{
	log_debug("-- free point:%lx\n", pointer);

	struct memblock_s *to_delete = memblock_search(pointer);
	if (to_delete == NULL)
		log_debug("-- free invalid pointer:%lx\n", pointer);
	memblock_delete(to_delete);
	return 0;
}

static int bph_realloc(uintptr_t new_pointer, uintptr_t old_pointer, uintptr_t size,
	uintptr_t none1, uintptr_t none2, uintptr_t none3, uintptr_t none4)
{
	log_debug("-- realloc pointer:%lx->%lx size:%ld\n", old_pointer, new_pointer, size);

	if (new_pointer == old_pointer) {
		memblock_update_size(memblock_search(old_pointer), size);
		return 0;
	} else {
		memblock_delete(memblock_search(old_pointer));
		return memblock_new(new_pointer, size);
	}
}

static int bph_calloc(uintptr_t pointer, uintptr_t nmemb, uintptr_t size,
	uintptr_t none1, uintptr_t none2, uintptr_t none3, uintptr_t none4)
{
	log_debug("-- calloc pointer:%lx nmemb:%ld size:%ld\n", pointer, nmemb, size);

	return memblock_new(pointer, nmemb * size);
}

static int bph_posix_memalign(uintptr_t result, uintptr_t pointer_addr, uintptr_t alignment, uintptr_t size,
	uintptr_t none1, uintptr_t none2, uintptr_t none3)
{
	if (result != 0) {
		log_debug("-- posix_memalign FAILURE: %ld\n", result);
		callstack_print(callstack_current());
		return 0;
	}
	uintptr_t *addr = (uintptr_t*)pointer_addr;
	log_debug("-- posix_memalign pointer:%lx alignment:%ld size:%ld\n", *addr, alignment, size);

	return memblock_new(*addr, size);
}

static int bph_mmap(uintptr_t pointer, uintptr_t addr, uintptr_t len, uintptr_t prot,
	uintptr_t flags, uintptr_t fd, uintptr_t offset)
{
	log_debug("-- mmap ret:%lx addr:%lx len:%ld prot:%ld flags:%lx fd:%lx offset:%lx\n",
		pointer, addr, len, prot, flags, fd, offset);
	if ((pointer != (uintptr_t)MAP_FAILED) && (addr != 0) && ((flags & MAP_FIXED) != 0)) {
		/* If MAP_FIXED is specified, a successful mmap deletes any previous mapping in the allocated address range. 
		Previous mappings are never deleted if MAP_FIXED is not specified. */
		memblock_delete(memblock_search(addr));
	}

	if (pointer != (uintptr_t)MAP_FAILED) {
		return memblock_new(pointer, len - offset);
	}
	return 0;
}

static int bph_munmap(uintptr_t result, uintptr_t addr, uintptr_t len,
	uintptr_t none1, uintptr_t none2, uintptr_t none3, uintptr_t none4)
{
	log_debug("-- munmap res:%ld addr:%lx len:%ld\n", result, addr, len);
	if (result == 0) {
		memblock_delete(memblock_search(addr));
	}
	return 0;
}

static void do_breakpoint_init(pid_t pid, struct breakpoint_s *bp,
		const char *name, bp_handler_f handler)
{
	bp->name = name;
	bp->handler = handler;
	bp->entry_address = symtab_by_name(name);
	if (bp->entry_address == 0) {
		fprintf(stderr, "not found api: %s\n", name);
		exit(3);
	}

	/* read original code */
	bp->entry_code = ptrace_get_data(pid, bp->entry_address);
	log_debug("-- do_breakpoint_init: pid %ld, name %s\n", (long int)pid, name);

	/* write the trap instruction 'int 3' into the address */
	set_breakpoint(pid, bp->entry_address, bp->entry_code);
}

void breakpoint_init(pid_t pid)
{
	do_breakpoint_init(pid, &g_breakpoints[0], "malloc", bph_malloc);
	do_breakpoint_init(pid, &g_breakpoints[1], "free", bph_free);
	do_breakpoint_init(pid, &g_breakpoints[2], "realloc", bph_realloc);
	do_breakpoint_init(pid, &g_breakpoints[3], "calloc", bph_calloc);
	do_breakpoint_init(pid, &g_breakpoints[4], "posix_memalign", bph_posix_memalign);
	do_breakpoint_init(pid, &g_breakpoints[5], "mmap", bph_mmap);
	do_breakpoint_init(pid, &g_breakpoints[6], "munmap", bph_munmap);
}

void breakpoint_cleanup(pid_t pid)
{
	int i;
	for (i = 0; i < NUMBER_OF_BPH; i++) {
		struct breakpoint_s *bp = &g_breakpoints[i];
		ptrace_set_data(pid, bp->entry_address, bp->entry_code);
	}
}

struct breakpoint_s *breakpoint_by_entry(uintptr_t address)
{
	int i;
	for (i = 0; i < NUMBER_OF_BPH; i++) {
		if (address == g_breakpoints[i].entry_address) {
			return &g_breakpoints[i];
		}
	}
	return NULL;
}
