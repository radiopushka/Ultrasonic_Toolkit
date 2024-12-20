#include "Utransmitter.h"
#include "frequencymanager.c"
#include "LPF/high_cut.h"
struct high_cut* filter = NULL;
struct high_cut* filtertx = NULL;

//frequency in khz, 1 is 1 khz
void amplitude_modulate(short* input, int* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = init_highcut(sample_rate,6000,3,2,0.5);
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

void int_to_short_buff(int* input,int length,short* output){

 for(int i =0 ;i < length;i++){

      output[i]=input[i];
  }

}

void declip(int* input,int length){

  int max = 0;

  for(int i =0 ;i < length;i++){

    if(input[i]>abs(max)){
      max=abs(input[i]);
    }
  }

  float factor=1;
  if(max > 30000){
    factor = max/30000.0;
  }
 for(int i =0 ;i < length;i++){
    input[i]=input[i]/factor;
  }

}

void DSB_modulate(short* input, int* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = init_highcut(sample_rate,6000,3,2,0.5);
  }

  int i;
  double coeff = gain;

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
    output[i] = (periods[freqi][i] * (preview * coeff));
  }

}


double shift_coeff = 0;
double maxg = 0;
double ming = 0;

int playback_rate=48000;

float again = 1;

double avg = 100000000;

void DSB_demodulate(short* input, int* output, int length, float freq, double gain){


  if(filter == NULL){
    filter = init_highcut(playback_rate,6000,3,3,0.6);
  }

  int i;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }

  double sin;
  double cos;

  double demod_sum=0;

  double output_prev;
  int old=0;

  int playback_sample_rate=sample_rate/playback_rate;

  //fourier transform
  for( i = 0; i < length; i++){


    sin = input[i] * periods[freqi][i];
    cos = input[i] * cosine[freqi][i];
    
    demod_sum = sin + cos +demod_sum;

    //output_prev = ( (demod) * again );
    //perform_filter(output_prev,filter,&output_prev);
    if(i%playback_sample_rate==0){

      output_prev = ( ((demod_sum)/(playback_sample_rate)) * again );
      demod_sum=0;
      perform_filter(output_prev,filter,&output_prev);
      output[i] = output_prev;
      old= output_prev;

      if(avg == 100000000){
        avg = fabs(output_prev);
      }else{
       avg = (avg + fabs(output_prev))/2;
      }
    

    }else{

      output[i] = old;
    }


    

  }

  if(avg > 8000){
    again = again - 0.1;
    if(again < 1){
      again = 1;
    }
  }else if(avg < 3000){
    again = again + 0.1;
    if(again > gain){
      again = gain;
    }
  }



}


void amplitude_demodulate(short* input, int* output, int length, float freq, double gain){


  if(filter == NULL){
    filter = init_highcut(sample_rate,5000,3,3,0.6);
  }

  int i;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }
  double sin;
  double cos;

  double demod_sum=0;

  double output_prev;
  int old=0;

  int playback_sample_rate=sample_rate/playback_rate;

  //fourier transform
  for( i = 0; i < length; i++){


    sin = input[i] * periods[freqi][i];
    cos = input[i] * cosine[freqi][i];
    
    demod_sum = sin + cos +demod_sum;

    //output_prev = ( (demod) * again );
    //perform_filter(output_prev,filter,&output_prev);
    if(i%playback_sample_rate==0){
      output_prev = ( (demod_sum/playback_sample_rate - shift_coeff) * again );
      perform_filter(output_prev,filter,&output_prev);
      output[i] = output_prev;
      old=output_prev;

      if(avg == 100000000){
       avg = fabs(output_prev);
      }else{
        avg = (avg + fabs(output_prev))/2;
      }

    }else{
      output[i] = old;
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

    shift_coeff = avg/2 + avg/4;

}
void cleanLPF(){
  if(filter != NULL){
    clear_filter(filter);
  }
  if(filtertx != NULL){
    clear_filter(filtertx);
  }

}
