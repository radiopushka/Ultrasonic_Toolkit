#include "Utransmitter.h"
#include "wav.c"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argn, char* argv[]){
  if(argn < 5){
    printf("%s <input file (wav)> <output file (wav)> <frequency_khz> <mode r/t> [gain]\n", argv[0]);
    printf("mode r - reception | mode t - transmit | mode d - DSB tx| good gain values are like 400\n");
    printf("the sample rate will mirror the input file\n");
    return 0;
  }


  FILE* in = fopen(argv[1] , "rb");
  FILE* out = fopen(argv[2] , "wb");
  read_wav_header_skip(in);
  
  int samplerate = rate_file;


  init_f_manager(1500, samplerate);
  srate = samplerate;

  double gain = 1;


  if(argv[5] != NULL)
    gain = atof(argv[5]);
  if(gain <= 0)
    gain = 1;

  double frequency = atof(argv[3]);

  char chn = channels;
  printf("size: %d, channels: %d, rate: %d\n",file_size, channels, samplerate);
  channels = 1;
  if( chn == 2 ){
    file_size = file_size >> 1;
  }

  write_wav_header(out);




  short* input = malloc(sizeof(short) * (1500));
  short* dchannel;
  short* dchannel2;
  if( chn == 2 ){
    dchannel = malloc(sizeof(short) * 1500);
    dchannel2 = malloc(sizeof(short) * 1500);
  }
  short* output = malloc(sizeof(short) * (1500));
  int error = 0;
  unsigned int i;
  int i2;
  unsigned int nfilecount = (file_size >> 1)/1500;
  for(i = 0; i < nfilecount; i++){

    fread(input, sizeof(short), 1500, in);
    if(chn == 2){
      fread(dchannel, sizeof(short), 1500, in);
      int dch = 0;
      for(i2 = 1; i2 < 1500; i2 = i2 + 2){
        int prev = input [i2 -1];
        int this = input [i2];
        int tmp = (prev + this)/2;
        dchannel2[dch] = tmp;
        dch++;

      }
      for(i2 = 1; i2 < 1500; i2 = i2 + 2){
        int prev = dchannel [i2 -1];
        int this = dchannel [i2];
        int tmp = (prev + this)/2;
        dchannel2[dch] = tmp;
        dch++;

      }
      for(i2 = 0; i2 < 1500; i2++){
        input [i2] = dchannel2[i2];
      }

    }

    if(argv[4][0] == 'r')
      amplitude_demodulate(input,output,1500,frequency,gain);
    else if (argv[4][0] == 'd')
      DSB_modulate(input,output,1500,frequency,gain);
    else
      amplitude_modulate(input, output,1500,frequency,gain);

    fwrite(output, sizeof(short), 1500, out);


  }

  clean_f_manager();
  cleanLPF();

  free(input);
  fclose(in);
  fclose(out);
  free(output);
  if(chn == 2){
    free(dchannel2);
    free(dchannel);
  }

  return 0;
}
