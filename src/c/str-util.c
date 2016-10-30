#include <pebble.h>
#include "src/c/str-util.h"

char lower_to_upper(char c) {
  return (c>='a' && c<='z') ? c&0xdf : c;
}

void str_to_upper(char* str)  {
  for(char* pc=str;*pc!=0;++pc) 
  {
    *pc = lower_to_upper(*pc);
  }
}
