/*
 *      Speech Recognition in C:
 *      Copyright (C) 2016  Mihir Mistry
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<fftw3.h>
#include "settings.h"
#define PI 3.147

void apply_fft(float *data,int numSamples, char* argv){
	FILE *fw;
	fw = fopen(argv, "wb");
	if(!fw) printf("Error opening file.\n");
	fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*FFT_SIZE);
	fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*FFT_SIZE);
	fftw_plan p = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
	
	int i, j, i_max = 0, count = 0;
	double max = 0, ans;
	float *samples = (float *) malloc(sizeof(float) * numSamples);


	for(i = 0; i< numSamples ; i = i + FRAMES_PER_BUFFER){
		if(data[i+1] != SAMPLE_SILENCE){
			for(j = i; j < i + FRAMES_PER_BUFFER; j++){
				samples[count] = data[j];
				count++;
			}
		}
	}
		
	int sample_count = 0;

	while(sample_count < count){
		ans = max = 0;
		i_max = 0;
		for(j = 0; j < FFT_SIZE && sample_count < count; j++){
			double multiplier = 0.5 * ( 1 - cos(2 * PI * j / (FFT_SIZE - 1))); /*Applying Hann Window Function*/
			in[j][0] = multiplier * samples[sample_count];
			sample_count++;
		}	

		fftw_execute(p);
		
		for(i = 0; i < j/2 + 1; i++){
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
	fclose(fw);
	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);
}
