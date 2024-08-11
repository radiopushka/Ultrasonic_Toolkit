#include <alsa/asoundlib.h>
#include<stdio.h>
#include "alsaconf.c"
#include "alsa.h"



snd_pcm_t* record;
int record_channels=1;
unsigned int record_sample_rate=48000;
snd_pcm_t* playback;
snd_pcm_status_t* record_stat;
snd_pcm_status_t* plbk_stat;
int playback_channels=1;
unsigned int playback_sample_rate=48000;

int alsa_buffer_size=0;
short* nullp;

int setup_alsa(char* recordf, char* playbackf, int buffer_size, int sampler){//we won't be using microphone until later version
	int frames=1;
  nullp = malloc(sizeof(short)* buffer_size);
  record_sample_rate = sampler;
  playback_sample_rate = sampler;
  alsa_buffer_size = buffer_size;
  snd_pcm_status_malloc(&record_stat);
  snd_pcm_status_malloc(&plbk_stat);
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
void drain_incomming(){
  snd_pcm_drain(record);
}
void drain_exiting(){
  snd_pcm_drain(playback);
}

void sync_record(int retime){
  snd_pcm_status(playback,plbk_stat);
  int delay = snd_pcm_status_get_delay(plbk_stat);

  if(delay > retime){
    aread(nullp);
  }
}
void sync_play(int retime){
  sync_record(retime);
  }

void awrite(short* input){
  int status;
		snd_pcm_wait(playback,-1);
	 if((status=snd_pcm_writei(playback,(char*)input,alsa_buffer_size ))== -EPIPE){
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
  snd_pcm_status_free(plbk_stat);
  snd_pcm_status_free(record_stat);
  free(nullp);
}

