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
  char c = getchar();
  char * character = malloc(sizeof(c));
  *character = c;
  if( c == EOF)
	  return("");
  else
  	return(character);
}

int32_t __ord(const char *s) {
  if(*s == 0)
	  return(-1);
  return((int32_t)* s);
}

const char *__chr(int32_t i) {
  if(i == 0)
	  return("");
  if(i < 0 || i > 255)
	  error("ASCII character must be between 0 and 255");
  if(i > 128)
	  return( (char*) 255 - i);
  else
	  return( (char*) 0 -(-i));
}

int32_t __size(const char *s) {
  return(strlen(s));
}

const char *__substring(const char *s, int32_t first, int32_t length) {
  int size = __size(s);
  if( length < 0 || first < 0 || first + length > size)
	  error("Impossible to get a substring: index out of bounds or negative parameter");

}

const char *__concat(const char *s1, const char *s2) {
  char * tmp ;
  tmp = malloc(sizeof(s1)+sizeof(s2));
  strcpy(tmp, s1);
  return(strcat(tmp, s2));
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
