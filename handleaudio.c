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
#include <stdlib.h>
#include "portaudio.h"
#include "settings.h"
#include <string.h>
#include <limits.h>
#include <dirent.h>
double noise = 0;
int silentFrames = INT_MIN / 2 + 10;
int wCount = -1;
/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
	TestData *data = (TestData*)userData;
	float *rptr = (float*)inputBuffer;
	float *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
	long framesToCalc;
	long i;
	int finished;
	unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

	(void) outputBuffer; /* Prevent unused variable warnings. */
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;


	if( framesLeft < framesPerBuffer ){
		framesToCalc = framesLeft;
		finished = paComplete;
	}
	else{
		framesToCalc = framesPerBuffer;
		finished = paContinue;
	}

	if( inputBuffer == NULL ){
		for( i=0; i<framesToCalc; i++ ){
			*wptr++ = SAMPLE_SILENCE;  /* left */
			if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
		}	
	}
	else{ 
		float *ptr = rptr, avg = 0.0;
		for( i =0 ; i< framesToCalc; i++) {
			if(*ptr++ > avg){
				avg = *ptr;
			}	
		}	
		if(avg < (noise + 0.1*noise)){
			data->maxFrameIndex -= framesToCalc;
			silentFrames += framesToCalc;

			/* If silentFrames > WORD_SPACE * SAMPLE_RATE
			 * add frameIndex to wordIndices[wordCount]
			 * Increment wordCount
			 * set silentFrames to 0
			 */
			if(silentFrames > (int)(WORD_SPACE * SAMPLE_RATE)){
				if(data->wordIndices[wCount] != data->frameIndex) wCount++;
				data->wordIndices[wCount] = data->frameIndex;
			}
			return finished;
		}
		else {
			silentFrames = 0;
        		for( i=0; i<framesToCalc; i++ ){
	          		*wptr++ = *rptr++;  /* left */
            			if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
			}
		}
	}
	data->frameIndex += framesToCalc;
	return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int playCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
	TestData *data = (TestData*)userData;
	float *rptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
	float *wptr = (float*)outputBuffer;
	unsigned int i;
	int finished;
	unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

	(void) inputBuffer; /* Prevent unused variable warnings. */
	(void) timeInfo;
	(void) statusFlags;
	(void) userData;

	if( framesLeft < framesPerBuffer ){
        	/* final buffer... */
        	for( i=0; i<framesLeft; i++ ){
            		*wptr++ = *rptr++;  /* left */
            		if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        	}
        	for( ; i<framesPerBuffer; i++ )
        	{
            		*wptr++ = 0;  /* left */
            		if( NUM_CHANNELS == 2 ) *wptr++ = 0;  /* right */
        	}
        	data->frameIndex += framesLeft;
        	finished = paComplete;
    	}
    	else{
        	for( i=0; i<framesPerBuffer; i++ ){
            		*wptr++ = *rptr++;  /* left */
            		if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        	}
        	data->frameIndex += framesPerBuffer;
        	finished = paContinue;
    	}
	return finished;
}

PaError init(PaStreamParameters* inputParameters){
	printf("Initialising PortAudio.....\n");

	PaError err = paNoError;
	err = Pa_Initialize();
    	if( err != paNoError ) 
		return err;
	
	inputParameters->device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters->device == paNoDevice) {
		fprintf(stderr,"Error: No default input device.\n");
        	return err;
    	}
	
	inputParameters->channelCount = NUM_CHANNELS;                    /* stereo input */
    	inputParameters->sampleFormat = paFloat32;
	inputParameters->suggestedLatency = Pa_GetDeviceInfo( inputParameters->device )->defaultLowInputLatency;
    	inputParameters->hostApiSpecificStreamInfo = NULL;

	printf("If you see messages above its not this code's fault\n");
	return paNoError;
    	
}

