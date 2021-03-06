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

#include<portaudio.h>
#define SAMPLE_RATE 16000
#define NUM_CHANNELS 1
#define FFT_SIZE (1024)
#define FRAMES_PER_BUFFER (128)
#define NUM_SECONDS     (20)
#define SAMPLE_SILENCE  (0.0f)
#define WORD_SPACE (0.25) /* In Seconds */

typedef struct
{
	int	frameIndex;  /* Index into sample array. */
	int	readIndex;	
	int	maxFrameIndex;
	float	*recordedSamples;
	int	*wordIndices;
}
TestData;


void apply_fft(float *, int, char *);
float compare(char *, char *);
char* findcurr(char *, char *);
PaError add(char *);
PaError replace(char *);
PaError speechToText();
PaError getStream(PaStream **, PaStreamParameters, TestData *,int);
PaError init(PaStreamParameters *);
PaError setNoise(PaStreamParameters);
PaError play(TestData);
void list();
void usage();
