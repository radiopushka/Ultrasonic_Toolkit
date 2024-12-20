#include <alsa/asoundlib.h>
#include<stdio.h>
#include "alsaconf.c"
#include "alsa.h"



snd_pcm_t* record;
int record_channels=2;
unsigned int record_sample_rate=48000;
snd_pcm_t* playback;
snd_pcm_status_t* record_stat;
snd_pcm_status_t* plbk_stat;
int playback_channels=2;
unsigned int playback_sample_rate=48000;

int alsa_buffer_size=0;
short* nullp;

short* mock_read;
int mock_read_size;

short* mock_write;
int mock_write_size;
short* zeros;


int setup_alsa(char* recordf, char* playbackf, int buffer_size, int sampler){//we won't be using microphone until later version
	int frames=1;
  nullp = malloc(sizeof(short)* buffer_size);
  record_sample_rate = sampler;
  playback_sample_rate = sampler;
  alsa_buffer_size=buffer_size;
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
	if(record_sample_rate!=playback_sample_rate){
		printf("card parameter mismatch\n");
		return -1;
	}
  printf("alsa setup \n");
  snd_pcm_link( record,playback);//synchornize

  
  mock_read_size = buffer_size * record_channels; 
  mock_read = malloc(sizeof(short)*mock_read_size);

  mock_write_size = buffer_size * playback_channels;
  mock_write = malloc(sizeof(short)*mock_write_size);
  zeros=malloc(sizeof(short)*mock_write_size);
  memset(zeros,0,sizeof(short)*mock_write_size);

	snd_pcm_prepare(playback);
	snd_pcm_prepare(record);
	
	return 1;
}

void sync_interfaces(){
 int sload=10;
 short dummy[sload];
int fail=0;
  int size1 = sload/record_channels;
  int size2 = sload/playback_channels;
  while(fail==0){
  int status;
  if (snd_pcm_readi(record, dummy, size1)<0) {
			//printf("read %d\n",status);
			
  }
  fail=1;
	if((status=snd_pcm_writei(playback,dummy,size2 ))== -EPIPE){
    snd_pcm_drain(playback);
		snd_pcm_recover(playback,status,1);
	  snd_pcm_prepare(playback);
    snd_pcm_drain(record);
    fail=0;
  }
  }

}
int aread(short* output){
	int status;
	if ((status=snd_pcm_readi(record,mock_read,alsa_buffer_size))<0) {
		  snd_pcm_recover(record,status,1);
      sync_interfaces();
			return -1;
  	 }
  if(record_channels==1){
    memcpy(output,mock_read,sizeof(short)*mock_read_size);
  }else{
    int cc=0;
    for(int i=0; i < mock_read_size;i++){
      if(i%record_channels==0){
        output[cc]=mock_read[i];
        cc++;
      }
    }
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
  if(playback_channels==1){
    memcpy(mock_write,input,sizeof(short)*mock_write_size);
  }else{
    int cc=0;
    int tval=input[0];
    for(int i =0; i<mock_write_size;i++){
      if(i%playback_channels==0){
        tval = input[cc];
        cc++;
      }
      mock_write[i]=tval;
    }
  }
	 if((status=snd_pcm_writei(playback,mock_write,alsa_buffer_size ))== -EPIPE){
		snd_pcm_recover(playback,status,1);
	  snd_pcm_prepare(playback);
    sync_interfaces();
    snd_pcm_writei(playback,zeros,alsa_buffer_size );
		// snd_pcm_wait(playback,-1);
		
	 }
	 

}
int get_sample_rate(){
	return playback_sample_rate;
}
void free_alsa(){
  free(mock_write);
  free(mock_read);
	snd_pcm_close(record);
	snd_pcm_close(playback);
	snd_pcm_hw_free(record);
	snd_pcm_hw_free(playback);
  snd_pcm_status_free(plbk_stat);
  snd_pcm_status_free(record_stat);
  free(nullp);
}

