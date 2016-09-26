#include<stdio.h>
#include <stdlib.h>
#include "settings.h"
int i, j;

float compare(float *a, float *b){
	int k = 0, z = 0;
	float error = 2 * SAMPLE_RATE / FFT_SIZE;
	while(k < j  && k < i ){ 
		if(b[k] > (a[k] - error) && b[k] < (a[k] + error)) {
			z++;
		}
		k++;
	}
	return ((double)z)/i;
}

int main(int argc, char* argv[]) {
	int temp;
	FILE *fp;
	
	float a[30], b[30];
	
	fp = fopen(argv[1],"r");
	if(fp == NULL)
		exit(1);

	i = 0;
	while(fscanf(fp, "%f", a + i) != EOF)
		 i++;
	
	fp = fopen(argv[2],"r");
	if(fp == NULL)
		exit(1);
	
	j = 0;
	while (fscanf(fp,"%f", &b[j]) != EOF)
		j++;
	//int m = 0 , n = 0;
	/*for(; m < i ; m++){
		printf("%f", a[m]);
	}
	for(; n < j ; n++){
		printf("%f", b[n]);
	}*/
	float matches;
	matches = compare(a, b);
	printf("%f\n", matches);
	if(matches >= 0.4){
		printf("hello\n");
	}
	return 0;
}
