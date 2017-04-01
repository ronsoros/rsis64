#include "vm.h"
#include <stdint.h>
#ifndef RELEASE
#define DEBUG_INCLUDE
#include "asm.c"
#endif
#ifdef TEST
#endif
#define MemOp_BoundsCheck(_vm,addr) if(_vm->codesize < addr || addr < 0) return 0
int64_t DefaultMemOp(rsis_vm* vm, mem_mode opt, int64_t addr, int64_t val) {
	MemOp_BoundsCheck(vm, addr);
	if ( opt == rbyte ) return vm->code[addr];
	if ( opt == wbyte ) { vm->code[addr] = val; return 1; }
	MemOp_BoundsCheck(vm, addr+8);
	if ( opt == read ) return *(int64_t*)(&(vm->code[addr]));
	if ( opt == write ) { *(int64_t*)(&(vm->code[addr])) = val; return 1; }
	return E_HUH;
}

rsis_instruction _Fetch(rsis_vm* vm, int64_t addr) {
	rsis_instruction _Ret = {
		vm->memop(vm, read, addr, 0),
		vm->memop(vm, read, addr+8, 0),
		vm->memop(vm, read, addr+16, 0)
	};
	return _Ret;
}
boolean initvm(rsis_vm* vm, unsigned char *code, int64_t codesize) {
	vm->codesize = codesize;
	vm->code = code;
	vm->fetch = _Fetch;
	vm->memop = DefaultMemOp;
	vm->pc = 0;
	vm->sp = vm->codesize - 4096;
	vm->bp = vm->codesize - 4096;
	vm->dbg = 0;
}
boolean vmraise(rsis_vm* vm, int intn) {
	printf("Raised int %d\n", intn);
	return false;
}
boolean vmpush(rsis_vm* vm, int64_t n) {
	vm->memop(vm, write, vm->sp, n);
	vm->sp += 8;
	return true;
}
int64_t vmpop(rsis_vm* vm) {
	vm->sp -= 8;
	if ( vm->sp < vm->bp ) { vmraise(vm, 2); return -1; }
	return vm->memop(vm, read, vm->sp, 0);
}
int64_t vmpop__NoUnderFlow(rsis_vm* vm) {
	vm->sp -= 8;
	//if ( vm->sp < vm->bp ) { vmraise(vm, 2); return -1; }
	return vm->memop(vm, read, vm->sp, 0);
}
#define CheckRegBoundsOrHalt(p) if ( p < 0 || p > 25 ) { return vmraise(vm, 1); }
boolean step(rsis_vm* vm) {
	rsis_instruction inst = vm->fetch(vm, vm->pc);
	vm->pc+=24;
//	printf("%x--\n", inst.opcode);
	#ifndef RELEASE
		int64_t i = 0;
		for ( i = 0; i < sizeof(_ops)/sizeof(optable); i++ ) {
	//		printf("%s\n", _ops[i].name);
			if ( _ops[i].opcode == inst.opcode ) {
	printf("%x: ", vm->pc-24);
printf("%s %x, %x\n", _ops[i].name, inst.param1, inst.param2);
				break;
			}
		}
	#endif
	if (vm->dbg > 0) { printf("%d\n", vm->dbg); vm->dbg = 0; }
	if (inst.opcode == nop) return true;
	if (inst.opcode == hlt) return false;
	if (inst.opcode == set) {
		CheckRegBoundsOrHalt(inst.param1);
		vm->registers[inst.param1] = inst.param2;
		return true;
	}
	if (inst.opcode == jmp || inst.opcode == jt || inst.opcode == jf || inst.opcode == call) {
		CheckRegBoundsOrHalt(inst.param1);
		if ( inst.opcode == jf ) vm->zf = !vm->zf;
		if ( inst.opcode == jt ) {
			if ( !vm->zf ) { vm->zf = 0; return true; }
			vm->zf = 0;
		}
		if ( inst.opcode == call ) {
		vmpush(vm, vm->pc);
		vmpush(vm, vm->bp);
		vm->bp = vm->sp;
		}
		vm->pc = vm->registers[inst.param1];
		return true;
	}
	if ( inst.opcode == mov ) {
		CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2);
		vm->registers[inst.param1] = vm->registers[inst.param2];
		return true;
	}
	if ( inst.opcode == ret ) { 
		vm->sp = vm->bp;
		vm->bp = vmpop__NoUnderFlow(vm);
		vm->sp = vm->bp + 8;
		vm->pc = vmpop__NoUnderFlow(vm); 
	#ifndef RELEASE
		printf("Return to %x, Bp=%x, Sp=%x\n", vm->pc, vm->bp, vm->sp);
	#endif
		return true;
	 }
	if ( inst.opcode == rm64 ) { CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); vm->registers[inst.param1] = vm->memop(vm, read, vm->registers[inst.param2], 0); return true;}
	if ( inst.opcode == rm8 ) { CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); vm->registers[inst.param1] = vm->memop(vm, rbyte, vm->registers[inst.param2], 0); return true;}
	if ( inst.opcode == wm64 ) { CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); vm->memop(vm, write, vm->registers[inst.param1], vm->registers[inst.param2]); return true; }
	if ( inst.opcode == wm8 ) { CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); vm->memop(vm, wbyte, vm->registers[inst.param1], vm->registers[inst.param2]); return true; }
	#define _Internal_EmitArithOpcode(oc, act) if ( inst.opcode == oc ) { \
		CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); \
		vm->registers[inst.param1] act vm->registers[inst.param2]; return true; }
	_Internal_EmitArithOpcode(add, +=); _Internal_EmitArithOpcode(mul, *=);
	_Internal_EmitArithOpcode(sub, -=); _Internal_EmitArithOpcode(_div, /=);
	_Internal_EmitArithOpcode(not, =!);
	#define _Internal_EmitCmpOpcode(oc, act) if ( inst.opcode == oc ) { \
		CheckRegBoundsOrHalt(inst.param1); CheckRegBoundsOrHalt(inst.param2); \
		vm->zf = vm->registers[inst.param1] act vm->registers[inst.param2]; return true; }
	_Internal_EmitCmpOpcode(eq, ==);
	_Internal_EmitCmpOpcode(lt, <);
	_Internal_EmitCmpOpcode(gt, >);
	_Internal_EmitCmpOpcode(neq, !=);
	return vmraise(vm, 3);
}

