#include "Utransmitter.h"
#include "alsa_pipe/alsa_pipe.h"
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

  if(bsize < 1500)
    bsize = 1500;


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

  int channelsin=1;
  int channelsout=1;

  int rate_in=samplerate;
  int rate_out=samplerate;

  int status = setup_alsa_pipe(argv[1], argv[2],&channelsin, &channelsout,&rate_in,&rate_out,bsize);
  
  if( status < 0)
    return 0;

  printf("input channels: %d output: %d\n",channelsin,channelsout);

  if(channelsin>1 || channelsout>1){
    printf("please get ALSA do down mix both ports to mono\n");
    printf("currently down mixing is not supported\n");
    printf("but if you want to help, by all means!\n");
    return 0;
  }

  short* input = malloc(sizeof(short) * (bsize));
  int* output_t = malloc(sizeof(int) * (bsize));
  short* output = malloc(sizeof(short) * (bsize));
  short* output_e=output+bsize;
  short caseiv[bsize];
  short caseiv2[bsize];
  int error = 0;
  while( error != -1){
    error = get_audio(input);

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

    //channel mixing
    if(channelsin==1 && channelsout==2){
       int count=0;
       for(short* tbuff=output;tbuff<output_e;tbuff++){
        if(count<bsize){
          caseiv[count++]=*tbuff;
          caseiv[count++]=*tbuff;
        }else{
          caseiv2[bsize-(count++)]=*tbuff;
          caseiv2[bsize-(count++)]=*tbuff;

        }
      }
      queue_audio(caseiv);
      queue_audio(caseiv2);
    }else{
      queue_audio(output);
    }

  }
  printf("error\n");

  clean_f_manager();

  free(input);
  free(output);
  alsa_pipe_exit();

  return 0;
}
