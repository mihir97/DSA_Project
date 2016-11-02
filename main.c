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
#include<unistd.h>
#include "settings.h"
#include <string.h>

int main(int argc, char * argv[])
{
	PaError err = paNoError;

	if(argc == 2){
		if( strcmp(argv[1],"-h") == 0){
			usage();
		}
		else if(strcmp(argv[1], "-r") == 0){
			char str[10],buff[20];
			printf("Enter word to replace:\n");
			scanf("%s", str);
			sprintf(buff,"data/word/%s.dat",str);
			if(access(buff,F_OK) == -1){
				printf("Word doesn't exist\nTry adding first");
			}
			else
				err = replace(str);
		}	
		else if(strcmp(argv[1], "-a") == 0){
			char str[10];
			printf("Enter word to add:\n");
			scanf("%s", str);
			err = add(str);
		}
		else if(strcmp(argv[1], "-l") == 0){
			list();
		}
		else{
			printf("Invalid use");
			usage();
		}
	}
	else if(argc != 1){
		printf("Invalid use");
		usage();
	}
	else{
		speechToText();
	}
	if( err != paNoError ){
			fprintf( stderr, "An error occured while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	}
	
	return err;

}