PaError getStream(PaStream **stream, PaStreamParameters inputParameters, TestData *data, int seconds){
	int                 	i;
	int                 	totalFrames;
	int                 	numSamples;
	int                 	numBytes;
	PaError			err = paNoError;
	data->maxFrameIndex = totalFrames = seconds * SAMPLE_RATE; /* Record for a few seconds. */
	data->frameIndex = 0;
	numSamples = totalFrames * NUM_CHANNELS;
	numBytes = numSamples * sizeof(float);
	data->recordedSamples = (float *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
	
	if( data->recordedSamples == NULL ){
        	printf("Could not allocate record array.\n");
        	return -1;
    	}
	for( i=0; i<numSamples; i++ ) 
		data->recordedSamples[i] = 0;
    	

	/* Record some audio for noise. -------------------------------------------- */
	err = Pa_OpenStream(
              	stream,
              	&inputParameters,
              	NULL,                  /* &outputParameters, */
              	SAMPLE_RATE,
              	FRAMES_PER_BUFFER,
              	paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              	recordCallback,
              	data );
    	
	if( err != paNoError ){
		return err;
	}	
	return paNoError;
}

void usage(){
	printf("Usage: ./project\n");
	printf("-h for help\n");
	printf("-r to replace signature vector for existing word\n");
	printf("-a to add a new word\n");
	printf("-l to list current words\n");
	

}
int neg_alphasort(const struct dirent **a, const struct dirent **b){
	return alphasort(b,a);
}
void list(){
	struct dirent **namelist;
	int n;
	char *p;
	printf("The existing words are:\n");
	n = scandir("data/word/./", &namelist, 0, neg_alphasort);
	if (n < 0)
		perror("scandir");
	else {
		while (n--) {
			p = namelist[n]->d_name;
			if(*p == '.') continue;
			while(*p != '.') 
				p++;
			*p = '\0';
			printf("%s\n", namelist[n]->d_name);
			free(namelist[n]);
		}
		free(namelist);
	}


//	system("ls data/word/ -1 | sed -e 's/\.dat$//'");
}

PaError setNoise(PaStreamParameters inputParameters){
/* Calibrating for noise detection */
	float              	max, val;
    	double              	average;
	PaStream*           	stream;
	PaError             	err = paNoError;
	TestData        	data;
	int i;

	err = getStream(&stream, inputParameters, &data, 2);
	
	if( err != paNoError ) 
		return err;

	err = Pa_StartStream( stream );
    	
	if( err != paNoError ) 
		return err;
    	
	printf("\n=== Calibrating... Please keep silence ===\n"); fflush(stdout);

  	while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    	{
        	Pa_Sleep(1000);

	}
    
	if( err < 0 ) 
		return err;

    	err = Pa_CloseStream( stream );
    	if( err != paNoError ) return err;

    	/* Measure maximum peak amplitude. */
    	max = 0;
    	average = 0.0;
    	for( i=0; i< 2 * SAMPLE_RATE; i++ )
    	{
		val = data.recordedSamples[i];
        	if( val < 0 ) val = -val; /* ABS */
        	if( val > max )
        	{
            		max = val;
        	}
        	average += val;
    	}

    	average = average / (double)(2 * SAMPLE_RATE);
	noise = max;
    
	printf("noise max amplitude = %f\n", max );
	printf("noise average = %lf\n", average );


/* Noise detection Ended */
	if( data.recordedSamples )       /* Sure it is NULL or valid. */
		free( data.recordedSamples );
	return err;
}


PaError replace(char *str){
	PaStreamParameters inputParameters;
	PaError err = paNoError;
	TestData data;
	PaStream *stream;
	int flag = 1;
	char ch;
	data.wordIndices = (int*)calloc(10, sizeof(int));
	err = init(&inputParameters);
	if(err != paNoError){
		return err;
	}

	err = setNoise(inputParameters);
	if(err != paNoError){
		return err;
	}

/* Record some audio. -------------------------------------------- */
	while(flag) {
		err = getStream(&stream, inputParameters, &data, 3);
		if( err != paNoError ) 
			return err;
		printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

		err = Pa_StartStream( stream );
		if( err != paNoError ) 
			return err;

		while( ( err = Pa_IsStreamActive( stream ) ) == 1 ){
			Pa_Sleep(1000);
		}
	
		err = play(data);
		if( err != paNoError ) 
			return err;
		printf("Is this voice okay? (y/n) ");
		getc(stdin);
		ch = getc(stdin);
		if(ch == 'y' || ch == 'Y'){
			flag = 0;
		}
	}
	char buff[80];
	sprintf(buff,"data/word/%s.dat",str);
	apply_fft(data.recordedSamples, data.maxFrameIndex, buff);
	
	return err;
}

PaError add(char *str){
	PaStreamParameters inputParameters;
	PaError err = paNoError;
	TestData data;
	PaStream *stream;
	int flag = 1;
	char ch;
	data.wordIndices = (int*)calloc(10, sizeof(int));

	err = init(&inputParameters);
	if(err != paNoError){
		return err;
	}

	err = setNoise(inputParameters);
	if(err != paNoError){
		return err;
	}
	
	/* Record some audio. -------------------------------------------- */
	while(flag) {
		err = getStream(&stream, inputParameters, &data, 3);
		if( err != paNoError ) 
			return err;
		printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

		err = Pa_StartStream( stream );
		if( err != paNoError ) 
			return err;

		while( ( err = Pa_IsStreamActive( stream ) ) == 1 ){
			Pa_Sleep(1000);
		}
	
		err = play(data);
		if( err != paNoError ) 
			return err;
			
		printf("Is this voice okay? (y/n) ");
		getc(stdin);
		ch = getc(stdin);
		if(ch == 'y' || ch == 'Y'){
			flag = 0;
		}
	}
	char buff[30];
	sprintf(buff,"data/word/%s.dat",str);
	apply_fft(data.recordedSamples, data.maxFrameIndex, buff);
	
	char word[20];

	FILE *fp;

	list();
	printf("Enter words from list after which the added word occurs:\n(enter START if it occurs in the beginning)\n");
	while(scanf("%s", word) != EOF){
		sprintf(buff, "data/next/%s.nxt", word);
		if(access(buff,F_OK) == -1){
			printf("Word doesn't exist\nTry adding first");
			return err;
		}
		fp = fopen(buff, "a");
		fprintf(fp, "%s\n", str);
		fclose(fp);
	
	}
	
	list();
	printf("Enter words from list which occur after the added word:\n(enter START if sentence ends after it)\n");
	sprintf(buff, "data/next/%s.nxt", str);
	fp = fopen(buff, "w");
	while(scanf("%s", word) != EOF){
		sprintf(buff, "data/next/%s.nxt", word);
		if(access(buff,F_OK) == -1){
			printf("Word doesn't exist\nTry adding first");
			return err;
		}
		fprintf(fp, "%s\n", word);
	}
	fclose(fp);
	return err;
}

PaError play(TestData data){
	PaStream *stream;
	PaStreamParameters outputParameters;
	PaError err = paNoError;
		
   /* Playback recorded data.  -------------------------------------------- */
    	data.frameIndex = 0;

    	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr,"Error: No default output device.\n");
		return err;
	}
    
	outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
	outputParameters.sampleFormat =  paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

