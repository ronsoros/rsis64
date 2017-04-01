#ifndef __HEADER_BY_RONSOR_VM_H
#define __HEADER_BY_RONSOR_VM_H
#include <stdint.h>
typedef enum { false, true } boolean;
typedef enum { read, write, rbyte, wbyte } mem_mode;
#pragma pack()
typedef struct rsis_instruction {
	int64_t opcode;
	int64_t param1;
	int64_t param2;
} rsis_instruction;
typedef struct rsis_vm {
        unsigned char *code;
	int64_t codesize;
	int64_t userint;
	unsigned char *userptr;
	int64_t registers[26];
        int64_t (*memop)(struct rsis_vm* vm, mem_mode opt, int64_t addr, int64_t val);
	rsis_instruction (*fetch)(struct rsis_vm*, int64_t addr);
	int64_t (*stackop)(struct rsis_vm*, boolean pop, int64_t val);
} rsis_vm;
#define pc registers[25]
#define sp registers[24]
#define bp registers[23]
#define dbg registers[22]
#define zf registers[21]
#define GetVMTypeSize(n) sizeof(#n)
enum /* OpCodes */ {
	nop = 0,
	hlt,
	rm64,
	rm8,
	wm64,
	wm8,
	add,
	sub,
	mul,
	_div,
	set,
	mov,
	push,
	pop,
	call,
	jmp,
	jt,
	jf,
	not,
	ret,
	gt,
	lt,
	eq,
	neq
};

#define E_HUH 2
#define SEGFAULTNOW 0
char* _vm_errs[4] = { "Invalid option; This can't happen?", "Invalid error; This can't happen?", 
	SEGFAULTNOW, SEGFAULTNOW };
#endif
