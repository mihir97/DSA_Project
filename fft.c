#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<fftw3.h>
#include "settings.h"
#define PI 3.147

void apply_fft(float *data,int numSamples, char* argv){
	FILE *fw, *fp1, *fp2, *fp3;
	fw = fopen(argv, "wb");
	if(!fw) printf("Error opening file.\n");
	fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*FFT_SIZE);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*FFT_SIZE);
	fftw_plan p = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	
	int i, j, i_max = 0, count = 0;
	double max = 0, ans;
	float *samples = (float *) malloc(sizeof(float) * numSamples);

	fp1 = fopen("test.dat1", "wb");
	fp2 = fopen("test.dat2", "wb");
	fp3 = fopen("test.dat3", "wb");

	for(i = 0; i< numSamples ; i = i + FRAMES_PER_BUFFER){
		if(data[i+1] != SAMPLE_SILENCE){
			for(j = i; j < i + FRAMES_PER_BUFFER; j++){
				samples[count] = data[j];
				count++;
				//fprintf(fp1,"%f\n", samples[j]);
			}
		}
	}
		
	int sample_count = 0;

	while(sample_count < count){
		ans = max = 0;
		i_max = 0;
		for(j = 0; j < FFT_SIZE && sample_count < count; j++){
			fprintf(fp1,"%f\n", samples[sample_count]);
			double multiplier = 0.5 * ( 1 - cos(2 * PI * j / (FFT_SIZE - 1))); /*Applying Hann Window Function*/
			in[j][0] = multiplier * samples[sample_count];
			sample_count++;
			fprintf(fp2, "%f\n", in[j][0]);
		}	

		fftw_execute(p);
		
		for(i = 0; i < j/2 + 1; i++){
			fprintf(fp3, "%f\t%f\n", out[i][0], out[i][1]);
			ans = pow(out[i][0],2) + pow(out[i][1], 2);
			ans = sqrt(ans);
			if(ans > max){
				max = ans;
				i_max = i;
			}
			
		}
		/*Printing the Dominating Frequency in file*/
		fprintf(fw, "%lf\n",(i_max * ((double)SAMPLE_RATE) / FFT_SIZE));
	}

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
}
