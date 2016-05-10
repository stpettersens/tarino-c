#include <stdio.h>
#include "tarino.h"

int main(void) {
  printf("\n");
  //extract_tar_entries("lua-5.3.2.tar", 1, 1);
  //list_tar_entries("tarino.tar", 1);
  //write_tar_entries("tarino.tar", "tarino.entries");
  #ifdef __unix__
  write_tar_entry("tarino.tar", "Makefile");
  #else
  system("7z u -ttar tarino.tar Makefile"); // Use 7-zip until fixed on Windows.
  printf("\n");
  #endif
  list_tar_entries("tarino.tar", 1);
  return 0;
}
