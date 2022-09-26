#ifndef AUDIO_H
#define AUDIO_H

#include "MKL25Z4.h"

void initBuzzer(void);
void pauseTune(int);
void playMain(int);
void playStart(void);
void playCelebrate(void);

#endif
