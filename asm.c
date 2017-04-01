#include <stdio.h>
#include <string.h>
#include "vm.h"
/* Ronsor's Macro Assembler */

typedef struct _optable {
	char name[8];
	int64_t params;
	int64_t opcode;
} optable;

optable _ops[] = {
	{"hlt", 0, hlt},
	{"set", 2, set},
	{"nop", 0, nop},
	{"rm64", 2, rm64},
	{"rm8", 2, rm8},
	{"not", 2, not},
	{"ret", 0, ret},
	{"gt", 2, gt},
	{"lt", 2, lt},
	{"eq", 2, eq},
	{"neq", 2, neq},
	{"wm64", 2, wm64},
	{"wm8", 2, wm8},
	{"add", 2, add},
	{"sub", 2, sub},
	{"mul", 2, mul},
	{"div", 2, _div},
	{"mov", 2, mov},
	{"push", 1, push},
	{"pop", 1, pop},
	{"call", 1, call},
	{"jmp", 1, jmp},
	{"jt", 1, jt},
	{"jf", 1, jf}
};

typedef struct _symbol {
	char name[512];
	int64_t ptr;
} symbol;
#ifndef DEBUG_INCLUDE
symbol symbols[256];
int nsymbols = 0;
void addsym(int64_t ptr, char* name) {
	symbols[nsymbols].ptr = ptr;
	strcpy(symbols[nsymbols].name, name);
	nsymbols++;
}
int64_t getsym(char* name) {
	int i; for ( i = 0; i < 256; i++ ) {
		if ( !strcmp(symbols[i].name, name) ) return symbols[i].ptr;
	}
	return -1;
}
#define NumOps sizeof(_ops)/sizeof(optable)
FILE *out;
int npass = 1;
int psz = 0;
int getint(char* str) {
//	return 0;
	if ( str[0] == '$' ) {
		return str[1] - 'a';
	}
	if ( str[0] == '@' ) {
		return getsym(&str[1]);
	}
	return strtol(str, NULL, 0);
}
void pass(char* _str) {
  char *str = strdup(_str);
  //printf("%s\n", str);
  char *pch = strtok (str,"\n\t\r");
  char opcode[512];
  char param1[512];
  char param2[512];
  int n;
  int _pc;
  _pc = 0;
  int line = 0;
  while (pch != NULL)
  {
	line++;
	if ( line > 1 ) {
	pch = strtok (NULL, "\r\n\t");
	}
	if ( !pch ) break;
	param1[0] = 0; param2[0] = 0;
	n = sscanf(pch, "%s %[^,], %s", opcode, param1, param2); int i = 0;
	if ( npass > 1 ) printf("%x: %d: %s\n", _pc, line, pch);
	if ( !strcmp(opcode, "dw") ) {
		if ( npass > 1 ) {
		int64_t dat = getint(pch + 3);
		fwrite(&dat, 1, 8, out);
		}
		_pc+=8;
		continue;
	}
	if ( !strcmp(opcode, "org") ) {
		_pc = getint(pch + 3);
		continue;
	}
	if ( !strcmp(opcode, "ds") ) {
		if ( npass > 1 ) {
			fwrite(pch + 3, 1, strlen(pch + 3), out);
		}
		_pc+=strlen(pch + 3);
		continue;
	}
	if ( !strcmp(opcode, "db") ) {
		if ( npass > 1 ) {
			char dat = getint(pch + 3);
			fwrite(&dat, 1, 1, out);
		}
		_pc+=1;
		continue;
	}
	if ( opcode[0] == ':' ) {
	
		if (npass == 1) addsym(_pc, &opcode[1]);
		continue;
	}
	for ( i = 0; i < NumOps; i++ ) {
		if ( !strcmp(_ops[i].name, opcode) ) {
			if ( npass > 1 ) {		
			rsis_instruction _o = { 0, 0, 0 };
			_o.opcode = _ops[i].opcode;
			if ( _ops[i].params >= 1 ) _o.param1 = getint(param1);
			if ( _ops[i].params >= 2 ) _o.param2 = getint(param2);
			fwrite(&_o, 1, sizeof(rsis_instruction), out);
			}
			_pc+=24;
		}
	}
  }
	if ( npass == 1 ) psz = _pc;
	npass++;
}
int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	if ( argc < 3 ) abort();
	FILE *fp = fopen(argv[1], "r");
	if ( !fp ) abort();
	out = fopen(argv[2], "wb");
	if ( !out ) abort();
	char data[65536];
	fread(data, 1, 65536, fp);
	pass(data);
	pass(data);
	int i; for ( i = 0; i < 256; i++ ) {
		if ( symbols[i].name[0] != 0 ) {
		printf("Symbol: %s = %ld\n", symbols[i].name, symbols[i].ptr);
		}
	}
	fclose(out);
	return 0;
}
#endif
