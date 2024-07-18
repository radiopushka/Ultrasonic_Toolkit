#ifndef ALSAI
#define ALSAI
int setup_alsa(char* record,char* playback, int buffer_size, int samplerate);
void free_alsa();
int get_sample_rate();

int aread(short* data);
void awrite(short* data);


#endif // !ALSAI
