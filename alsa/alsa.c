#include <alsa/asoundlib.h>
#include<stdio.h>
#include "alsaconf.c"
#include "alsa.h"



snd_pcm_t* record;
int record_channels=1;
unsigned int record_sample_rate=48000;
snd_pcm_t* playback;
int playback_channels=1;
unsigned int playback_sample_rate=48000;

int alsa_buffer_size=0;

int setup_alsa(char* recordf, char* playbackf, int buffer_size, int sampler){//we won't be using microphone until later version
	int frames=1;
  record_sample_rate = sampler;
  playback_sample_rate = sampler;
  alsa_buffer_size = buffer_size;
	if ( snd_pcm_open(&playback, playbackf,SND_PCM_STREAM_PLAYBACK, 0) < 0){		
		printf("unable to open playback interface\n");
		return -1;
	} 
  if ( snd_pcm_open(&record, recordf,SND_PCM_STREAM_CAPTURE, 0) < 0){		
		printf("unable to open recording interface\n");
		return -1;
	} 

	if(configure_sound_card(playback,&playback_sample_rate,&playback_channels,1,buffer_size,&frames)<0){
    printf("play config failed \n");
		return -1;
	}
	if(configure_sound_card(record,&record_sample_rate,&record_channels,0,buffer_size,&frames)<0){
    printf("record config failed \n");
		return -1;
	}
	if(record_sample_rate!=playback_sample_rate||record_channels!=playback_channels){
		printf("card parameter mismatch\n");
		return -1;
	}
  printf("alsa setup \n");
  snd_pcm_link( record,playback);//synchornize

	snd_pcm_prepare(playback);
	snd_pcm_prepare(record);
	
	return 1;
}
int aread(short* output){
	int status;
	if ((status=snd_pcm_readi(record, (char*)output, alsa_buffer_size))<0) {
			//printf("read %d\n",status);
			
			return -1;
  	 }
  return 1;
}
void awrite(short* input){
  int status;
		snd_pcm_wait(playback,-1);
	 if((status=snd_pcm_writei(playback,(char*)input,alsa_buffer_size))== -EPIPE){
    snd_pcm_drain(playback);
		snd_pcm_recover(playback,status,1);

    snd_pcm_link( record,playback);//synchornize
		// snd_pcm_wait(playback,-1);
		
	 }else if(status<0){
	 	 //printf("write %d\n",status);
	 	//return -1;
        snd_pcm_drain(playback);
		 	 snd_pcm_recover(playback,status,1);
	     snd_pcm_prepare(playback);
        snd_pcm_link( record,playback);//synchornize
	 }
	 

}
int get_sample_rate(){
	return playback_sample_rate;
}
void free_alsa(){
	snd_pcm_close(record);
	snd_pcm_close(playback);
	snd_pcm_hw_free(record);
	snd_pcm_hw_free(playback);
}

