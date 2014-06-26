//extern "C" {
  #include "stdlib.h"
//}

void randomSeed(unsigned int seed);


long random(long howbig);


long random_maxmin(long howsmall, long howbig);


long map(long x, long in_min, long in_max, long out_min, long out_max);


unsigned int makeWord(unsigned int w);
unsigned int makeWord_hl(unsigned char h, unsigned char l);
