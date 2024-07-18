#include <math.h>
#include <stdlib.h>

float* frequencies = NULL;
double** periods = NULL;
double** cosine = NULL;
int buffer_size = 0;
int sample_rate = 0;
int freqs = 0;

int get_index_for_frequency(float freq){

  int i;
  for(i = 0; i < freqs; i++){
    if( frequencies[i] == freq )
      return i;
  }
  return -1;
}


void clean_f_manager(){
  free(frequencies);
  frequencies = NULL;

  buffer_size = 0;
  sample_rate = 0;

  if(periods == NULL && cosine == NULL){
    freqs = 0;
    return;
  }

  int i;

  for( i = 0; i < freqs; i++){
    free(periods[i]);
    free(cosine[i]);
  }

  free(periods);
  free(cosine);
  cosine = NULL;
  periods = NULL;
  freqs = 0;
  
}

void init_f_manager(int bsize, int rate){
  if(buffer_size != bsize || rate != sample_rate){
    clean_f_manager();
  }
  sample_rate = rate;
  buffer_size = bsize;


}

void populate_index_f(int index){

  float frequencykhz = frequencies[index];

  double pi = 3.14159265358979323846;
  double reset_targ = 2 * pi;


  double step = ( (frequencykhz*2000) / sample_rate) * pi;
  double sv = 0;

  int i;
  for(i = 0; i < buffer_size; i++){
    periods[index][i] = cos(sv);
    cosine[index][i] = sin(sv);

    sv = sv + step;
    if(sv > reset_targ)
      sv = sv - reset_targ;
  }


}


void add_frequency(float frequencykhz){

  int nsize = freqs + 1;

  float* newarray = malloc(sizeof(float) * (nsize));
  double** newconv = malloc(sizeof(double*) * nsize);
  double** newconv2 = malloc(sizeof(double*) * nsize);

  int i;

  for(i = 0; i < freqs; i++){
    newarray[i] = frequencies[i];
    newconv[i] = periods[i];
    newconv2[i] = cosine[i];
  }
  newarray[i] = frequencykhz;
  free(frequencies);
  frequencies = newarray;

  newconv[i] = malloc(sizeof(double) * buffer_size);
  newconv2[i] = malloc(sizeof(double) * buffer_size);
  free(periods);
  periods = newconv;
  free(cosine);
  cosine = newconv2;

  freqs = nsize;

  populate_index_f(i);
}
