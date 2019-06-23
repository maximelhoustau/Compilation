#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

__attribute__((noreturn))
static void error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}

void __print_err(const char *s) {
  fprintf(stderr, "%s", s);
}

void __print(const char *s) {
  printf("%s", s);
}

void __print_int(const int32_t i) {
  printf("%i", i);
}

void __flush(void) {
  fflush(stdout);
}

const char *__getchar(void) {
  char character = getchar();
  if(character != EOF)
	 return(character);
  else
	  return("\0");
}

int32_t __ord(const char *s) {
  if(s != "\0")
	  return((int32_t) s);
}

const char *__chr(int32_t i) {
  error("UNIMPLEMENTED __chr");
}

int32_t __size(const char *s) {
  return(strlen(s));
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  error("UNIMPLEMENTED __substring");
}

const char *__concat(const char *s1, const char *s2) {
  return(strcat(s1, s2));
}

int32_t __strcmp(const char *s1, const char *s2) {
  int32_t result = strcmp(s1, s2);
  if( result > 0)
	  return(1);
  else if(result < 0)
	  return(-1);
  else
	  return(0);
}

int32_t __streq(const char *s1, const char *s2) {
  int32_t result = __strcmp(s1, s2);
  if(result == 0)
	  return(1);
  else
	  return(0);

}

int32_t __not(int32_t i) {
  return(!i);
}

void __exit(int32_t c) {
  exit(c);
}
