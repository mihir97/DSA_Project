project: main.o handleaudio.o fft.o compare.o findcurr.o settings.h 
	gcc main.o handleaudio.o fft.o findcurr.o libportaudio.a compare.o -Wall -lfftw3 -lrt -pthread -lasound -lm -o project
main.o: main.c settings.h
	gcc -c main.c -Wall
handleaudio.o: handleaudio.c settings.h
	gcc -c handleaudio.c -Wall
fft.o: fft.c settings.h
	gcc -c fft.c -Wall 
compare.o: compare.c settings.h
	gcc -c compare.c -Wall
findcurr.o: findcurr.c settings.h
	gcc -c findcurr.c -Wall
