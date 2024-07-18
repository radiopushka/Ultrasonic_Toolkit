#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int file_size = 0;
unsigned int header_size = 0;
char channels = 1;
int srate = 96000;
unsigned int rate_file = 0;

void read_wav_header_skip(FILE* f){
  
  int i;
  for(i = 0; i < 4; i++){
    fgetc(f);
  }
  unsigned int size = 0;
  size |= fgetc(f);
  size |= (fgetc(f) << 8);
  size |= (fgetc(f) << 16);
  size |= (fgetc(f) << 24);

  file_size = size;
  for(i = i + 4; i < 22; i++){
    fgetc(f);
  }
  channels = fgetc(f);
  fgetc(f);

  unsigned int rate_f = 0;
  rate_f |= fgetc(f);
  rate_f |= (fgetc(f) << 8);
  rate_f |= (fgetc(f) << 16);
  rate_f |= (fgetc(f) << 24);
  rate_file = rate_f;




  for(i = i + 6; i < 16; i++){
    fgetc(f);
  }


  char exit = 0;
  char fd;
  while( exit != -1){
    fd = fgetc(f);
    if(fd == 0){
      exit = 1;
    }else if(exit == 1 && fd == 'd'){
      exit = 2;
    }else if(exit == 2 && fd == 'a'){
      exit = 3;
    }else if(exit == 3 && fd == 't'){
      exit = 4;
    }else if(exit == 4 && fd == 'a'){
      exit = -1;
      break;
    }else{
      exit = 0;
    }
  }
  size = 0;
  size |= fgetc(f);
  size |= fgetc(f) << 8;
  size |= fgetc(f) << 16;
  size |= fgetc(f) << 24;

  header_size = size - file_size;
  file_size = size;


}
void print_st(unsigned char** data, char* st){
  while (*st != 0){
    **data = *st;
    *data = *data + 1;
    st++;
  }
}
void write_wav_header(FILE* f){
  unsigned char* header = malloc(sizeof(char) * 45);
  unsigned char* dpointer = header;
  print_st(&dpointer, "RIFF");
  unsigned int new_file_size = file_size;

  


  int i;
  for(i = 0; i < 4; i++){
    *dpointer = new_file_size;
    new_file_size = new_file_size >> 8;
    dpointer++;
  }

  print_st(&dpointer, "WAVEfmt");
  *dpointer = 32;
  dpointer++;
  *dpointer = 16;
  dpointer++;
  *dpointer = 0;
  dpointer++;
  *dpointer = 0;
  dpointer++;
  *dpointer = 0;
  dpointer++;
  *dpointer = 1;
  dpointer++;
  *dpointer = 0;
  dpointer++;
  *dpointer = channels;
  dpointer++;
  *dpointer = 0;
  dpointer++;


  new_file_size = srate;
 


  for(i = 0; i < 4; i++){
    *dpointer = new_file_size;
    new_file_size = new_file_size >> 8;
    dpointer++;
  }


  
  new_file_size = (srate * 16 * channels )/8;


  for(i = 0; i < 4; i++){
    *dpointer = new_file_size;
    new_file_size = new_file_size >> 8;
    dpointer++;
  }

 // *dpointer = 0;
 // dpointer++;

  *dpointer = 2 * channels;
  dpointer++;
  *dpointer = 0;
  dpointer++;
  *dpointer = 16;
  dpointer++;
  *dpointer = 0;
  dpointer++;


  print_st(&dpointer, "data");

 

 new_file_size = file_size;

  
  for(i = 0; i < 4; i++){
    *dpointer = new_file_size;
    new_file_size = new_file_size >> 8;
    dpointer++;
  }


  fwrite(header, sizeof(char), 44,f);
  free(header);
  

}


