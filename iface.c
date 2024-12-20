#include "Utransmitter.h"
#include "alsa/alsa.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argn, char* argv[]){
  if(argn < 5){
    printf("%s <input device> <output device> <frequency_khz> <mode r/t> [buffer size] [gain] [sample rate] [buffer size]\n", argv[0]);
    printf("mode r - AM demodulate| mode b - DSB demodulate | mode t - transmit | mode d - DSB tx | good gain values are like 400\n");
    return 0;
  }
  int bsize = 1500;

  int samplerate = 48000;

  if(argv[7] != NULL){
    samplerate = atoi(argv[7]);
  }

  if(argv[5] != NULL)
    bsize = atoi(argv[5]);

  if(bsize < 1000)
    bsize = 1000;


  if(samplerate < 48000)
    samplerate = 48000;

  init_f_manager(bsize, samplerate);

  double gain = 1;


  if(argv[6] != NULL)
    gain = atof(argv[6]);
  if(gain <= 0)
    gain = 1;

  if(argv[4][0] == 'r' ||argv[4][0] == 'b'){
    if(gain == 1)
      gain = 400;
  }


  double frequency = atof(argv[3]);
  printf("gain: %f, buffer size: %d, rate: %d, frequency: %fkhz\n",gain, bsize,samplerate,frequency);

  int status = setup_alsa(argv[1], argv[2], bsize, samplerate);
  
  if( status < 0)
    return 0;

  short* input = malloc(sizeof(short) * (bsize));
  int* output_t = malloc(sizeof(int) * (bsize));
  short* output = malloc(sizeof(short) * (bsize));
  int error = 0;
  sync_interfaces();
  while( error != -1){
    error = aread(input);

    if(argv[4][0] == 'r')
      amplitude_demodulate(input,output_t,bsize,frequency, gain);
    else if(argv[4][0] == 'b')
      DSB_demodulate(input,output_t,bsize,frequency, gain);

    else if (argv[4][0] == 'd')
      DSB_modulate(input,output_t,bsize,frequency,gain);
    else
      amplitude_modulate(input, output_t,bsize,frequency,gain);

    declip(output_t,bsize);
    
    int_to_short_buff(output_t,bsize,output);
    //sync_record(record_sleep);

    awrite(output);

  }
  printf("error\n");

  clean_f_manager();
  cleanLPF();

  free(input);
  free(output);
  free_alsa();

  return 0;
}
