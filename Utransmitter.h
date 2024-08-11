#ifndef UTX
#define UTX

void amplitude_modulate(short* input, short* output, int length, float frequency, double gain);
void DSB_modulate(short* input, short* output, int length, float frequency, double gain);

void amplitude_demodulate(short* input, short* output, int length, float frequency,  double gain);

//----
//
//
void clean_f_manager(); //clear cached frequency data
void cleanLPF();

void init_f_manager(int buffer_size, int sample_rate_hz);
//call this to start caching frequency data

#endif // !UTX
