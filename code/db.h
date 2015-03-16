#ifndef DB_DEFS
#define DB_DEFS


#include "printf.h"

extern void db(void);
void hexdump(char* buf);
bool process_input(int* registers, char* buf);
void backtrace(int* registers);

#endif
