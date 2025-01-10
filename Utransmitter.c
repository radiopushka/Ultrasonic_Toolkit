#include "Utransmitter.h"
#include "frequencymanager.c"
#include "RC/rc.h"
rc_filter_info* filter = NULL;
rc_filter_info* filtertx = NULL;

//frequency in khz, 1 is 1 khz
void amplitude_modulate(short* input, int* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = create_rc_filter(1000,sample_rate,0);
  }

  int i;
  double coeff = gain * 0.5;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }
// amplitude modulation
  int preview;
  for(i = 0; i < length; i++){
    preview = input[i];
    preview=do_rc_filter(filtertx,preview);
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
  if(max > 25000){
    factor = max/25000.0;
  }
 for(int i =0 ;i < length;i++){
    input[i]=input[i]/factor;
  }

}

void DSB_modulate(short* input, int* output,int length, float freq, double gain){
  if(filtertx == NULL){
    filtertx = create_rc_filter(1000,sample_rate,0);
  }

  int i;
  double coeff = gain;

  int freqi = get_index_for_frequency(freq);
  if(freqi == -1){
    add_frequency(freq);
    freqi = get_index_for_frequency(freq);
  }
// amplitude modulation
  int preview;
  for(i = 0; i < length; i++){
    preview = input[i];
    preview=do_rc_filter(filtertx,preview);
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
    filter = create_rc_filter(3000,playback_rate,0);

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
      output_prev=do_rc_filter(filter,output_prev);
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
    filter = create_rc_filter(3000,playback_rate,0);
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
      output_prev=do_rc_filter(filter,output_prev);
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
    free_rc_filter(filter);
  }
  if(filtertx != NULL){
    free_rc_filter(filtertx);
  }

}
