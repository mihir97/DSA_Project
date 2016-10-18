#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "settings.h"
#include <string.h>
double noise = 0;

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    float      *recordedSamples;
}
TestData;

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
		if(avg < (noise + 0.01*noise)){
        		for( i=0; i<framesToCalc; i++ ){
        			*wptr++ = SAMPLE_SILENCE; 
            			if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;
        		}

		}
		else {
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

/*******************************************************************/
int main(int argc, char * argv[])
{
	PaStreamParameters	inputParameters, outputParameters;
	PaStream*           	stream;
	PaError             	err = paNoError;
	TestData        	data;
	int                 	i;
	int                 	totalFrames;
	int                 	numSamples;
	int                 	numBytes;
    	float              	max, val;
    	double              	average;

	data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
	data.frameIndex = 0;
	numSamples = totalFrames * NUM_CHANNELS;
	numBytes = numSamples * sizeof(float);
	data.recordedSamples = (float *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
    
	if(strcmp(argv[1],"-h") == 0){
		printf("Usage: ./record <filename>\nProgram writes the value of recorded sample after applying fft on it to <filename>\nFor same word spoken this values are similar\n");
		return 0;
	}	

	if( data.recordedSamples == NULL ){
        	printf("Could not allocate record array.\n");
        	goto done;
    	}

	for( i=0; i<numSamples; i++ ) 
		data.recordedSamples[i] = 0;
    	
	err = Pa_Initialize();
    	if( err != paNoError ) 
		goto done;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		fprintf(stderr,"Error: No default input device.\n");
        	goto done;
    	}
    	
	inputParameters.channelCount = NUM_CHANNELS;                    /* stereo input */
    	inputParameters.sampleFormat = paFloat32;
    	inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    	inputParameters.hostApiSpecificStreamInfo = NULL;
    
	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
              	&stream,
              	&inputParameters,
              	NULL,                  /* &outputParameters, */
              	SAMPLE_RATE,
              	FRAMES_PER_BUFFER,
              	paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              	recordCallback,
              	&data );
    	
	if( err != paNoError ) 
		goto done;
	
    	err = Pa_StartStream( stream );
    	
	if( err != paNoError ) 
		goto done;
    	
	printf("\n=== Calibrating... Please keep silence ===\n"); fflush(stdout);

  	while( ( err = Pa_IsStreamActive( stream ) ) == 1 )
    	{
        	Pa_Sleep(1000);
//        	printf("index = %d\n", data.frameIndex );
	}
    
	if( err < 0 ) 
		goto done;

    	err = Pa_CloseStream( stream );
    	if( err != paNoError ) goto done;

    	/* Measure maximum peak amplitude. */
    	max = 0;
    	average = 0.0;
    	for( i=0; i<numSamples; i++ )
    	{
		val = data.recordedSamples[i];
        	if( val < 0 ) val = -val; /* ABS */
        	if( val > max )
        	{
            		max = val;
        	}
        	average += val;
    	}

    	average = average / (double)numSamples;
	noise = max;
    
	printf("sample max amplitude = %f\n", max );
	printf("sample average = %lf\n", average );

/* Record some audio. -------------------------------------------- */
    	data.frameIndex = 0;
    	for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;
    	err = Pa_OpenStream(
              	&stream,
              	&inputParameters,
              	NULL,                  /* &outputParameters, */
              	SAMPLE_RATE,
              	FRAMES_PER_BUFFER,
              	paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              	recordCallback,
              	&data );
    	if( err != paNoError ) 
		goto done;

    	err = Pa_StartStream( stream );
    	if( err != paNoError ) 
		goto done;
    
	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    	while( ( err = Pa_IsStreamActive( stream ) ) == 1 ){
        	Pa_Sleep(1000);
    //    	printf("index = %d\n", data.frameIndex );
	}
    
	if( err < 0 ) 
		goto done;

    	err = Pa_CloseStream( stream );
    	if( err != paNoError ) goto done;

    	/* Measure maximum peak amplitude. */
    	max = 0;
    	average = 0.0;
    	for( i=0; i<numSamples; i++ )
    	{
		val = data.recordedSamples[i];
        	if( val < 0 ) 
			val = -val; /* ABS */
        	if( val > max )
        	{
            		max = val;
        	}
       	 	average += val;
    	}

    	average = average / (double)numSamples;

   	printf("sample max amplitude = %f\n", max );
    	printf("sample average = %lf\n", average );

    /* Write recorded data to a file. */
/*	if(argc == 2) {
    	{
        	FILE  *fid;
        	fid = fopen(argv[1], "wb");
        	if( fid == NULL ) {
            	printf("Could not open file.");
        	}
        	else{
            	//fwrite( data.recordedSamples, NUM_CHANNELS * sizeof(SAMPLE), totalFrames, fid );
		for(i = 0; i< numSamples ; i = i + FRAMES_PER_BUFFER){
			if(data.recordedSamples[i+1] != SAMPLE_SILENCE){
				int j;
				for(j = i; j < i + FRAMES_PER_BUFFER; j++)
					fprintf(fid, "%f\n", data.recordedSamples[j] );
			}
		}
		fclose( fid );
		printf("Wrote data to %s\n",argv[1]);
        	}
	}
}
*/

 
    /* Playback recorded data.  -------------------------------------------- */
    	data.frameIndex = 0;

    	outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
	if (outputParameters.device == paNoDevice) {
		fprintf(stderr,"Error: No default output device.\n");
		goto done;
	}
    
	outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
	outputParameters.sampleFormat =  paFloat32;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	printf("\n=== Now playing back. ===\n");
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
		goto done;

	if( stream ){
        	err = Pa_StartStream( stream );
        if( err != paNoError ) 
		goto done;
        
        printf("Waiting for playback to finish.\n");
	 
    	while( ( err = Pa_IsStreamActive( stream ) ) == 1 );
	if( err < 0 ) 
		goto done;
        
        err = Pa_CloseStream( stream );
        if( err != paNoError ) 
		goto done;
        
        printf("Done.\n");
	}

	apply_fft(data.recordedSamples, numSamples, argv[1]);

done:
	Pa_Terminate();
	if( data.recordedSamples )       /* Sure it is NULL or valid. */
//		free( data.recordedSamples );
		if( err != paNoError ){
			fprintf( stderr, "An error occured while using the portaudio stream\n" );
			fprintf( stderr, "Error number: %d\n", err );
			fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
	}
	return err;
}

