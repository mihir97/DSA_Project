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



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"

char* findcurr(char *prev, char *tmp){
	char buff1[20], buff2[20], fname[20], *curr;
	float curr_per = -1, tmp_per;
	curr = (char *)malloc(20 * sizeof(char));
	int start_flag = 0;

	FILE *fprev;

	sprintf(buff1,"data/next/%s.nxt",prev);
	fprev = fopen(buff1,"r");

	while(fscanf(fprev, "%s", fname) != EOF){
		if(strlen(fname) == 0) continue;
		if(strcmp(fname, "START") == 0){
			start_flag = 1;
			continue;
		}
		sprintf(buff2, "data/word/%s.dat", fname);
		tmp_per = compare(tmp, buff2);
		if(tmp_per > curr_per){
			curr_per = tmp_per;
			strcpy(curr, fname);
		}
	}

	if(start_flag){
		char *s_curr;
		s_curr = findcurr("START", tmp);
		sprintf(buff2, "data/word/%s.dat", s_curr);
		if( compare(tmp, buff2) > curr_per){
			free(curr);
			curr = s_curr;
			curr[strlen(curr) + 1] = 'S';
		}
	}
	fclose(fprev);
	return curr;
}
