#include "Utransmitter.h"
#include "frequencymanager.c"
#include "LPF/high_cut.h"
struct high_cut* filter = NULL;
struct high_cut* filtertx = NULL;

//frequency in khz, 1 is 1 khz
void amplitude_modulate(short* input, short* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = init_highcut(sample_rate,6000,3,3,0.5);
  }

  int i;
  double coeff = gain * 0.25;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }
// amplitude modulation
  double preview;
  for(i = 0; i < length; i++){
    preview = input[i];
    perform_filter(preview,filtertx,&preview);
    output[i] = (periods[freqi][i] * (24575 + preview * coeff));
  }

}

void DSB_modulate(short* input, short* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = init_highcut(sample_rate,6000,3,3,0.5);
  }

  int i;
  double coeff = gain * 0.5;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }
// amplitude modulation
  double preview;
  for(i = 0; i < length; i++){
    preview = input[i];
    perform_filter(preview,filtertx,&preview);
    output[i] = (periods[freqi][i] * (16383 + preview * coeff));
  }

}


double shift_coeff = 0;
double maxg = 0;
double ming = 0;

float again = 1;

void amplitude_demodulate(short* input, short* output, int length, float freq, double gain){


  if(filter == NULL){
    filter = init_highcut(sample_rate,3000,3,3,0.6);
  }

  int i;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }

  double sin;
  double cos;

  double max = -24575;
  double min = 24575;
  double demod;
  double avg = 100000000;

  double output_prev;

  //fourier transform
  for( i = 0; i < length; i++){
    sin = input[i] * periods[freqi][i];
    cos = input[i] * cosine[freqi][i];

    demod = sin + cos;

    if(demod > max)
      max = demod;

    if(demod < min)
      min = demod;

    

    output_prev = ( (demod + shift_coeff) * again );
    perform_filter(output_prev,filter,&output_prev);
    output[i] = output_prev;

    if(avg == 100000000){
      avg = abs(output_prev);
    }else{
      avg = (avg + abs(output_prev))/2;
    }
    

  }

  if(avg > 9000){
    again = again - 1;
    if(again < 1){
      again = 1;
    }
  }else if(avg < 9000){
    again = again + 0.1;
    if(again > gain){
      again = gain;
    }
  }

  if(ming == maxg){
    ming = min;
    maxg = max;
  }else {
    ming = (ming + min)/2;
    maxg = (maxg + max)/2;
  }
  shift_coeff = -((maxg - ming)/2 + ming);

}
void cleanLPF(){
  if(filter != NULL){
    clear_filter(filter);
  }
  if(filtertx != NULL){
    clear_filter(filtertx);
  }

}