#ifdef TEST
#include <stdio.h>
#ifdef WIN32
#include "winsdl/SDL2-2.0.5/i686-w64-mingw32/include/SDL2/SDL.h"
#else
#include "SDL.h"
#endif
#undef main
#include "font8x8_basic.h"
SDL_Surface *globalScr;
void put_pixel( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;
    
    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}
unsigned char codebuf[(1024*1024*8)];
int64_t unicode = 0;
int offx = 0;
int offy = 0;
boolean ScrPutChr( char v ) {
	//printf("put: %c", v);
    int x,y;
    int set;
    int mask;
    char *bitmap = font8x8_basic[v];
        for (y=0; y < 8; y++) {
    for (x=0; x < 8; x++) {
            set = bitmap[y] & 1 << x;
            //printf("%c", set ? 'X' : ' ');
	    if ( set ) {
		//put_pixel(globalScr, x, y, 0xFFFFFFFF);
		*(uint32_t*)&codebuf[(24576 + (((( (y+offy) * globalScr->w) + (x + offx)) * 4)))] = 0xFFFFFFFF;
	    }
        }
        
    }
}
int64_t MemOpIO ( rsis_vm* vm, mem_mode opt, int64_t addr, int64_t value ) {
	if ( addr == -1 ) {
		int tmp = unicode;
		unicode = 0;
		return tmp;
	}
	if ( addr == -2 && opt == wbyte ) {
		return (int64_t)ScrPutChr(value);	
	}
	if ( addr == -3 && opt == write ) {
		offy = value;
		return 1;
	}
	if ( addr == -4 && opt == write ) {
		offx = value;
		return 1;
	}
//	printf("addr=%ld\n", addr);
	if ( addr >= 0 ) return DefaultMemOp(vm, opt, addr, value);
	return -1;
}
int main(int argc, char **argv) {
	char timer = 0;
	setbuf(stdout, NULL);
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Surface* screen = NULL;
	SDL_Window* window = NULL;
	window = SDL_CreateWindow( "SDL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 512, SDL_WINDOW_SHOWN );
	screen = SDL_GetWindowSurface(window);
	globalScr = screen;
	if ( argc < 2 ) abort();
	FILE *fp = fopen(argv[1], "rb");
	if ( !fp ) abort();
	fread(codebuf, 1, sizeof(codebuf), fp);
	fclose(fp);
	rsis_vm vm;
	initvm(&vm, codebuf, sizeof(codebuf));
	vm.memop = MemOpIO;
	while (step(&vm) == true) {
		int x; int y;
		for ( y = 0; y < screen->h; y++ ) {
			for ( x = 0; x < screen->w; x++ ) {
put_pixel(screen, x, y, *(uint32_t*)&codebuf[(24576 + ((((y * screen->w) + x) * 4)))]);
			}
		}
		timer++;
		SDL_Event event;
		SDL_PollEvent(&event);
		if ( event.type == SDL_KEYDOWN ) {
			unicode = event.key.keysym.sym;
		}
		SDL_UpdateWindowSurface(window);
	}
	SDL_Quit();
	
}
#endif
