#include "Utransmitter.h"
#include "alsa/alsa.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argn, char* argv[]){
  if(argn < 5){
    printf("%s <input device> <output device> <frequency_khz> <mode r/t> [buffer size] [gain] [sample rate] [buffer size]\n", argv[0]);
    printf("mode r - reception | mode t - transmit | mode d - DSB tx | good gain values are like 400\n");
    return 0;
  }
  int bsize = 1500;

  int samplerate = 48000;

  if(argv[7] != NULL){
    samplerate = atoi(argv[7]);
  }

  if(argv[5] != NULL)
    bsize = atoi(argv[5]);

  if(bsize < 1500)
    bsize = 1500;


  if(samplerate < 48000)
    samplerate = 48000;

  int record_sleep = bsize*10;
  init_f_manager(bsize, samplerate);

  double gain = 1;


  if(argv[6] != NULL)
    gain = atof(argv[6]);
  if(gain <= 0)
    gain = 1;


  double frequency = atof(argv[3]);
  printf("gain: %f, buffer size: %d, rate: %d, frequency: %fkhz\n",gain, bsize,samplerate,frequency);

  int status = setup_alsa(argv[1], argv[2], bsize, samplerate);

  if( status < 0)
    return 0;

  short* input = malloc(sizeof(short) * (bsize));
  short* output = malloc(sizeof(short) * (bsize));
  int error = 0;
  while( error != -1){
    error = aread(input);

    if(argv[4][0] == 'r')
      amplitude_demodulate(input,output,bsize,frequency, gain);
    else if (argv[4][0] == 'd')
      DSB_modulate(input,output,bsize,frequency,gain);
    else
      amplitude_modulate(input, output,bsize,frequency,gain);
    sync_record(record_sleep);

    awrite(output);

  }

  clean_f_manager();
  cleanLPF();

  free(input);
  free(output);
  free_alsa();

  return 0;
}
