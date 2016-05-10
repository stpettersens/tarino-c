/*
  tarino: a tar implementation.
  Port of original Node.js add-on C++ code to C.

  Dual licensed unde the GPL and MIT licenses;
  see GPL-LICENSE and MIT-LICENSE respectively.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "dos2unix.h"

static int EOF_PADDING = 512;

struct tar_entry {
  char* part;
  char* file;
  int size;
  int modified;
  int etype;
};

void str_replace (char* str, char c, char r, char* rstr) {
  char new_str[1000];
  int length = strlen(str);
  int i;
  for (i = 0; i < length; i++) {
    if (str[i] == c) {
      new_str[i] = r;
    } else {
      new_str[i] = str[i];
    }
  }
  strcpy(rstr, new_str);
}

void split (char* str, char* delimiter, char* out1, char* out2) {
  char strs[2][100];
  char* token;
  int i = 0;
  token = strtok(str, delimiter);
  while (token != 0) {
    strcpy(strs[i], token);
    i++;
    token = strtok(0, delimiter);
  }
  strcpy(out1, strs[0]);
  strcpy(out2, strs[1]);
}

int get_gif_length (char* contents) {
  int i, length = 0;
  for (i = 0; i < 100000; i++) {
    length = i += 3;
    if (contents[i-1] == (char)0x00 &&
    contents[i] == (char)0x3B) {
      length = i += 1;
      break;
    }
  }
  return length;
}

int get_png_length (char* contents) {
  int i, length = 0;
  for (i = 0; i < 100000; i++) {
    length = i += 1;
    if (contents[i-1] == (char)0x60 &&
    contents[i] == (char)0x82)  {
      length = i += 1;
      break;
    }
  }
  return length;
}

void to_date_time (int timestamp, char* datetime) {
  time_t rawtime = timestamp;
  memset(datetime, 0, 24);
  strncpy(datetime, ctime(&rawtime), 24);
}

void pad_str (char* data, int length, char* pdata) {
  char padded[100];
  int i;
  int dlz = 0;
  for (i = 0; i <= length; i++) {
    if (dlz == 0 && data[i] == '0') {
      padded[i] = (char)32;
    } else if (dlz == 1 || data[i] != '0') {
      dlz = 1;
      padded[i] = data[i];
    }
  }
  if (padded[length - 1] == (char)32) {
    padded[length] = '0';
  }
  strcpy(pdata, padded);
}

int file_exists (char* filename) {
  struct stat st;
  int result = stat(filename, &st);
  return result;
}

int get_file_modified (char* filename) {
  struct stat st;
  stat(filename, &st);
  return st.st_mtime;
}

int get_file_size (char* filename) {
  struct stat st;
  stat(filename, &st);
  return st.st_size;
}

void dec_to_octal (unsigned long long dec, char* octal) {
  char sdec[12] = {0};
  char soctal[12] = {0};
  int d[12] = {0};
  int i;
  unsigned long long working = dec;
  sprintf(sdec, "%llu", dec);
  while (working > 0) {
    unsigned long long w = floor(working / 8);
    d[i] = working % 8;
    working = w;
    i++;
  }
  while (i >= 0) {
    char c[2];
    sprintf(c, "%d", d[i]);
    if (i <= (int)strlen(sdec)) {
      strcat(soctal, c);
    }
    i--;
  }
  strcpy(octal, soctal);
}

int octal_to_dec (char* octal) {
  int length = strlen(octal);
  int dec = 0;
  int i, z = 0;
  for(i = length; i > 0; i--) {
    int d = octal[i - 1] - '0';
    dec += d * pow(8, z);
    z++;
  }
  return dec;
}

void padded_octal (char* octal, int length, char* ppoctal) {
  char poctal[100];
  if (length > (int)strlen(octal)) {
    int i;
    for (i = 0; i < (length - (int)strlen(octal)); i++) {
      strcat(poctal, "0");
    }
  }
  strcat(poctal, octal);
  strcpy(ppoctal, poctal);
}

int get_padding (char* data) {
  int eof = 0;
  int m = 1;
  int length = strlen(data);
  while (eof < length) {
    eof = EOF_PADDING * m;
    if (length <= eof) {
      break;
    }
    m++;
  }
  return eof - (length - 1);
}

void put_padding (int length, FILE* f) {
  int i;
  for (i = 0; i < length; i++) {
    fputc(0x00, f);
  }
}

void calc_checksum (char* header, char* pchecksum) {
  int checksum = 0;
  char ochecksum[100];
  int i;
  for(i = 0; i < 265; i++) {
    checksum += (int)header[i];
  }
  dec_to_octal(checksum - 64, ochecksum);
  padded_octal(ochecksum, 6, pchecksum);
}

void p_write_tar_entry (char* tarname, char* filename, int _size, int _modified, int etype) {
  char size[11];
  char psize[11];
  char modified[10];
  char pmodified[10];
  char fm[7];
  char contents[100000];

  dec_to_octal(_size, size);
  padded_octal(size, 11, psize);

  if (etype == 0) {
    strcpy(fm, "0100777");
    dos2unix_str(filename, contents);
  } else if (etype == 5) {
    strcpy(fm, "0040777");
  }
  /*
    * TAR FORMAT SPECIFICATION
    * (a) File name (0-)
    * (b) File mode (100; 8)
    * (c) Owner's numeric user ID (108; 8)
    * (d) Group's numeric user ID (116; 8)
    * (e) File size in bytes (octal) (124; 12)
    * (f) Last modificaton time in numeric Unix time format (octal) (136; 12)
    * (g) Checksum for header record (148; 8)
    * (h) Link indicator (file type) (156; 1)
    * (i) UStar indicator (257; 6)
  */

  FILE* tar;
  tar = fopen(tarname, "wb");
  fwrite(filename, 1, strlen(filename), tar);
  put_padding(100 - strlen(filename), tar);
  fputs(fm, tar);
  put_padding(1, tar);
  fputs("0000000", tar);
  put_padding(1, tar);
  fputs("0000000", tar);
  put_padding(1, tar);
  fputs(psize, tar);
  dec_to_octal(_modified, modified);
  padded_octal(modified, 0, pmodified);
  put_padding(1, tar);
  fputs(pmodified, tar);
  put_padding(1, tar);
  fputs("000000", tar);
  put_padding(1, tar);
  fputs(" ", tar);
  char cetype[2];
  sprintf(cetype, "%d", etype);
  fputs(cetype, tar);
  put_padding(100, tar);
  fputs("ustar", tar);
  put_padding(1, tar);
  fputs("00", tar);
  put_padding(247, tar);
  fwrite(contents, 1, strlen(contents), tar);
  int ps = get_padding(contents);
  put_padding(ps, tar);
  put_padding((EOF_PADDING * 2) - 1, tar);
  fclose(tar);
}

