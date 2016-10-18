#define SAMPLE_RATE 16000
#define NUM_CHANNELS 1
#define FFT_SIZE (1024)
#define FRAMES_PER_BUFFER (128)
#define NUM_SECONDS     (2)
#define SAMPLE_SILENCE  (0.0f)

void apply_fft(float *, int, char *);
