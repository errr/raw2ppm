CFLAGS = -O2 -pipe -Werror

all: raw2ppm

raw2ppm:	raw2ppm.c
	gcc $(CFLAGS) -o raw2ppm raw2ppm.c -lraw

clean:
	rm -f raw2ppm