// void p_write_tar_entries (char* filename, struct tar_entry entry);

void write_checksum (char* tarname, int etype) { //, char* rheader) {
  char checksum[7];
  char header[265];
  FILE* tar;
  tar = fopen(tarname, "rb+");
  fread(header, 265, 1, tar);
  calc_checksum(header, checksum);
  fseek(tar, 148, SEEK_SET);
  fputs(checksum, tar);
  fclose(tar);
  //strcpy(rheader, header);
}

char* merge_entries (char* tarname, char** entries) {
  // !TODO
}

int write_tar_entry (char* tarname, char* filename) {
  //char header[265];
  int size = get_file_size(filename);
  int modified = get_file_modified(filename);
  p_write_tar_entry(tarname, filename, size, modified, 0);
  write_checksum(tarname, 0); //, header);
  return 0;
}

int write_tar_entries (char* tarname, char* manifest) {
  struct tar_entry entry;
  char line[10000];
  char unused[50];
  char tentry[50];
  char part[50];
  char file[50];
  FILE* f;
  f = fopen(manifest, "r");
  while (fgets(line, sizeof(line), f)) {
    split(line, "\n", tentry, unused);
    split(tentry, ":", part, file);
    entry.part = part;
    entry.file = file;
    entry.size = get_file_size(file);
    entry.modified = get_file_modified(file);
    printf("%d\n", entry.size);
    // p_write_tar_entries(tarname, entry);
  }
  fclose(f);
  return 0;
}

