/*
  tarino: a tar implementation.
  Port of original Node.js add-on C++ code to ANSI C.

  Dual licensed unde the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#ifndef __TARINO
#define __TARINO

int write_tar_entry(char*, char*);
int write_tar_entries(char*, char*);
int extract_tar_entries(char*, int, int);
int list_tar_entries(char*, int);

#endif
