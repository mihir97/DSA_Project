record: record.o fft.o settings.h 
	gcc record.o fft.o libportaudio.a -lfftw3 -lrt -pthread -lasound -lm -o record
record.o: record.c settings.h
	gcc -c record.c
fft.o: fft.c settings.h
	gcc -c fft.c -lfftw3 -lm

