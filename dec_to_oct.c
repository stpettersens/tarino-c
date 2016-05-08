void dec_to_octal (unsigned long long dec, int length, char* output) {
  char d[12];
  char octal[12];
  int o[12];
  unsigned long long working = dec;
  int i = 0;
  while (working > 0) {
    int w = floor(working / 8);
    o[i] = working % 8;
    working = w;
    i++;
  }
  while (i >= 0) {
    itoa(o[i], d, 10);
    if (i < length) {
      strcat(octal, d);
    }
    i--;
  }
  strcpy(output, octal);
}
