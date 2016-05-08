/*
dos2unix implementation.
Copyright 2016 Sam Saint-Pettersen.

Ported to C from C++.

Released under the MIT License;
see MIT-LICENSE file.
*/

#include <stdio.h>
#include <string.h>

int is_ascii (char* filename) {
  int ascii = 1;
  int c = -1;
  char l[1000];
  FILE* f;
  f = fopen(filename, "rb");
  fgets(l, 1000, f);
  int i;
  for (i = 0; i < strlen(l); i++) {
    if (l[i] > 127 || l[i] == 0x01) {
      ascii = 0;
      break;
    }
  }
  fclose(f);
  return ascii;
}

int is_dos_eol (char* filename) {
  int dos_eol = 0;
  char l[1000];
  FILE* f;
  f = fopen(filename, "rb");
  fgets(l, 1000, f);
  int i;
  for(i = 0; i < strlen(l); i++) {
    if (l[i] == '\r') {
      dos_eol = 1;
      break;
    }
  }
  return dos_eol;
}

void to_unix_line_endings(char* contents, char* ucontents) {
	int i, j;
	int length = strlen(contents);
	for (j = i = 0; i < length; i++) {
		if(contents[i] != '\r') {
			ucontents[j++] = contents[i];
		}
	}
	ucontents[j] = 0;
}

void dos2unix_str(char* filename, char* ucontents) {
  char contents[100000];
  FILE* f;
  f = fopen(filename, "rb");
  fread(contents, 1, 100000, f);
  fclose(f);
  if (is_ascii(filename) == 1 && is_dos_eol(filename) == 1) {
    to_unix_line_endings(contents, ucontents);
    return;
  }
  strcpy(ucontents, contents);
}

int dos2unix(char* filename) {
  if (is_ascii(filename) == 1 && is_dos_eol(filename) == 1) {
    char contents[100000];
    char out[100000];
    FILE* f;
    f = fopen(filename, "rb");
    fread(contents, 1, 100000, f);
    fclose(f);
    to_unix_line_endings(contents, out);
    f = fopen(filename, "wb");
    fwrite(out, 1, strlen(out), f);
    fclose(f);
    return 0;
  }
  return 1;
}
