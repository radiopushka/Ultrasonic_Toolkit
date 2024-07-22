#include "Utransmitter.h"
#include "alsa/alsa.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argn, char* argv[]){
  if(argn < 5){
    printf("%s <input device> <output device> <frequency_khz> <mode r/t> [gain] [sample rate]\n", argv[0]);
    printf("mode r - reception | mode t - transmit | mode d - DSB tx | good gain values are like 400\n");
    return 0;
  }

  int samplerate = 48000;

  if(argv[6] != NULL){
    samplerate = atoi(argv[6]);
  }

  init_f_manager(1500, samplerate);

  double gain = 1;


  if(argv[5] != NULL)
    gain = atof(argv[5]);

  double frequency = atof(argv[3]);

  int status = setup_alsa(argv[1], argv[2], 1500, samplerate);

  if( status < 0)
    return 0;

  short* input = malloc(sizeof(short) * (1500));
  short* output = malloc(sizeof(short) * (1500));
  int error = 0;
  while( error != -1){
    error = aread(input);

    if(argv[4][0] == 'r')
      amplitude_demodulate(input,output,1500,frequency, samplerate,gain);
    else if (argv[4][0] == 'd')
      DSB_modulate(input,output,1500,frequency,gain);
    else
      amplitude_modulate(input, output,1500,frequency,gain);

    awrite(output);

  }

  clean_f_manager();
  cleanLPF();

  free(input);
  free(output);
  free_alsa();

  return 0;
}