//	printf("\n=== Now playing back. ===\n");
	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		playCallback,
		&data );
	if( err != paNoError ) 
		return err;

	if( stream ){
        	err = Pa_StartStream( stream );
        	if( err != paNoError ) 
			return err;
		printf("Waiting for playback to finish.\n");

		while( ( err = Pa_IsStreamActive( stream ) ) == 1 );
		if( err < 0 ) 
			return err;

		err = Pa_CloseStream( stream );
		if( err != paNoError ) 
			return err;

		printf("Done.\n");
	}
	return err;
}

PaError speechToText(){
	PaStreamParameters inputParameters;
	PaError err = paNoError;
	TestData data;
	PaStream *stream;

   	float *tmpSample = (float*)malloc(sizeof(float) * 16 * SAMPLE_RATE);
	int tmpIndex = 0;
	int wordCount = 0;
	data.readIndex = 0;
	data.wordIndices = (int*)calloc(16, sizeof(int));
	data.wordIndices[0] = -1;
 
	err = init(&inputParameters);
	if(err != paNoError){
		return err;
	}
	
	err = setNoise(inputParameters);
	if(err != paNoError){
		return err;
	}
	
	silentFrames = -16000;

/* Record some audio. -------------------------------------------- */

	err = getStream(&stream, inputParameters, &data, NUM_SECONDS);
    	if( err != paNoError ) 
		return err;
	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

	err = Pa_StartStream( stream );
    	if( err != paNoError ) 
		return err;
	char *curr, start[] = {'S', 'T' , 'A', 'R', 'T', '\0'};
	curr = start;
	while( ( err = Pa_IsStreamActive( stream ) ) == 1 ){
	/*Copy from recordedSamples to tmp array till readIndex is not equal to frameIndex
	 *If reached end word indicator call apply_fft and compare
	 *Increment word count
	 *Set tmparray index to 0
	 */
		if(data.readIndex <= data.frameIndex && wordCount < wCount) {
			tmpSample[tmpIndex++] = data.recordedSamples[data.readIndex++];
			if(data.readIndex == data.wordIndices[wordCount]){
				wordCount++;
				apply_fft(tmpSample, tmpIndex, "tmp.dat");
				tmpIndex = 0;
				curr = findcurr(curr, "tmp.dat");
				if(curr[strlen(curr) + 1] == 'S'){
					printf("\b.\n%s ", curr); fflush(stdout);
				}
				else{
					printf("%s ", curr); fflush(stdout);
				}
			}
		}
		
	}
/* Finish Processing for samples left after recording thread is over */

 	while(data.readIndex <= data.frameIndex) {
		tmpSample[tmpIndex++] = data.recordedSamples[data.readIndex++];
		if(data.readIndex == data.wordIndices[wordCount]){
			wordCount++;
			apply_fft(tmpSample, tmpIndex, "tmp.dat");
			curr = findcurr(curr, "tmp.dat");
			if(curr[strlen(curr) + 1] == 'S'){
				printf("\b.\n%s ", curr); fflush(stdout);
			}
			else{
				printf("%s ", curr); fflush(stdout);
			}
		}
	}
	printf("\b.\n");
	
	free(tmpSample);

	if( err < 0 ) 
		return err;

    	err = Pa_CloseStream( stream );
    	if( err != paNoError ) return err;

//	err = play(data);
	remove("tmp.dat");
	Pa_Terminate();
	if( data.recordedSamples )       /* Sure it is NULL or valid. */
		free( data.recordedSamples );
	if(data.wordIndices)
		free(data.wordIndices);
	return err;


}

