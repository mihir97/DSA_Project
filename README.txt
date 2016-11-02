Speech Recognition

111503042

The program recognises the speech with some errors.

Libraries used:
portaudio
fftw

What it does?
First it calibrates for 2 seconds to find the noise. Further all averages which are less than this value are considered as silence.
Then it listens for 20 seconds in a recording thread, splits the recording into words by counting number of silent frames, tries to identify the word with the context of previous word and outputs some meaningful sentence from the set of words defined.
NOTE: A gap of 0.25 seconds is expected between two words.
You can also add new words. Or replace the signature of existing words.

Use ./project -h for help
It may vary from user to user. Hence overwriting the saved signature of a word is allowed and recommended. 

HOW IT WORKS

How words are saved?
Using portaudio we get the the amplitudes ( For 1 sec 16000 values).
This is converted to frequency domain by applying fft ( used fftw library).
The dominant frequencies are extracted and saved in a file ( this is the signature of the word)
Mostly for a word and for the same person this signature matches.

How next words are predicted?
Every word has its .nxt file where there are set of words which can occur after it.

