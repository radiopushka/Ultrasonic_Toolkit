#ifndef HIGHCUT
#define HIGHCUT

struct high_cut{
	double** convolution_buffer;
	double* convolution_window;
	double* sum;
	int size;
	double gain;
	int poles;
};

struct high_cut* init_highcut(int samplerate,int frequency,int poles,double gain,double quality_factor);// frequency is in hz, sample rate is in hz
short perform_filter(double input,struct high_cut* object,double* output);
void clear_filter(struct high_cut* object);
void set_gain(struct high_cut* object,double gain);

#endif // !HIGHCUT
