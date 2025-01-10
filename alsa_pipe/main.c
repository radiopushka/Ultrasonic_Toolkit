#include "alsaconf.c"
#include <pthread.h>
#include <string.h>

#include "alsa_pipe.h"

short* input_buffer=NULL;
short* input_end_ptr=NULL;
unsigned int data_in_buffer=0;
unsigned int buffer_space=0;
unsigned int start_point=0;
pthread_mutex_t write_access;
pthread_mutex_t pipe_access;

snd_pcm_t* input=NULL;
snd_pcm_t* output=NULL;

unsigned int forward_buffer_size=1500;
unsigned int step_back=0;
unsigned int sink_back=0;
short* tmp_buff;

unsigned int empty_size=20;
short* empty_buff;

int input_channels=1;
int output_channels=1;


pthread_t playback_thread;

unsigned int sink_state=0;
unsigned int logged=8; //latency

int pcm_write(short* data,unsigned int size){
 
 snd_pcm_t* ptr_cpy=NULL;

 pthread_mutex_lock(&pipe_access);
 ptr_cpy=output;
 pthread_mutex_unlock(&pipe_access);
 if(ptr_cpy==NULL){
    return -1;
  }

  
 unsigned int bsize=size;
  if(output_channels>1){
    bsize=bsize>>1;
  }


  int status;
	if((status=snd_pcm_writei(ptr_cpy,data,bsize ))== -EPIPE){
    //snd_pcm_drain(output);
		snd_pcm_recover(ptr_cpy,status,1);
    snd_pcm_prepare(ptr_cpy);
  }
  return 1;
}

int forward_audio(){
  if(output==NULL)
    return -1;

  unsigned int data_in_buff=0;
  pthread_mutex_lock(&write_access);
  data_in_buff=data_in_buffer;
  if((data_in_buff<step_back && sink_state==0)||(data_in_buff<sink_back && sink_state==1)){
  
    
    pthread_mutex_unlock(&write_access);
    if(pcm_write(empty_buff,forward_buffer_size)==-1){
      return -1;
    }
    sink_state=1;


    return 1;
  }
  if(data_in_buff>=sink_back){
    //snd_pcm_drain(output);
    sink_state=0;
  }

  short* start_ptr=input_end_ptr - forward_buffer_size;
  unsigned int nosize=start_ptr-input_buffer;
  memcpy(tmp_buff,start_ptr,forward_buffer_size<<1);

   
  if(data_in_buffer!=0){
    memcpy(input_buffer+forward_buffer_size,input_buffer,nosize<<1);
    memset(input_buffer,0,forward_buffer_size<<1);

    data_in_buffer=data_in_buffer-forward_buffer_size;
    start_point=start_point+forward_buffer_size;
  }

  pthread_mutex_unlock(&write_access);
  if(pcm_write(tmp_buff,forward_buffer_size)==-1){
    return -1;
  }

  return 1;
}

void* audio_thread_cont(void* arg){
  pthread_detach(pthread_self()); 
  int status=0;
  while(status!=-1&&output!=NULL){
    status=forward_audio();
  }

  return NULL;
}

int queue_audio(short* data){
  pthread_mutex_lock(&write_access);
  //printf("%d %d\n",start_point,data_in_buffer);
  if(buffer_space<data_in_buffer){
    pthread_mutex_unlock(&write_access);
    return -1;
  }
  /*if(synced==1){
    synced=0;
    printf("synced\n");
  }
  if(unsynced==1){
    unsynced=0;
    printf("unsynced\n");

  }*/

  //printf("%d %d\n",start_point,data_in_buffer);
  memcpy(input_buffer+start_point,data,forward_buffer_size<<1);
  if(start_point!=0){
    start_point=start_point-forward_buffer_size;
    data_in_buffer=data_in_buffer+forward_buffer_size;
  }

  

  pthread_mutex_unlock(&write_access);
  return 1;
}

int get_audio(short* data){
  if(input==NULL)
    return -1;

  unsigned int bsize=forward_buffer_size;
  if(input_channels>1){
    bsize=bsize>>1;
  }

  int status;
	if ((status=snd_pcm_readi(input,data,bsize))<0) {
		  snd_pcm_recover(input,status,1);
			return -1;
  	 }
  return 1;

}

void set_latency(int latency_buffers){
 logged=(unsigned int) latency_buffers; 
}

int setup_alsa_pipe(char* recording_iface, char* playback_iface, int* channels_in,int* channels_out,int* input_rate,int* output_rate, int buffer_Size){
 
  //needed to configure alsa parameters
  forward_buffer_size=(unsigned int)buffer_Size;
  step_back=forward_buffer_size*2;
  sink_back=forward_buffer_size*(logged/2);

  //alsa stuff

	if ( snd_pcm_open(&output, playback_iface,SND_PCM_STREAM_PLAYBACK, 0) < 0){		
		printf("unable to open playback interface\n");
		return -1;
	} 
  if ( snd_pcm_open(&input, recording_iface,SND_PCM_STREAM_CAPTURE, 0) < 0){		
		printf("unable to open recording interface\n");
		return -1;
	} 

	if(configure_sound_card(output,step_back,(unsigned int*)output_rate,channels_out)<0){
    printf("play config failed \n");
		return -1;
	}
	if(configure_sound_card(input,step_back,(unsigned int*)input_rate,channels_in)<0){
    printf("record config failed \n");
		return -1;
	}

  snd_pcm_link(input,output);//synchornize
	snd_pcm_prepare(output);
	snd_pcm_prepare(input);

  output_channels=*channels_out;
  input_channels=*channels_in;
  
  buffer_space=forward_buffer_size*logged;
  start_point=forward_buffer_size*(logged-1);
  data_in_buffer=0;
  input_buffer=malloc(sizeof(short)*(buffer_space+1));//short is always 2 bytes on all operating systems
  input_end_ptr=input_buffer+buffer_space;

  tmp_buff=malloc(sizeof(short)*buffer_Size);
  empty_buff=malloc(sizeof(short)*buffer_Size);

  memset(empty_buff,0,sizeof(short)*buffer_Size);


	pthread_mutex_init(&write_access, NULL);
	pthread_mutex_init(&pipe_access, NULL);
	pthread_mutex_unlock(&write_access);
	pthread_mutex_unlock(&pipe_access);

  pthread_create(&playback_thread,NULL,&audio_thread_cont,NULL);

  return 1;
}

void alsa_pipe_exit(){

	  pthread_mutex_lock(&pipe_access);
    snd_pcm_t* otmp=output;
    output=NULL;
	  pthread_mutex_unlock(&pipe_access);
    
    pthread_join(playback_thread,NULL);
    
    //pthread_destroy(playback_thread);
  

	  snd_pcm_hw_free(input);
	  snd_pcm_hw_free(otmp);
    snd_pcm_close(input);
	  snd_pcm_close(otmp);

    snd_config_update_free_global();

    pthread_mutex_destroy(&pipe_access);
    pthread_mutex_destroy(&write_access);

    free(input_buffer);
    free(tmp_buff);
    free(empty_buff);

    printf("exit\n");


}
