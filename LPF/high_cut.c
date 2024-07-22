#include "high_cut.h"
#include <stdlib.h>
#include<stdio.h>
#include <math.h>

#define PI 3.141592653589793




struct high_cut* init_highcut(int samplerate,int frequencyi,int polesi,double gain,double Q){//quality factor, max 1
	int frequency=frequencyi;
	int size=(samplerate/frequency);
	if(polesi<1){
		return NULL;
	}
	int poles=polesi;
	if(size<2){
		return NULL;
	}
	struct high_cut* object=malloc(sizeof(struct high_cut));
	object->size=size;
	object->poles=poles;
	object->convolution_buffer=malloc(sizeof(double*)*poles);
	object->convolution_window=malloc(sizeof(double)*size);
	object->sum=malloc(sizeof(double)*poles);
	object->gain=gain;
	int i;
	double step=(PI/(size-1));
	double start=0;
	for(i=0;i<size;i++){
		object->convolution_window[i]=(1-Q)+(cos(start)*Q);
		start=start+step;
	}
	int i2;
	for(i=0;i<poles;i++){
		object->convolution_buffer[i]=malloc(sizeof(double*)*size);
		for(i2=0;i2<size;i2++){
			object->convolution_buffer[i][i2]=0;
		}
		object->sum[i]=0;
	}
	return object;
}

short perform_filter(double input,struct high_cut* object,double* output){
	if(object==NULL){
		return -1;
	}
	int i;
	int size=object->size;
	int vsss=size-1;
	int i2;
	int poles=object->poles;
	for(i=size-1;i>=0;i--){
		for(i2=0;i2<poles;i2++){
			object->sum[i2]=object->sum[i2]+(object->convolution_buffer[i2][i])*((object->convolution_window[vsss-i])*(object->gain));
			if(i>0){
				object->convolution_buffer[i2][i]=object->convolution_buffer[i2][i-1];
			}else if(i2>0){
				object->convolution_buffer[i2][0]=((object->sum[i2-1])/(size));	
				object->sum[i2-1]=0;
			}
			
			
		}
	}
	object->convolution_buffer[0][0]=input;
	double pout=(object->sum[poles-1])/(size);
	object->sum[poles-1]=0;
	*output=pout;
	return 1;
	
}

void set_gain(struct high_cut* object,double gain){
	if(object==NULL){
		return;
	}
	object->gain=gain;
}
void clear_filter(struct high_cut* object){
	if(object==NULL){
		return;
	}
	int depth=object->poles;
	int i;
	for(i=0;i<depth;i++){
		free(object->convolution_buffer[i]);
	}
	free(object->convolution_buffer);
	free(object->convolution_window);
	free(object->sum);
	free(object);
}
