#include<stdio.h>
#include<math.h>
#include<fftw3.h>
#include "settings.h"
#define PI 3.147

int main(int argc, char* argv[]){
	FILE* fp, *fw;
	fp = fopen(argv[1], "r");
	fw = fopen(argv[2], "wb");
	fftw_complex in[FFT_SIZE], out[FFT_SIZE];
	int i, i_max = 0;
	double max = 0, ans;
	
	
	while(!feof(fp)){
		ans = max = 0;
		i_max = 0;
		for(i = 0; i < FFT_SIZE; i++){
			double multiplier = 0.5 * ( 1 - cos(2 * PI * i / (FFT_SIZE - 1))); /*Applying Hann Window Function*/
			fscanf(fp, "%lf", &in[i][0]);
			in[i][0] = multiplier * in[i][0];
		}
		
		fftw_plan p = fftw_plan_dft_1d(FFT_SIZE, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
		fftw_execute(p);
		for(i = 0; i < FFT_SIZE/2 + 1; i++){
			ans = pow(out[i][0],2) + pow(out[i][1], 2);
			ans = sqrt(ans);
			if(ans > max){
				max = ans;
				i_max = i;
			}
		}
		/*Printing the Dominating Frequency in file*/
		fprintf(fw, "%lf\n",(i_max * (double)SAMPLE_RATE) / FFT_SIZE);
	}
	return 0;
}