void extract_entry (char* tarname, int i, int overwrite, int verbose, int extract) {
  char filename[100];
  char mode[8];
  char owner[8];
  char group[8];
  char size[12];
  char modified[12];
  char checksum[8];
  char type[1];

  FILE* tar;
  tar = fopen(tarname, "rb");
  fseek(tar, i, SEEK_SET);
  fgets(filename, 99, tar);
  fseek(tar, i + 100, SEEK_SET);
  fgets(mode, 8, tar);
  fseek(tar, i + 108, SEEK_SET);
  fgets(owner, 8, tar);
  fseek(tar, i + 116, SEEK_SET);
  fgets(group, 8, tar);
  fseek(tar, i + 124, SEEK_SET);
  fgets(size, 12, tar);
  fseek(tar, i + 136, SEEK_SET);
  fgets(modified, 12, tar);
  fseek(tar, i + 148, SEEK_SET);
  fgets(checksum, 8, tar);
  fseek(tar, i + 156, SEEK_SET);
  fgets(type, 2, tar);
  char contents[10000];
  fseek(tar, i + 512, SEEK_SET);
  fread(contents, 10000, 1, tar);
  fclose(tar);

  if (verbose == 1 && extract == 1) {
    printf("%s\n", filename);
  }
  else {
    char smodified[25];
    char ssize[12];
    pad_str(size, 12, ssize);
    to_date_time(octal_to_dec(modified), smodified);
    printf("%d    %s      %s    %s\n",
    atoi(mode), ssize, smodified, filename);
  }

  if (strcmp(type, "5") == 0 && extract == 1) {
    char command[1000];
    strcat(command, "mkdir ");
    #ifdef __unix__
    strcat(command, "-p ");
    strcat(command, filename);
    #else
    char wfilename[1000];
    str_replace(filename, '/', '\\', wfilename);
    strcat(command, wfilename);
    if (file_exists(filename) == 0) {
    #endif
      int ec = system(command);
      if (ec == 1) {
        printf("tarino-native: Exit code from system call was 1.\n");
      }
    #ifndef __unix__
    }
    #endif
    memset(command, 0, strlen(command));
  }

  if (strcmp(type, "0") == 0 && extract == 1) {
    if (overwrite == 1 || file_exists(filename) == 1) {
      FILE* out;
      out = fopen(filename, "wb");
      int length = strlen(contents);
      if (strstr(filename, ".gif") != 0) {
        length = get_gif_length(contents);
      }
      if (strstr(filename, ".png") != 0) {
        length = get_png_length(contents);
      }
      fwrite(contents, length, 1, out);
      fclose(out);
    }
  }
}

void get_entry_offsets (char* tarname, int* roffsets) {
  int offsets[100000];
  char magic[5];
  int oi, i, size = 0;
  FILE* tar;
  tar = fopen(tarname, "rb");
  fseek(tar, 0, SEEK_END);
  size = ftell(tar);
  rewind(tar);
  for (i = 257; i <= size; i++) {
    fseek(tar, i, SEEK_SET);
    fgets(magic, 6, tar);
    if(strcmp(magic, "ustar") == 0 || strcmp(magic, "ustar ") == 0) {
      offsets[oi] = (i + 249) - 506;
      oi++;
    }
  }
  fclose(tar);
  //return offsets;
  memcpy(roffsets, offsets, 10000);
}

int get_entries (int* offsets) {
  int i, entries = 0;
  for(i = 0; i < 10000; i++) {
    if (i > 0 && offsets[i] == 0) break;
    entries++;
  }
  return entries;
}

int extract_tar_entries (char* tarname, int overwrite, int verbose) {
  if (file_exists(tarname) == -1) {
    printf("tarino-native: Archive \'%s\' does not exist.\n", tarname);
    return -1;
  }

  int offsets[10000];
  //int* offsets =
  get_entry_offsets(tarname, offsets);
  int entries = get_entries(offsets);
  if (verbose == 1) {
    printf("tarino-native: Extracting %d entries from archive.\n\n", entries);
  }
  int i;
  for (i = 0; i < entries; i++) {
    extract_entry(tarname, offsets[i], overwrite, verbose, 1);
  }
  return 0;
}

int list_tar_entries (char* tarname, int verbose) {
  if (file_exists(tarname) == -1) {
    printf("tarino-native: Archive \'%s\' does not exist.\n", tarname);
    return -1;
  }

  int offsets[10000];
  //int* offsets =
  get_entry_offsets(tarname, offsets);
  int entries = get_entries(offsets);
  if (verbose == 1) {
    printf("tarino-native: Listing %d entries from archive.\n\n", entries);
  }
  int i;
  for (i = 0; i < entries; i++) {
    extract_entry(tarname, offsets[i], 0, verbose, 0);
  }
  return 0;
}
