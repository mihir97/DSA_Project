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
#include <stdlib.h>
#include "settings.h"
#define MAX(i,j) (i)>(j)?(i):(j)
int readfromfile(char *fname, float *a){
	FILE *fp;
	fp = fopen(fname, "r");
	int i = 0;
	while(fscanf(fp,"%f" ,a + i) != EOF){
		i++;
	}
	fclose(fp);
	return i;

}
float compare(char *f1, char *f2){
	int k = 0, z = 0, i , j;
	float error = 4 * SAMPLE_RATE / FFT_SIZE;
	float* a = (float*)calloc (10, sizeof(float));
	float* b = (float*)calloc (10, sizeof(float));

	i = readfromfile(f2, a);
	j = readfromfile(f1, b);

	k = 0;
	
	while(k < j  && k < i ){ 
		if(b[k] > (a[k] - error) && b[k] < (a[k] + error)) {
			z++;
		}
		k++;
	}

	float sim = (float)z/((float)MAX(i,j));
	return sim;
}
